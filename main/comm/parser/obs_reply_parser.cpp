/** \file obs_reply_parser.cpp
 *  \brief Implementation file for obs-websocket reply parser class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "obs_reply_parser.h"
#include "cJSON.h"
#include "esp_log.h"

namespace eobsws::comm::parser {

    OBSReplyParser::OBSReplyParser(std::shared_ptr<DataBroker> db) : Parser(db) {
        this->in_message_type = MessageType::Event;
        this->out_message_type = MessageType::OutboundAny;
        this->db->subscribe(this->convert_callback<OBSReplyParser>(this));
    }

    bool OBSReplyParser::publish_callback(MessageType t, const std::string & data) {
        if ((t & this->in_message_type) == MessageType::NoOutlet) return false;

        // check that data is indeed JSON
        cJSON * js = cJSON_Parse(data.c_str());
        if (js == nullptr)
            return false;
        // we expect data forwarded by OBSRequestResponse or OBSRequestBatchResponse 
        // for single requests, there must be a requestType field
        // for batch requests, there must be a results field
        // in both types, we expect a requestId field
        if (!cJSON_HasObjectItem(js,"requestId")
            || (!cJSON_HasObjectItem(js,"requestType") && !cJSON_HasObjectItem(js, "results"))) {
            cJSON_Delete(js);
            return false;
        }

        // clean up parsers from dangling pointers
        this->clean_up_stubs();
        auto reqId = cJSON_GetStringValue(cJSON_GetObjectItem(js,"requestId"));
        auto stub = this->find_stub_for_command(reqId);
        cJSON_Delete(js);
        if (stub != nullptr) {
            // parse data content with appropriate parser
            auto [message_type, success, result] = stub->parse(data);
            ESP_LOGI("OBSReplyParser", "stub replies with message %s", result.c_str());
            return success & this->db->publish(message_type, result);
        }
        return true;
    }

}
