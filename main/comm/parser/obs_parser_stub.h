/** \file obs_parser_stub.h
 *  \brief Header file for obs-websocket command parsers.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <sstream>
#include <memory>

#include "parser_stub.h"

/** \namespace eobsws::comm::parser::obs
 *  \brief obs-websocket message parser stubs.
 */
namespace eobsws::comm::parser::obs {

    /** \enum Opcode
     *  \brief Opcodes from obs-websocket 5.0.0 protocol
     */
    enum class Opcode : uint8_t {
        Hello = 0, /**< server 'hello' message */
        Identify = 1, /**< send identification data */
        Identified = 2, /**< server tells client is identified */
        Reidentify = 3, /**< send re-identification request to server */
        Event = 5, /**< server-issued event */
        Request = 6, /**< send request to server */
        RequestResponse = 7, /**< server reply to request */
        RequestBatch = 8, /**< send batch request to server */
        RequestBatchResponse = 9 /**< server reply to batch request */
    };

    /** \fn static std::string to_string(const Opcode & op)
     *  \brief Conversion function from Opcode to string.
     *  \param op: opcode
     *  \returns stringified opcode.
     */
    static std::string to_string(const Opcode & op) {
        return std::to_string(static_cast<uint8_t>(op));
    }

    /** \fn std::string add_request_id(const std::string & req)
     *  \brief Adds a request ID to a request message.
     *  \param req: request to add ID to. This must be a string parsable to JSON.
     *  \returns modified string.
     */
    std::string add_request_id(const std::string & req);

    /** \class OBSHello
     *  \brief Class for parsing 'Hello' messages (opcode 0).
     */
    class OBSHello : public ParserStub {
    private:
        /** \property std::string password
         *  \brief Password string for authentication.
         */
        std::string password;

    public:
        /** \fn OBSHello(const std::string & password)
         *  \brief Constructor.
         *  \param password: password string.
         */
        OBSHello(const std::string & password = "") : password(password) { this->command = to_string(Opcode::Hello); }

        /** \fn ParserTuple parse(const std::string & data) override
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        ParserTuple parse(const std::string & data) override;

        /** \fn std::string authenticate(const std::string & challenge, const std::string & salt)
         *  \brief Create authentication string.
         *  \param challenge: challenge string provided by server.
         *  \param salt: encryption salt string provided by server.
         *  \returns authentication string.
         */
        std::string authenticate(const std::string & challenge, const std::string & salt);

        /** \fn void abort()
         *  \brief Abort current command chain.
         */
        void abort() override {};
    };


    /** \class OBSIdentified
     *  \brief Class for parsing 'Identified' messages (opcode 2).
     */
    class OBSIdentified : public ParserStub {
    public:
        /** \fn OBSIdentified()
         *  \brief Constructor.
         */
        OBSIdentified() { this->command = to_string(Opcode::Identified); }

        /** \fn ParserTuple parse(const std::string & data) override
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        ParserTuple parse(const std::string & data) override;

        /** \fn void abort()
         *  \brief Abort current command chain.
         */
        void abort() override {};
    };


    /** \class OBSEvent
     *  \brief Class for parsing 'Event' messages (opcode 5).
     */
    class OBSEvent : public ParserStub {
    public:
        /** \fn OBSEvent()
         *  \brief Constructor.
         */
        OBSEvent() { this->command = to_string(Opcode::Event); }

        /** \fn ParserTuple parse(const std::string & data) override
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        ParserTuple parse(const std::string & data) override;
        
        /** \fn void abort()
         *  \brief Abort current command chain.
         */
        void abort() override {};
    };


    /** \class OBSRequestResponse
     *  \brief Class for parsing 'RequestResponse' messages (opcode 7).
     */
    class OBSRequestResponse : public ParserStub {
    public:
        /** \fn OBSRequestResponse()
         *  \brief Constructor.
         */
        OBSRequestResponse() { this->command = to_string(Opcode::RequestResponse); }

        /** \fn ParserTuple parse(const std::string & data) override
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        ParserTuple parse(const std::string & data) override;
        
        /** \fn void abort()
         *  \brief Abort current command chain.
         */
        void abort() override {};
    };


    /** \class OBSRequestBatchResponse
     *  \brief Class for parsing 'RequestBatchResponse' messages (opcode 9).
     */
    class OBSRequestBatchResponse : public ParserStub {
    public:
        /** \fn OBSRequestBatchResponse()
         *  \brief Constructor.
         */
        OBSRequestBatchResponse() { this->command = to_string(Opcode::RequestBatchResponse); }

        /** \fn ParserTuple parse(const std::string & data) override
         *  \brief Parse given data and return result.
         *  \param data: data to parse.
         *  \returns result compiled as ParserTuple.
         */
        ParserTuple parse(const std::string & data) override;
        
        /** \fn void abort()
         *  \brief Abort current command chain.
         */
        void abort() override {};
    };

}
