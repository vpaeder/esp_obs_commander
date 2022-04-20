/** \file wifi_pipe.cpp
 *  \brief Implementation file for WiFi handler class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <cstring>

#include "wifi_pipe.h"

namespace eobsws::comm::pipe {

  #define WIFI_CONNECTED_BIT BIT0
  #define WIFI_FAIL_BIT      BIT1

  WiFiPipe::WiFiPipe(
    std::shared_ptr<DataBroker> db,
    const std::string & wifi_ssid,
    const std::string & wifi_password) : DataNode(db)
  {
    this->wifi_ssid = wifi_ssid;
    this->wifi_password = wifi_password;
    this->in_message_type = MessageType::OutboundWireless;
    this->out_message_type = MessageType::InboundWireless;
  }

  WiFiPipe::~WiFiPipe() {
      esp_wifi_disconnect();
      esp_wifi_stop();
      esp_wifi_deinit();
  }

  void WiFiPipe::connect() {
      if (!this->connected) {
          ESP_LOGI("WiFiPipe", "connecting to WiFi.");
          ESP_LOGI("WiFiPipe", "Network SSID: %s", this->wifi_ssid.c_str() );

          this->wifi_event_group = xEventGroupCreate();
          // set up WiFi station
          esp_netif_create_default_wifi_sta();
          wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
          // initialize WiFi interface
          ESP_ERROR_CHECK(esp_wifi_init(&cfg));

          // define handler for WiFi events
          auto fwifi = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            auto obj = reinterpret_cast<WiFiPipe*>(arg);
            obj->wifi_callback(event_base, event_id, event_data);
          };
          // set handler for WiFi events: ESP_EVENT_ANY_ID, IP_EVENT_STA_GOT_IP
          esp_event_handler_instance_t instance_any_id;
          esp_event_handler_instance_t instance_got_ip;
          ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                              ESP_EVENT_ANY_ID,
                                                              fwifi,
                                                              static_cast<void*>(this),
                                                              &instance_any_id));
          ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                              IP_EVENT_STA_GOT_IP,
                                                              fwifi,
                                                              static_cast<void*>(this),
                                                              &instance_got_ip));
          
          // configure WiFi with provided SSID/password
          wifi_config_t wifi_config = {};
          wifi_config.sta = {};
          memcpy(wifi_config.sta.ssid, this->wifi_ssid.c_str(), this->wifi_ssid.length());
          memcpy(wifi_config.sta.password, this->wifi_password.c_str(), this->wifi_password.length());
          // set security level to at least WPA2
          wifi_config.sta.threshold = {};
          wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
          // set how to behave with protected management frames
          wifi_config.sta.pmf_cfg = {
                  .capable = true,
                  .required = false
              };
          
          // apply WiFi configuration
          ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
          ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

          // start WiFi connection
          ESP_ERROR_CHECK(esp_wifi_start());

          /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
          * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
          xEventGroupWaitBits(this->wifi_event_group,
              WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
              pdFALSE,
              pdFALSE,
              portMAX_DELAY);

          ESP_LOGI("WiFiPipe", "WiFi connection initialized.");
      }
  }


  void WiFiPipe::wifi_callback(esp_event_base_t event_base, int32_t event_id, void* event_data) {
      if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
          esp_wifi_connect();
      } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
          // connection failed...
          if (this->wifi_retry_count < CONFIG_WIFI_MAX_RETRIES) {
              // if number of retries < max, retry
              esp_wifi_connect();
              this->wifi_retry_count++;
          } else {
              // number of retries reached max => fail
              xEventGroupSetBits(this->wifi_event_group, WIFI_FAIL_BIT);
          }
      } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
          // connected, yippie!
          this->wifi_retry_count = 0;
          xEventGroupSetBits(this->wifi_event_group, WIFI_CONNECTED_BIT);
      }
  }


  int8_t WiFiPipe::get_rssi() const {
      if (!this->connected) return 0;
      wifi_ap_record_t ap_info;
      esp_wifi_sta_get_ap_info(&ap_info);
      return ap_info.rssi;
  }

}
