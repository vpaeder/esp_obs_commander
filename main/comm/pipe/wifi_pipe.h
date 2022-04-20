/** \file wifi_pipe.h
 *  \brief Header file for WiFi handler class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "sdkconfig.h"

#include <stdio.h>
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "../data_node.h"

namespace eobsws::comm::pipe {

  /** \class WiFiPipe
   *  \brief Base WiFi pipe class. It handles communication through a WiFi connection.
   */
  class WiFiPipe : public DataNode {
    /**
     * @brief Need access to DataNode internals.
     * @relates DataNode
     */
    friend DataNode;

    protected:
    /** \property std::string wifi_ssid
     *  \brief WiFi network SSID
     */
    std::string wifi_ssid;
    
    /** \property std::string wifi_password
     *  \brief WiFi network password
     */
    std::string wifi_password;

    /** \property std::string wifi_hostname
     *  \brief Hostname on WiFi network
     */
    std::string wifi_hostname;
    
    /** \property bool loop_running
     *  \brief If true, event loop is running. If set to false, event loop stops.
     */
    bool loop_running;

    /** \property bool connected
     *  \brief If true, WiFi connection is established; false otherwise.
     */
    bool connected = false;
    
    /** \property EventGroupHandle_t wifi_event_group
     *  \brief WiFi event group.
     */
    EventGroupHandle_t wifi_event_group;

    /** \property int wifi_retry_count
     *  \brief Connection retry count.
     */
    int wifi_retry_count = 0;
    
    /** \fn void wifi_callback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
     *  \brief Callback to process WiFi events.
     */
    void wifi_callback(esp_event_base_t event_base, int32_t event_id, void* event_data);

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback to handle data broker messages.
     *  \param t: message type.
     *  \param data: message content.
     *  \returns true if callback could process data, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data);
    
    public:
    /** \fn WiFiPipe(std::shared_ptr<DataBroker> db,
     *               const std::string & wifi_ssid,
     *               const std::string & wifi_password
     *              )
     *  \param db: pointer to a pub-sub data broker.
     *  \param wifi_ssid : WiFi SSID
     *  \param wifi_password : WiFi password
     *  \brief Constructor.
     */
    WiFiPipe(
      std::shared_ptr<DataBroker> db,
      const std::string & wifi_ssid,
      const std::string & wifi_password
      );

    /** \fn ~WiFiPipe()
     *  \brief Destructor.
     */
    ~WiFiPipe();

    /** \fn virtual void connect()
     *  \brief Initiate a WiFi connection.
     */
    virtual void connect();

    /** \fn int8_t get_rssi() const
     *  \brief Gets raw WiFi RSSI value.
     *  \returns raw value of WiFi RSSI.
     */
    int8_t get_rssi() const;
    
  };

}
