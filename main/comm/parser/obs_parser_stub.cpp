/** \file obs_parser_stub.cpp
 *  \brief Implementation file for obs-websocket command parsers.
 *
 *  Author: Vincent Paeder
 *  License: MIT
*/
#include "mbedtls/md.h"
#include <mbedtls/base64.h>
#include <string.h>
#include "cJSON.h"
#include "esp_log.h"

#include "util.h"
#include "obs_parser_stub.h"

namespace eobsws::comm::parser::obs {
    /** \var static const uint8_t rpcVersion
     *  \brief RPC protocol version (must match obs-websocket's version)
     */
    static const uint8_t rpcVersion = 1;

    /** \fn static cJSON * make_payload_template(Opcode opcode)
     *  \brief Compiles a template for a message payload.
     *  \param opcode: message code.
     *  \returns payload as JSON structure.
     */
    static cJSON * make_payload_template(Opcode opcode) {
        cJSON *root, *data;
        root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "op", static_cast<int>(opcode));
        cJSON_AddItemToObject(root, "d", data=cJSON_CreateObject());
        cJSON_AddNumberToObject(data, "rpcVersion", rpcVersion);
        return root;
    }

    
    std::string add_request_id(const std::string & req) {
        cJSON * js = cJSON_Parse(req.c_str());
        auto payload = cJSON_GetObjectItem(js, "d");
        cJSON_AddStringToObject(payload, "requestId", uuid_generate().c_str());
        auto dump = cJSON_Print(js);
        cJSON_Delete(js);
        return std::string(dump);
    }


    ParserTuple OBSHello::parse(const std::string & data) {
        cJSON * js = cJSON_Parse(data.c_str());
        /* Hello message contains:
         * rpcVersion : integer
         * obsWebSocketVersion : string
         * authentication (optional): {
         *      challenge : string
         *      salt : string
         * }
         */
        // check RPC version
        if (!cJSON_HasObjectItem(js, "rpcVersion")) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "RPC version not provided.");
        }
        if (cJSON_GetNumberValue(cJSON_GetObjectItem(js,"rpcVersion")) != rpcVersion) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "RPC version mismatch.");
        }
        // prepare result
        auto res = make_payload_template(Opcode::Identify);
        auto payload = cJSON_GetObjectItem(res, "d");
        cJSON_AddNumberToObject(payload, "eventSubscriptions", 0x7ff);
        // deal with authentication if necessary
        if (cJSON_HasObjectItem(js, "authentication")) {
            auto auth = cJSON_GetObjectItem(js, "authentication");
            if (!cJSON_HasObjectItem(auth, "challenge") || !cJSON_HasObjectItem(auth, "salt")) {
                cJSON_Delete(js);
                cJSON_Delete(res);
                return parser_error(this->parser_message_type, "");
            }
            auto challenge = cJSON_GetStringValue(cJSON_GetObjectItem(auth, "challenge"));
            auto salt = cJSON_GetStringValue(cJSON_GetObjectItem(auth, "salt"));
            cJSON_AddStringToObject(payload, "authentication", this->authenticate(challenge, salt).c_str());
        }
        auto dump = cJSON_Print(res);
        cJSON_Delete(js);
        cJSON_Delete(res);
        return {this->parser_message_type, true, dump};
    }

    std::string OBSHello::authenticate(const std::string & challenge, const std::string & salt) {
        // auth protocol from
        // https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md
        unsigned char hmac_result[32];
        bzero(hmac_result, 32);
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

        auto cnv = [](const char * c) { return reinterpret_cast<const unsigned char*>(c); };
        // prepare string : key = sha256(password + salt)
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
        mbedtls_md_starts(&ctx);
        mbedtls_md_update(&ctx, cnv(password.c_str()), password.size());
        mbedtls_md_update(&ctx, cnv(salt.c_str()), salt.size());
        mbedtls_md_finish(&ctx, hmac_result);
        mbedtls_md_free(&ctx);

        // produce base64 representation of key
        unsigned char encoded[45];
        bzero(encoded, 45);
        size_t encoded_length;
        mbedtls_base64_encode(encoded, 45, &encoded_length, hmac_result, 32);

        // prepare string : sha256(encoded + challenge)
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
        mbedtls_md_starts(&ctx);
        mbedtls_md_update(&ctx, const_cast<unsigned char *>(encoded), encoded_length);
        mbedtls_md_update(&ctx, cnv(challenge.c_str()), challenge.size());
        mbedtls_md_finish(&ctx, hmac_result);
        mbedtls_md_free(&ctx);

        // auth response : base64 of SHA256 hash of encoded + challenge
        unsigned char result[45];
        bzero(result, 45);
        mbedtls_base64_encode(result, 45, &encoded_length, hmac_result, 32);

        ESP_LOGI("OBSHello", "base64 auth: %s", result);

        return std::string(reinterpret_cast<char*>(result), encoded_length);
    }


    ParserTuple OBSIdentified::parse(const std::string & data) {
        cJSON * js = cJSON_Parse(data.c_str());
        /* Identified message contains:
         * negotiatedRpcVersion : integer
         */
        // check RPC version
        if (!cJSON_HasObjectItem(js, "negotiatedRpcVersion")) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "RPC version not provided.");
        }
        if (cJSON_GetNumberValue(cJSON_GetObjectItem(js,"negotiatedRpcVersion")) != rpcVersion) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "RPC version mismatch.");
        }

        cJSON_Delete(js);
        return parser_message(MessageType::NoOutlet, false, "");
    }


    ParserTuple OBSEvent::parse(const std::string & data) {
        cJSON * js = cJSON_Parse(data.c_str());
        /* Identified message contains:
         * eventType : string
         * eventIntent : integer
         * eventData : object
         */
        if (!cJSON_HasObjectItem(js, "eventType") || !cJSON_HasObjectItem(js, "eventIntent")
            || !cJSON_HasObjectItem(js, "eventData")) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "Misformed event message.");
        }

        cJSON_Delete(js);
        return parser_message(this->parser_message_type, true, data);
    }


    ParserTuple OBSRequestResponse::parse(const std::string & data) {
        cJSON * js = cJSON_Parse(data.c_str());
        /* Identified message contains:
         * requestType : string
         * requestId : string
         * responseData : object
         * requestStatus :
         *      result : bool
         *      code : number
         *      comment (optional) : string
         */
        if (!cJSON_HasObjectItem(js, "requestType") || !cJSON_HasObjectItem(js, "requestId")
            || !cJSON_HasObjectItem(js, "responseData") || !cJSON_HasObjectItem(js, "requestStatus")) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "Misformed request reply.");
        }
        
        cJSON_Delete(js);
        return parser_message(this->parser_message_type, true, data);
    }


    ParserTuple OBSRequestBatchResponse::parse(const std::string & data) {
        cJSON * js = cJSON_Parse(data.c_str());
        /* Identified message contains:
         * requestId : string
         * results : array of objects
         */
        if (!cJSON_HasObjectItem(js, "requestId") || !cJSON_HasObjectItem(js, "results")) {
            cJSON_Delete(js);
            return parser_error(this->parser_message_type, "Misformed batch request reply.");
        }
        
        cJSON_Delete(js);
        return parser_message(this->parser_message_type, true, data);
    }

}
