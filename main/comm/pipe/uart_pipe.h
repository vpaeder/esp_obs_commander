/** \file uart_pipe.h
 *  \brief Header file for UART handler base class. This class can be used to write
 *  an UART communication handler.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "sdkconfig.h"

#include "driver/uart.h"
#include "freertos/queue.h"

#include "../data_node.h"

/** \namespace eobsws::comm::pipe
 *  \brief Data pipes.
 */
namespace eobsws::comm::pipe {
  /** \class UARTPipe
    *  \brief Base UART pipe class. It handles communication through an UART port.
    *  Overload process_data to create specific processor.
    */
  class UARTPipe : public DataNode {
    /**
     * @brief Need access to DataNode internals.
     * @relates DataNode
     */
    friend DataNode;
    
    private:
    /** \property uart_port_t port
      *  \brief UART port number.
      */
    uart_port_t port;
    
    /** \property QueueHandle_t queue
      *  \brief Handle of event queue.
      */
    QueueHandle_t queue;
    
    protected:
    /** \fn void event_task()
      *  \brief Event task.
      */
    void event_task();

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback to handle data broker messages.
     *  \param t: message type.
     *  \param data: message content.
     *  \returns true if callback could process data, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data);

    public:
    /** \fn UARTPipe(std::shared_ptr<DataBroker> db,
     *               uart_port_t uart_port = UART_NUM_0,
     *               int tx_io_num = 1,
     *               int rx_io_num = 3,
     *               int baud_rate=115200,
     *               uart_word_length_t data_bits = UART_DATA_8_BITS,
     *               uart_parity_t parity = UART_PARITY_DISABLE,
     *               uart_stop_bits_t stop_bits = UART_STOP_BITS_1
     *              );
     *  \brief Constructor. See ESP IDF documentation for details on UART parameters.
     *  \param db: pointer to a pub-sub data broker.
     *  \param uart_port: UART port corresponding to USB connection.
     *  \param tx_io_num: GPIO pin for TX.
     *  \param rx_io_num: GPIO pin for RX.
     *  \param baud_rate: baud rate.
     *  \param data_bits: number of data bits per word.
     *  \param parity: parity mode
     *  \param stop_bits: stop bits number.
      */
    UARTPipe(
      std::shared_ptr<DataBroker> db,
      uart_port_t uart_port = UART_NUM_0,
      int tx_io_num = 1,
      int rx_io_num = 3,
      int baud_rate=115200,
      uart_word_length_t data_bits = UART_DATA_8_BITS,
      uart_parity_t parity = UART_PARITY_DISABLE,
      uart_stop_bits_t stop_bits = UART_STOP_BITS_1
    );

    /** \fn ~UARTPipe()
      *  \brief Destructor.
      */
    ~UARTPipe();
    
    /** \fn int write_bytes(const std::string & bytes)
      *  \brief Write bytes to transfer buffer.
      *  \param bytes : data to be transferred
      *  \returns Number of bytes written, or -1 if an error occurred.
      */
    int write_bytes(const std::string & bytes);

  };

}
