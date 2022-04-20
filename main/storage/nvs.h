/** \file nvs.h
 *  \brief Header file for NVS partition manager.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "nvs_handle.hpp"
#include "esp_err.h"
#include "esp_log.h"

namespace eobsws::storage {

    /** \typedef ItemType
     *  \brief Shorthand for nvs::ItemType
     */
    using ItemType = ::nvs::ItemType;

    /** \typedef NVSHandle
     *  \brief Shorthand for nvs::NVSHandle
     */
    using NVSHandle = ::nvs::NVSHandle;

    /** \class NVStorage
     *  \brief Class providing access to a non-volatile storage partition.
     */
    class NVStorage {
    private:
        /** \property bool initialized
         *  \brief True if partition has been initialized, false otherwise.
         */
        bool initialized = false;

        /** \property std::string part_name
         *  \brief Partition name.
         */
        std::string part_name;

    public:
        /** \fn NVStorage(const std::string & part_name)
         *  \brief Constructor.
         *  \param part_name: name of partition to open.
         */
        NVStorage(const std::string & part_name);

        /** \fn ~NVStorage()
         *  \brief Destructor.
         */
        ~NVStorage();

        /** \fn bool is_initialized
         *  \brief Tell if storage is initialized.
         *  \returns true if partition has been initialized, false otherwise.
         */
        bool is_initialized() { return this->initialized; }
        
        /** \fn std::unique_ptr<NVSHandle> open_namespace(const std::string & ns, const nvs_open_mode_t open_mode, esp_err_t & err)
         *  \brief Open namespace on current partition.
         *  \param ns: namespace to open.
         *  \param open_mode: opening mode (NVS_READONLY, NVS_READWRITE).
         *  \param err: container for return error code.
         *  \returns namespace handle if successful, nullptr otherwise.
         */
        std::unique_ptr<NVSHandle> open_namespace(const std::string & ns, const nvs_open_mode_t open_mode, esp_err_t & err);

        /** \fn ItemType get_type(const std::string & ns, const std::string & key)
         *  \brief Get type of given key in given namespace.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \returns key type.
         */
        ItemType get_type(const std::string & ns, const std::string & key);

        /** \fn std::string get_string(const std::string & ns, const std::string & key, const std::string & def)
         *  \brief Get string from given key in given namespace.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \param def: default value.
         *  \returns value read from key, or default value if action failed.
         */
        std::string get_string(const std::string & ns, const std::string & key, const std::string & def);
        /** \fn std::string get_string(const std::string & ns, const std::string & key)
         *  \brief Get string from given key in given namespace.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \returns value read from key, or empty string if action failed.
         */
        std::string get_string(const std::string & ns, const std::string & key);

        /** \fn bool set_string(const std::string & ns, const std::string & key, const std::string & value)
         *  \brief Set value of given key in given namespace to given string.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \param value: string value.
         *  \returns true if key could be set, false otherwise.
         */
        bool set_string(const std::string & ns, const std::string & key, const std::string & value);

        /** \fn template <class keytype> keytype get_item(const std::string & ns, const std::string & key, keytype def_value)
         *  \brief Get value from given key in given namespace.
         *  \tparam keytype: type of value to read.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \param def_value: default value.
         *  \returns value read from key, or default value if action failed.
         */
        template <typename keytype> keytype get_item(const std::string & ns, const std::string & key, keytype def_value) {
            assert(this->initialized);
            esp_err_t ret;

            std::unique_ptr<NVSHandle> nvs_handle = this->open_namespace(ns, NVS_READONLY, ret);
            if (ret != ESP_OK) {
                ESP_LOGI("NVStorage::get_item", "cannot access namespace '%s'. Error: 0x%x", ns.c_str(), ret);
                return def_value;
            }

            keytype value;
            ret = nvs_handle->get_item(key.c_str(), value);
            if (ret != ESP_OK) {
                ESP_LOGI("NVStorage::get_item", "cannot access key '%s'. Error: 0x%x", key.c_str(), ret);
                return def_value;
            }

            return value;
        }
        /** \fn template <typename keytype> keytype get_item(const std::string & ns, const std::string & key)
         *  \brief Get value from given key in given namespace.
         *  \tparam keytype: type of value to read.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \returns value read from key, or default value for key type if action failed.
         */
        template <typename keytype> keytype get_item(const std::string & ns, const std::string & key) {
            return this->get_item(ns, key, keytype{});
        }

        /** \fn bool set_item(const std::string & ns, const std::string & key, const std::string & value, ItemType type)
         *  \brief Set value of given key in given namespace.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \param value: value to set.
         *  \param type: item type.
         *  \returns true if value could be set, false otherwise.
         */
        bool set_item(const std::string & ns, const std::string & key, const std::string & value, ItemType type);
        
        /** \fn template <typename keytype> bool set_item(const std::string & ns, const std::string & key, const keytype & value)
         *  \brief Set value of given key in given namespace.
         *  \tparam keytype: type of value to read.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \param value: value to set.
         *  \returns true if value could be set, false otherwise.
         */
        template <typename keytype> bool set_item(const std::string & ns, const std::string & key, const keytype & value) {
            auto stored_type = this->get_type(ns, key);
            ::nvs::ItemType value_type = ::nvs::itemTypeOf(value);

            if (value_type == ItemType::ANY) {
                ESP_LOGE("NVStorage::set_item", "invalid type.");
                return false;
            }

            if (stored_type != value_type && stored_type != ItemType::ANY) {
                ESP_LOGE("NVStorage::set_item", "value type (%d) differs from stored type (%d).", static_cast<int>(value_type), static_cast<int>(stored_type));
                return false;
            }

            esp_err_t ret;
            std::unique_ptr<NVSHandle> nvs_handle = this->open_namespace(ns, NVS_READWRITE, ret);
            if (ret != ESP_OK) {
                ESP_LOGE("NVStorage::set_item", "cannot access namespace '%s'. Error: 0x%x", ns.c_str(), ret);
                return false;
            }

            ret = nvs_handle->set_item(key.c_str(), value);
            if (ret != ESP_OK) {
                ESP_LOGE("NVStorage::set_item", "cannot write to key '%s'. Error: 0x%x", key.c_str(), ret);
                return false;
            }

            ret = nvs_handle->commit();
            if (ret != ESP_OK) {
                ESP_LOGE("NVStorage::set_item", "Couldn't commit to flash. Error: 0x%x", ret);
                return false;
            }

            return true;
        }

        /** \fn bool erase_item(const std::string & ns, const std::string & key)
         *  \brief Erase given key in given namespace.
         *  \param ns: namespace.
         *  \param key: key name.
         *  \returns true if key could be erased, false otherwise.
         */
        bool erase_item(const std::string & ns, const std::string & key);

    };
}

