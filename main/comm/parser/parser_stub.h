/** \file parser_stub.h
 *  \brief Header file for parser stub base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <memory>
#include "esp_log.h"
#include "../data_broker.h"

namespace eobsws::comm::parser {

    /// Type issued by parser stub when processing data
    using ParserTuple = std::tuple<MessageType, bool, std::string>;

    /** \var static const ParserTuple DefaultParserTuple
     *  \brief Default ParserTuple value.
     */
    static const ParserTuple DefaultParserTuple = {MessageType::NoOutlet, false, ""};

    /** \fn static ParserTuple parser_message(MessageType t, bool success, const std::string & message)
     *  \brief Generate a ParserTuple from given arguments.
     *  \param t: message type.
     *  \param success (optional, default=true): true for normal message, false for error message.
     *  \param message (optional, default=""): message payload
     *  \returns compiled ParserTuple.
     */
    static ParserTuple parser_message(MessageType t, bool success = true, const std::string & message = "") {
        return ParserTuple{t, success, message};
    }
    /** \fn static ParserTuple parser_error(MessageType t, const std::string & reason)
     *  \brief Generate a ParserTuple representing an error message from given arguments.
     *  \param t: message type.
     *  \param reason (optional, default=""): error reason
     *  \returns compiled ParserTuple.
     */
    static ParserTuple parser_error(MessageType t, const std::string & reason = "") {
        return parser_message(t, false, reason);
    }

    /** \class ParserStub
     *  \brief This class is meant to implement parsing operations.
     *  ParserStub instances are registered with a Parser instance,
     *  which provides data to be parsed, and return data to be published
     *  as a result.
     */
    class ParserStub {
    protected:
        /** \property std::string command
         *  \brief Command treated by parser stub.
         */
        std::string command;

        /** \property MessageType parser_message_type
         *  \brief Message type issued by parser stub. This can be
         *  modified with set_message_type.
         */
        MessageType parser_message_type = MessageType::NoOutlet;

    public:
        /** \fn virtual ParserTuple parse(const std::string & data)
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        virtual ParserTuple parse(const std::string & data) = 0;

        /** \fn virtual bool can_handle_command(const std::string & cmd)
         *  \brief Tell if parser stub can handle command.
         *  \param cmd: command.
         *  \returns true if stub can handle command, false otherwise.
         */
        virtual bool can_handle_command(const std::string & cmd) {
            ESP_LOGI("ParserStub", "takes command %s, received %s", this->command.c_str(), cmd.c_str());
            return this->command == cmd;
        }

        /** \fn virtual void abort()
         *  \brief Abort current command chain.
         */
        virtual void abort() = 0;

        /** \fn void set_message_type(MessageType t)
         *  \brief Set output message type.
         */
        void set_message_type(MessageType t) { this->parser_message_type = t; }
    };


}
