/** \file data_broker.h
 *  \brief Header file for data broker class. This class handles communication
 *  between pipes (USB, WiFi, Bluetooth ...), parsing routines, display routines,
 *  and event routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "esp_log.h"

namespace eobsws::comm {

    /** \enum MessageType
     *  \brief Message types available to subscribers.
     */
    enum class MessageType : uint16_t {
        NoOutlet = 0, /**< not meant for any outlet */
        InboundAny = 1 | (1 << 1), /**< any inbound message */
        OutboundAny = (1 << 2) | (1 << 3), /**< any outbound message */
        InboundWired = 1, /**< wired inbound message */
        OutboundWired = (1 << 2), /**< wired outbound message */
        InboundWireless = (1 << 1), /**< wireless inbound message */
        OutboundWireless = (1 << 3), /**< wireless outbound message */
        Event = (1 << 4) /**< event message */
    };

    /** \fn static bool operator&(MessageType & m1, MessageType & m2)
     *  \brief Boolean & operator for MessageType arguments.
     *  \param m1,m2: operands.
     *  \returns MessageType resulting from intersection of m1 and m2.
     */
    static MessageType operator&(MessageType & m1, MessageType & m2) {
        return static_cast<MessageType>(static_cast<uint16_t>(m1) & static_cast<uint16_t>(m2));
    }

    /** \class PublisherTemplate
     *  \brief Base template for publisher class.
     *  It defines a publisher issuing data in a form given
     *  by template's variadic arguments.
     *  \param Args: arguments accepted by publish function.
     */
    template <typename... Args> class PublisherTemplate {
    private:
        /** \typedef FuncType
         *  \brief Function signature expected by data broker.
         */
        using FuncType = std::shared_ptr<std::function<bool(Args...)> >;

        /** \property std::mutex mtx
         *  \brief Mutex for thread-safe operation.
         */
        std::mutex mtx;

        /** \property std::vector<FuncType> callbacks
         *  \brief Set of callbacks registered with publisher instance.
         *  I chose std::vector here for the following reasons:
         *   - data traversal is O(1) for iterators anyway
         *   - smallest memory overhead
         *   - I probably don't need to re-order the vector frequently
         *   - I don't have many callbacks
         *   - while it has a bigger footprint (bigger binary), it's negligible
         *  In order of binary size, we have: unordered_set -> unordered_list -> vector
         *  In order of memory usage, same list but reversed.
         */
        std::vector<FuncType> callbacks;
        
    public:
        /** \fn ~PublisherTemplate()
         *  \brief Destructor.
         */
        ~PublisherTemplate() {
            mtx.unlock();
        }

        /** \fn void subscribe(FuncType callback)
         *  \brief Subscribe to publisher with given callback.
         *  \param callback: callback function to subscribe.
         */
        void subscribe(FuncType callback) {
            const std::lock_guard<std::mutex> lock(this->mtx);
            this->callbacks.emplace_back(callback);
        }

        /** \fn bool publish(Args... args)
         *  \brief Publish data to registered callbacks.
         *  \param args: arguments as defined by template specialization
         */
        bool publish(Args... args) {
            ESP_LOGI("DataBroker", "forwarding data to callbacks");
            for (auto & cb: this->callbacks) {
                ESP_LOGI("DataBroker", "trying callback 0x%x", reinterpret_cast<int>(cb.get()));
                if (cb.get()!=nullptr && (*cb)(args...)) {
                    ESP_LOGI("DataBroker", "success with callback 0x%x", reinterpret_cast<int>(cb.get()));
                    return true;
                }
            }
            ESP_LOGI("DataBroker", "data not accepted by any node!");
            return false;
        }

    };

    /** \class DataBroker
     *  \brief Data broker class.
     *  This is a specialization of PublisherTemplate with MessageType and std::string arguments.
     */
    class DataBroker : public PublisherTemplate<MessageType, const std::string &> {};

}
