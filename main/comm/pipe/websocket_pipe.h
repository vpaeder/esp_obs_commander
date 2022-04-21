/** \file websocket_pipe.h
 *  \brief Header file for WebSocket handler class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "esp_websocket_client.h"

#include "wifi_pipe.h"

namespace eobsws::comm::pipe {

  /** \class WebSocketPipe
   *  \brief Base UART command parser class. It handles communication through an UART port
   *  and provides scaffolding to process data.
   *  Overload event_task to create specific processor.
   */
  class WebSocketPipe : public WiFiPipe {
    /**
     * @brief Need access to DataNode internals.
     * @relates DataNode
     */
    friend DataNode;
    
    private:
    /** \property std::string ws_host
     *  \brief WebSocket server host
     */
    std::string ws_host;
    
      /** \property uint16_t ws_port
     *  \brief WebSocket server port
     */
    uint16_t ws_port;
    
    /** \property std::string ws_path
     *  \brief WebSocket server path
     */
    std::string ws_path;
    
    /** \property char data[UART_BUF_SIZE]
     *  \brief Data buffer.
     */
    char data[CONFIG_WEBSOCKET_BUF_SIZE];
    
    /** \property esp_websocket_client_handle_t ws_client
     *  \brief Instance of WebSocket client.
     */
    esp_websocket_client_handle_t ws_client;
    
    /** \fn void websocket_callback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
     *  \brief Callback to process WebSocket events.
     *  \param event_base: base ID of the event to register the handler for.
     *  \param event_id: ID of the event to register the handler for.
     *  \param event_data: user-defined context data.
     */
    void websocket_callback(esp_event_base_t event_base, int32_t event_id, void *event_data);
    
    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback to handle data broker messages.
     *  \param t: message type.
     *  \param data: message content.
     *  \returns true if callback could process data, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data);
    
    public:
    /** \fn WebSocketPipe(std::shared_ptr<DataBroker> db,
     *                    const std::string & wifi_ssid,
     *                    const std::string & wifi_password,
     *                    const std::string & ws_host,
     *                    const uint16_t & ws_port,
     *                    const std::string & ws_path
     *                   )
     *  \param db: pointer to a pub-sub data broker.
     *  \param wifi_ssid : WiFi SSID
     *  \param wifi_password : WiFi password
     *  \param ws_host : WebSocket host name
     *  \param ws_port : WebSocket port on host
     *  \param ws_path : WebSocket URI
     *  \brief Constructor.
     */
    WebSocketPipe(
      std::shared_ptr<DataBroker> db,
      const std::string & wifi_ssid,
      const std::string & wifi_password,
      const std::string & ws_host,
      uint16_t ws_port,
      const std::string & ws_path
      );

    /** \fn ~WebSocketPipe()
     *  \brief Destructor.
     */
    ~WebSocketPipe();

    /** \fn void connect()
     *  \brief Initiate a WebSocket connection.
     */
    void connect() override;
    
    /** \fn int write_bytes(const uint8_t * bytes, uint16_t len)
     *  \brief Write bytes to transfer buffer.
     *  \param bytes : data to be transferred
     *  \param len : length of data to be transferred, in bytes.
     *  \returns Number of bytes written, or -1 if an error occurred.
     */
    int write_bytes(const uint8_t * bytes, uint16_t len);
    /** \fn int write_bytes(const char * bytes, uint16_t len)
     *  \brief Write bytes to transfer buffer.
     *  \param bytes : data to be transferred
     *  \param len : length of data to be transferred, in bytes.
     *  \returns Number of bytes written, or -1 if an error occurred.
     */
    int write_bytes(const char * bytes, uint16_t len);
    /** \fn int write_bytes(std::string bytes)
     *  \brief Write bytes to transfer buffer.
     *  \param bytes : data to be transferred
     *  \returns Number of bytes written, or -1 if an error occurred.
     */
    int write_bytes(std::string bytes);

  };

}
