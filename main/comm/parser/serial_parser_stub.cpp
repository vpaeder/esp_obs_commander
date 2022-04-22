/** \file serial_parser_stub.cpp
 *  \brief Implementation file for serial command parsers.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "serial_parser_stub.h"
#include "storage/file.h"
#include "storage/dir.h"
#include "storage/nvs.h"
#include "util.h"
#include <mbedtls/base64.h>
#include <sstream>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_log.h"

namespace eobsws::comm::parser::serial {

    /** \var static const char SerialTermination
     *  \brief Serial termination character.
     */
    static const std::string SerialTermination = "\r";
    
    const std::string ATCommand::GetBufferSize = "AT+GETBUFS";
    const std::string ATCommand::GetFirmwareVersion = "AT+GETFWVER";
    const std::string ATCommand::Abort = "AT+ABORT";
    const std::string ATCommand::PutFile = "AT+PUTFILE";
    const std::string ATCommand::GetFile = "AT+GETFILE";
    const std::string ATCommand::PutData = "AT+PUTDATA";
    const std::string ATCommand::GetData = "AT+GETDATA";
    const std::string ATCommand::ListDir = "AT+LISTDIR";
    const std::string ATCommand::NextFile = "AT+NEXTFILE";
    const std::string ATCommand::MakeDir = "AT+MAKEDIR";
    const std::string ATCommand::Delete = "AT+DELETE";
    const std::string ATCommand::SetConf = "AT+SETCONF";
    const std::string ATCommand::GetConf = "AT+GETCONF";
    const std::string ATCommand::DelConf = "AT+DELCONF";
    
    const std::string ATReply::Ok = "OK" + SerialTermination;
    const std::string ATReply::Error = "ERROR" + SerialTermination;
    const std::string ATReply::Busy = "BUSY" + SerialTermination;
    const std::string ATReply::Unknown = "UNKN" + SerialTermination;
    const std::string ATReply::Size = "SIZE";
    const std::string ATReply::Data = "DATA";
    const std::string ATReply::NumFiles = "NUMFILES";
    const std::string ATReply::File = "FILE";
    const std::string ATReply::Value = "VALUE";
    const std::string ATReply::BufferSize = "BUFS";
    const std::string ATReply::FirmwareVersion = "FWVER";


    /** \fn static inline size_t compute_b64_length(const size_t & len)
     *  \brief Computes the equivalent length of a string encoded in base64.
     *  \param len: length of the decoded string.
     *  \returns the length of the base64-encoded string.
     */
    static inline size_t compute_b64_length(const size_t & len) {
        return 4*(len + (3-len%3)%3)/3;
    }

    /** \fn static std::string bytes_to_b64(const std::string & data)
     *  \brief Encodes a byte string in byte64.
     *  \param data: string to encode.
     *  \returns encoded string.
     */
    static std::string bytes_to_b64(const std::string & data) {
        auto blen = compute_b64_length(data.size()) + 1;
        std::vector<unsigned char> encoded(blen);
        size_t encoded_length;
        mbedtls_base64_encode(encoded.data(), blen, &encoded_length,
                              reinterpret_cast<const unsigned char*>(data.c_str()),
                              data.size());
        return std::string(reinterpret_cast<char*>(encoded.data()), encoded_length);
    }

    /** \fn static std::string b64_to_bytes(const std::string & data)
     *  \brief Decodes a byte64-encoded string.
     *  \param data: string to decode.
     *  \returns decoded string.
     */
    static std::string b64_to_bytes(const std::string & data) {
        auto blen = data.size()/4*3;
        std::vector<unsigned char> decoded(blen);
        size_t decoded_length;
        mbedtls_base64_decode(decoded.data(), blen, &decoded_length,
                              reinterpret_cast<const unsigned char*>(data.c_str()),
                              data.size());
        return std::string(reinterpret_cast<char*>(decoded.data()), decoded_length);
    }


    bool FileParserStub::open_file(const std::string & data, const char * mode) {
        auto file_name = trim_string(data);
        this->file = partition->open(file_name, mode);
        return (this->file != nullptr);
    }


    ParserTuple PutFileParserStub::parse(const std::string & data) {
        switch (this->phase) {
            case 0: // "open file" phase
            {
                auto [file_name, file_len_str] = split_first(data, ",");
                if (!this->open_file(file_name, "wb")) break;
                if (!is_numeric(file_len_str)) break;
                this->remaining_bytes = stoi(file_len_str);
                this->phase = 1;
                this->command = ATCommand::PutData;
                return parser_message(this->parser_message_type, true, ATReply::Ok);
                break;
            }
            case 1: // "put data to file" phase
            {
                if (data.size() % 4) {
                    // this is when data size won't fit in b64 decoder
                    return parser_message(this->parser_message_type, false, ATReply::Error);
                }
                auto decoded = b64_to_bytes(data);
                size_t written = this->file->write(decoded);
                this->remaining_bytes -= data.size();
                if (written != decoded.size()) {
                    this->abort(); // cannot write required amount, aborting.
                    break;
                }
                if (this->remaining_bytes <= 0)
                    this->abort(); // this closes file

                return parser_message(this->parser_message_type, true, ATReply::Ok);
                break;
            }
        }
        return parser_message(this->parser_message_type, false, ATReply::Error);
    }

    ParserTuple GetFileParserStub::parse(const std::string & data) {
        switch (this->phase) {
            case 0: // "open file" phase
            {
                auto file_name = trim_string(data);
                if (!this->open_file(file_name, "rb")) break;
                this->remaining_bytes = compute_b64_length(this->file->get_size());
                this->phase = 1;
                this->command = ATCommand::GetData;
                return parser_message(this->parser_message_type, true,
                                      ATReply::Size + "="
                                      + std::to_string(this->remaining_bytes)
                                      + SerialTermination);
                break;
            }
            case 1: // "get data from file" phase
            {
                if (!is_numeric(data)) break;
                // requested size, up to remaining number of bytes
                auto nb64 = std::min(static_cast<size_t>(stoi(data)), this->remaining_bytes);
                // number of b64 bytes must be divisible by 4
                if (nb64 % 4) break;
                auto nbytes = nb64/4*3; // number of raw bytes
                auto bytes = bytes_to_b64(this->file->read(nbytes));
                this->remaining_bytes -= nb64;
                if (bytes.size() != nb64) {
                    this->abort(); // cannot read required amount, aborting.
                    break;
                }
                if (this->remaining_bytes <= 0)
                    this->abort(); // this closes file
                
                return parser_message(this->parser_message_type, true,
                                      ATReply::Data + "=" + bytes + SerialTermination);
                break;
            }
        }
        return parser_message(this->parser_message_type, false, ATReply::Error);
    }


    ParserTuple ListDirParserStub::parse(const std::string & data) {
        switch (this->phase) {
            {
            case 0: // "open dir" phase
                auto dir_name = trim_string(data);
                this->dir = this->partition->opendir(dir_name);
                if (this->dir == nullptr) break;
                this->remaining_files = this->dir->get_num_files();
                this->phase = 1;
                this->command = ATCommand::NextFile;
                return parser_message(this->parser_message_type, true,
                                      ATReply::NumFiles + "="
                                      + std::to_string(this->remaining_files)
                                      + SerialTermination);
                break;
            }
            case 1: // "get next file name" phase
            {
                uint8_t file_type;
                std::string file_name;
                this->dir->get_file_info(-1, file_name, file_type);
                this->remaining_files--;
                if (this->remaining_files == 0)
                    this->abort(); // this closes dir
                return parser_message(this->parser_message_type, true,
                                      ATReply::File + "="
                                      + file_name + ","
                                      + std::to_string(file_type)
                                      + SerialTermination);
                break;
            }
        }
        return parser_message(this->parser_message_type, false, ATReply::Error);
    }

    ParserTuple DeleteFileParserStub::parse(const std::string & data) {
        auto file_name = trim_string(data);
        if (this->partition->remove(file_name))
            return {this->parser_message_type, true, ATReply::Ok};
        return parser_message(this->parser_message_type, false, ATReply::Error);
    }

    ParserTuple MakedirParserStub::parse(const std::string & data) {
        auto dir_name = trim_string(data);
        if (this->partition->makedir(dir_name))
            return {this->parser_message_type, true, ATReply::Ok};
        return parser_message(this->parser_message_type, false, ATReply::Error);
    }

    ParserTuple SetConfigParserStub::parse(const std::string & data) {
        std::string ns, key, rdata, rrdata, type_str, value_str;
        std::tie(ns, rdata) = split_first(data, ",");
        std::tie(key, rrdata) = split_first(rdata, ",");
        std::tie(type_str, value_str) = split_first(rrdata, ",");
        auto err = parser_message(this->parser_message_type, false, ATReply::Error);
        //ESP_LOGE("SetConfigParserStub", "split values: %s, %s, %s, %s", ns.c_str(), key.c_str(), );

        if (ns == "" || key == "" || type_str == "" || value_str == "") {
            ESP_LOGE("SetConfigParserStub", "one of the parameters is missing.");
            return err;
        }
        
        if (!is_numeric(type_str)) {
            ESP_LOGE("SetConfigParserStub", "type parameter must be numeric.");
            return err;
        }

        int type = stoi(type_str);
        if (auto stored_type = static_cast<int>(this->partition->get_type(ns, key));
            stored_type != NVS_TYPE_ANY && type != stored_type) {
                ESP_LOGE("SetConfigParserStub", "provided type differs from stored type.");
                return err;
            }

        if (type == NVS_TYPE_STR || type == NVS_TYPE_BLOB) {
            if (this->partition->set_string(ns, key, value_str))
                return parser_message(this->parser_message_type, false, ATReply::Ok);
            return err;
        }

        // numeric type
        if (this->partition->set_item(ns, key, value_str, static_cast<storage::ItemType>(type)))
            return parser_message(this->parser_message_type, true, ATReply::Ok);
        
        ESP_LOGE("SetConfigParserStub", "cannot assign value to key.");
        return err;
    }

    ParserTuple GetConfigParserStub::parse(const std::string & data) {
        auto [ns, key] = split_first(data, ",");
        ParserTuple err = parser_message(this->parser_message_type, false, ATReply::Error);
        if (key == "" || ns == "")
            return err;
        
        auto type = this->partition->get_type(ns, key);
        std::ostringstream result;
        result << ATReply::Value << "=" << static_cast<int>(type) << ",";
        
        if (type == storage::ItemType::SZ
            || type == storage::ItemType::BLOB
            || type == storage::ItemType::BLOB_DATA) {
            result << this->partition->get_string(ns, key);
        } else {
            switch (type) {
            case storage::ItemType::U8:
                result << this->partition->get_item<uint8_t>(ns, key);
                break;
            case storage::ItemType::I8:
                result << this->partition->get_item<int8_t>(ns, key);
                break;
            case storage::ItemType::U16:
                result << this->partition->get_item<uint16_t>(ns, key);
                break;
            case storage::ItemType::I16:
                result << this->partition->get_item<int16_t>(ns, key);
                break;
            case storage::ItemType::U32:
                result << this->partition->get_item<uint32_t>(ns, key);
                break;
            case storage::ItemType::I32:
                result << this->partition->get_item<int32_t>(ns, key);
                break;
            case storage::ItemType::U64:
                result << this->partition->get_item<uint64_t>(ns, key);
                break;
            case storage::ItemType::I64:
                result << this->partition->get_item<int64_t>(ns, key);
                break;
            default:
                break;
            }
        }
        result << SerialTermination;
        return parser_message(this->parser_message_type, true, result.str());
    }

    ParserTuple DelConfigParserStub::parse(const std::string & data) {
        auto [ns, key] = split_first(data, ",");
        auto err = parser_message(this->parser_message_type, false, ATReply::Error);
        if (key == "" || ns == "")
            return err;
        
        if (this->partition->erase_item(ns, key)) {
            return parser_message(this->parser_message_type, true, ATReply::Ok);
        } else {
            return err;
        }
    }


    ParserTuple GetBufSizeParserStub::parse(const std::string & data) {
        return parser_message(this->parser_message_type, true,
                              ATReply::BufferSize + "="
                              + std::to_string(CONFIG_UART_BUF_SIZE)
                              + SerialTermination);
    }


    ParserTuple GetFirmwareVersionParserStub::parse(const std::string & data) {
        return parser_message(this->parser_message_type, true,
                              ATReply::FirmwareVersion + "="
                              + CONFIG_ECTRL_FIRMWARE_VERSION
                              + SerialTermination);
    }
    
}
