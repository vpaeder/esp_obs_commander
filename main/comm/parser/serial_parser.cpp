/** \file serial_parser.cpp
 *  \brief Implementation file for serial parser.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "storage/file.h"
#include "storage/dir.h"
#include "serial_parser.h"
#include "util.h"

#include <iostream>
#include "esp_log.h"


namespace eobsws::comm::parser {

    SerialParser::SerialParser(std::shared_ptr<DataBroker> db) : Parser(db) {
        this->in_message_type = MessageType::InboundWired;
        this->out_message_type = MessageType::OutboundWired;
        // subscribe callback to data broker
        this->db->subscribe(this->convert_callback<SerialParser>(this));
    }

    bool SerialParser::publish_callback(MessageType t, const std::string & data) {
        if ((t & this->in_message_type) == MessageType::NoOutlet) {
            ESP_LOGI("SerialParser", "message of type %d rejected. Expected %d", static_cast<int>(t), static_cast<int>(this->in_message_type));
            return false;
        }
        ESP_LOGI("SerialParser", "processing message of type %d", static_cast<int>(t));
        ESP_LOGI("SerialParser", "received %s", data.c_str());
        // get command; we expect a data string starting with an AT command such as AT+PUTFILE=...
        auto eq_pos = data.find_first_of("=");
        auto cmd = data.substr(0, eq_pos);
        auto content = data.substr(eq_pos+1);
        if (cmd == serial::ATCommand::Abort) {
            ESP_LOGI("SerialParser", "%s command received.", serial::ATCommand::Abort.c_str());
            this->abort_stubs();
            this->db->publish(this->out_message_type, serial::ATReply::Ok);
            return true;
        }
        // clean up parsers from dangling pointers
        this->clean_up_stubs();
        // check which parser takes command
        ESP_LOGI("SerialParser", "searching for parser for command %s.", cmd.c_str());
        auto stub = this->find_stub_for_command(cmd);
        if (stub != nullptr) {
            // parse data content with appropriate parser
            ESP_LOGI("SerialParser", "found parser for command %s.", cmd.c_str());
            ESP_LOGI("SerialParser", "argument: %s", content.c_str());
            auto [message_type, success, result] = stub->parse(content);
            return success & this->db->publish(message_type, result);
        }
        ESP_LOGI("SerialParser", "no parser found for command %s.", cmd.c_str());
        this->db->publish(this->out_message_type, serial::ATReply::Unknown);
        return false;
    }

}
