/** \file obs_parser.cpp
 *  \brief Implementation file for obs-websocket command parser class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <string.h>
#include "cJSON.h"

#include "obs_parser.h"
#include "obs_parser_stub.h"

namespace eobsws::comm::parser {

  OBSParser::OBSParser(std::shared_ptr<DataBroker> db) : Parser(db) {
        this->in_message_type = MessageType::InboundWireless;
        this->out_message_type = MessageType::OutboundWireless;
        this->db->subscribe(this->convert_callback<OBSParser>(this));
  }

  bool OBSParser::publish_callback(MessageType t, const std::string & data) {
    if ((t & this->in_message_type) == MessageType::NoOutlet) return false;

    cJSON * js = cJSON_Parse(data.c_str());
    if (js == nullptr)
      return false;

    // OBS messages must contain an opcode ("op") and a data field ("d")
    if (!cJSON_HasObjectItem(js,"op") || !cJSON_HasObjectItem(js,"d")) {
      cJSON_Delete(js);
      return false;
    }

    // clean up parsers from dangling pointers
    this->clean_up_stubs();
    int op = cJSON_GetNumberValue(cJSON_GetObjectItem(js,"op"));
    auto stub = this->find_stub_for_command(std::to_string(op));
    if (stub != nullptr) {
      // parse data content with appropriate parser
      auto payload = cJSON_Print(cJSON_GetObjectItem(js, "d"));
      auto [message_type, success, result] = stub->parse(payload);
      free(payload);
      cJSON_Delete(js);
      ESP_LOGI("OBSParser", "stub replies with message %s", result.c_str());
      return success & this->db->publish(message_type, result);
    }
    cJSON_Delete(js);
    return false;
  }
  
}
