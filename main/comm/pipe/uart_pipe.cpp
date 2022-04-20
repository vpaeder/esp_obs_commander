/** \file uart_pipe.cpp
 *  \brief UART pipe base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "uart_pipe.h"
#include "esp_log.h"
#include <string.h>

namespace eobsws::comm::pipe {

  /** \var static const char SerialTermination
    *  \brief Serial termination character.
    */
  static const char SerialTermination = '\r';

  UARTPipe::UARTPipe(
    std::shared_ptr<DataBroker> db,
    uart_port_t uart_port /* = UART_NUM_0 */,
    int tx_io_num /* = 1 */,
    int rx_io_num /* = 3 */,
    int baud_rate /* = 115200 */,
    uart_word_length_t data_bits /* = UART_DATA_8_BITS */,
    uart_parity_t parity /* = UART_PARITY_DISABLE */,
    uart_stop_bits_t stop_bits /* = UART_STOP_BITS_1 */
    ) : DataNode(db) {
    // this part is adapted from uart_echo example of ESP IDF
    ESP_LOGI("UARTPipe", "initializing handler for port %d.", uart_port);
    // define UART configuration
    uart_config_t uart_config = {};
    uart_config.baud_rate = baud_rate;
    uart_config.data_bits = data_bits;
    uart_config.parity    = parity;
    uart_config.stop_bits = stop_bits;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    
    /*const uart_intr_config_t intr_conf = {
      .intr_enable_mask = 0x19d,
      .rx_timeout_thresh = 1,
      .txfifo_empty_intr_thresh = 1,
      .rxfifo_full_thresh = 1
    };
    uart_intr_config(uart_port, &intr_conf);*/

    this->port = uart_port;

    // setup UART driver
    ESP_LOGI("UARTPipe", "installing driver.");
    uart_driver_install(uart_port, 2*CONFIG_UART_BUF_SIZE, 2*CONFIG_UART_BUF_SIZE, 20, &(this->queue), 0);
    uart_param_config(uart_port, &uart_config);
    uart_set_pin(uart_port, tx_io_num, rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI("UARTPipe", "setting up event queue.");
    uart_pattern_queue_reset(uart_port, 20); // reset queue with 20 positions
    
    // set event loop
    this->loop_running = true;
    auto fevent = [](void* arg) {
      auto obj = reinterpret_cast<UARTPipe*>(arg);
      obj->event_task();
    };
    // Stack size for event task has to be adapted to what is run by the task
    xTaskCreate(fevent, "uart_event_task", CONFIG_UART_EVENT_STACK_SIZE, static_cast<void*>(this), 10,  nullptr);
    
    this->in_message_type = MessageType::OutboundWired;
    this->out_message_type = MessageType::InboundWired;
    // subscribe callback to data broker
    this->db->subscribe(this->convert_callback<UARTPipe>(this));
  }


  UARTPipe::~UARTPipe() {
    this->loop_running = false;
  }


  int UARTPipe::write_bytes(const std::string & bytes) {
    return uart_write_bytes(this->port, bytes.c_str(), bytes.length());
  }


  bool UARTPipe::publish_callback(MessageType t, const std::string & data) {
    if ((t & this->in_message_type) == MessageType::NoOutlet) {
      ESP_LOGI("UARTPipe", "message of type %d rejected. Expected %d", static_cast<int>(t), static_cast<int>(this->in_message_type));
      return false;
    }
    ESP_LOGI("UARTPipe", "processing message of type %d", static_cast<int>(t));
    return this->write_bytes(data) == data.size();
  }

  void UARTPipe::event_task() {
    ESP_LOGI("UARTPipe", "created event task.");
    uart_event_t event;
    std::string data;
    char buf[CONFIG_UART_BUF_SIZE];
    // UART event task accumulates characters until the SerialTermination character was found
    while (this->loop_running) {
      bzero(buf, CONFIG_UART_BUF_SIZE);
      if(xQueueReceive(this->queue, static_cast<void*>(&event), (portTickType)portMAX_DELAY)) {
        ESP_LOGI("UARTPipe", "received UART event; processing...");
        switch(event.type) {
          case UART_DATA: {
            ESP_LOGI("UARTPipe", "received UART event of type UART_DATA.");
            auto len = uart_read_bytes(this->port, buf, CONFIG_UART_BUF_SIZE, 20 / portTICK_RATE_MS);
            if (len>0) {
              data.append(buf, len);
              const char & last = data.back();
              if (last == SerialTermination) {
                data.erase(data.end()-1);
                this->db->publish(this->out_message_type, data);
                data.clear();
              }
            }
          }
          break;
          case UART_FIFO_OVF:
          case UART_BUFFER_FULL: {
            ESP_LOGI("UARTPipe", "received UART event of type UART_%s.", event.type==UART_FIFO_OVF ? "_FIFO_OVF" : "_BUFFER_FULL");
            uart_flush_input(this->port);
            xQueueReset(this->queue);
          }
          break;
          
          default:
          break;
        }
      }
    }
    vTaskDelete(nullptr); // delete task from RTOS task list
  }
  
}
