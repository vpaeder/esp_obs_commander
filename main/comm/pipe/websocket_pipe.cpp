/** \file websocket_pipe.h
 *  \brief Implementation file for WebSocket handler class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "websocket_pipe.h"

namespace eobsws::comm::pipe {

  WebSocketPipe::WebSocketPipe(
    std::shared_ptr<DataBroker> db,
    const std::string & wifi_ssid,
    const std::string & wifi_password,
    const std::string & ws_host,
    uint16_t ws_port,
    const std::string & ws_path) : WiFiPipe(db, wifi_ssid, wifi_password),
    ws_host(ws_host), ws_port(ws_port), ws_path(ws_path)
  {
    // subscribe callback to data broker
    this->db->subscribe(this->convert_callback<WebSocketPipe>(this));
  }


  WebSocketPipe::~WebSocketPipe() {
      esp_websocket_client_close(this->ws_client, portMAX_DELAY);
      esp_websocket_client_stop(this->ws_client);
      esp_websocket_client_destroy(this->ws_client);
  }


  void WebSocketPipe::connect() {
      if (!this->connected) {
          WiFiPipe::connect();
          // configure WebSocket client
          esp_websocket_client_config_t websocket_cfg = {};
          websocket_cfg.host = this->ws_host.c_str();
          websocket_cfg.port = this->ws_port;
          websocket_cfg.buffer_size = CONFIG_WS_BUFFER_SIZE;
          websocket_cfg.task_stack = 8192;
          websocket_cfg.task_prio = 18;
          
          ESP_LOGI("WebSocketPipe", "initializing WebSocket client.");

          // initialize WebSocket client
          this->ws_client = esp_websocket_client_init(&websocket_cfg);
          // define WebSocket event handler
          auto fws = [](void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto obj = reinterpret_cast<WebSocketPipe*>(event_handler_arg);
            obj->websocket_callback(event_base, event_id, event_data);
          };
          esp_websocket_register_events(this->ws_client, WEBSOCKET_EVENT_ANY, fws, static_cast<void *>(this) );
          
          // start WebSocket client
          esp_websocket_client_start(this->ws_client);
          ESP_LOGI("WebSocketPipe", "WebSocket client started.");
      }
  }


  int WebSocketPipe::write_bytes(const uint8_t * bytes, uint16_t len) {
    return this->write_bytes(reinterpret_cast<const char *>(bytes), len);
  }
  int WebSocketPipe::write_bytes(const char * bytes, uint16_t len) {
    if (this->connected && esp_websocket_client_is_connected(this->ws_client)) {
      ESP_LOGI("WebSocketPipe", "sending message: %.*s", len, bytes);
      esp_websocket_client_send_text(this->ws_client, bytes, len, 500/portTICK_PERIOD_MS);
      return len;
    }
    return -1;
  }
  int WebSocketPipe::write_bytes(std::string bytes) {
    return this->write_bytes(bytes.c_str(), bytes.length());
  }


  bool WebSocketPipe::publish_callback(MessageType t, const std::string & data) {
    if ((t & this->in_message_type) == MessageType::NoOutlet) {
      ESP_LOGI("WebSocketPipe", "message of type %d rejected. Expected %d", static_cast<int>(t), static_cast<int>(this->in_message_type));
      return false;
    }
    return this->write_bytes(data) == 0;
  }


  void WebSocketPipe::websocket_callback(esp_event_base_t event_base, int32_t event_id, void *event_data) {
      auto data = static_cast<esp_websocket_event_data_t *>(event_data);
      switch (event_id) {
      case WEBSOCKET_EVENT_CONNECTED:
          this->connected = true;
          break;
      case WEBSOCKET_EVENT_DISCONNECTED:
          this->connected = false;
          break;
      case WEBSOCKET_EVENT_DATA:
          if (data->op_code == 0x08) {
              // connection close
              ESP_LOGI("WebSocketPipe", "got connection close frame with data=%.*s", data->data_len-2, static_cast<const char*>(data->data_ptr+2));
              this->connected = false; 
          } else if (data->op_code == 0x00 || data->op_code == 0x01 || data->op_code == 0x02) {
              // continuation frame, text frame or binary frame
              ESP_LOGI("WebSocketPipe", "Received=%.*s", data->data_len, (char *)data->data_ptr);
              this->db->publish(this->out_message_type, std::string(data->data_ptr, data->data_len));
          } else if (data->op_code == 0x09 || data->op_code == 0x0a) {
              // ping or pong
          }
          break;
      case WEBSOCKET_EVENT_ERROR:
          break;
      }
  }

}
