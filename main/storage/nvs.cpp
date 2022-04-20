/** \file nvs.cpp
 *  \brief NVS partition manager implementation.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "nvs_flash.h"
#include "nvs_handle.hpp"
#include "esp_log.h"
#include <cstring>

#include "util.h"
#include "nvs.h"

namespace eobsws::storage {

    NVStorage::NVStorage(const std::string & part_name) {
        this->initialized = false;
        esp_err_t err = nvs_flash_init_partition(part_name.c_str());
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            err = nvs_flash_erase_partition(part_name.c_str());
            if (err != ESP_OK)
                return;
            err = nvs_flash_init_partition(part_name.c_str());
            if (err != ESP_OK)
                return;
        }
        this->part_name = part_name;
        this->initialized = true;
    }


    NVStorage::~NVStorage() {
        if (this->initialized)
            nvs_flash_deinit_partition(this->part_name.c_str());
    }


    std::unique_ptr<NVSHandle> NVStorage::open_namespace(const std::string & ns, const nvs_open_mode_t open_mode, esp_err_t & err) {
        if (!this->initialized) return nullptr;
        return ::nvs::open_nvs_handle_from_partition(this->part_name.c_str(), ns.c_str(), NVS_READWRITE, &err);
    }

    ItemType NVStorage::get_type(const std::string & ns, const std::string & key) {
        nvs_iterator_t it = nvs_entry_find(this->part_name.c_str(), ns.c_str(), NVS_TYPE_ANY);
        if (it == nullptr) {
            ESP_LOGI("NVStorage::get_type", "Namespace '%s' couldn't be found.", ns.c_str());
            return ItemType::ANY;
        }
        nvs_entry_info_t info;
        while (it != nullptr) {
            nvs_entry_info(it, &info);
            if (!strcmp(info.key, key.c_str())) break;
            it = nvs_entry_next(it);
        }

        if (it == nullptr) {
            ESP_LOGI("usb_cmd_config_get_type", "Key '%s' couldn't be found.", key.c_str());
            return ItemType::ANY;
        }
        nvs_release_iterator(it);

        return static_cast<ItemType>(info.type);
    }
    

    std::string NVStorage::get_string(const std::string & ns, const std::string & key) {
        return this->get_string(ns, key, "");
    }
    std::string NVStorage::get_string(const std::string & ns, const std::string & key, const std::string & def) {
        assert(this->initialized);
        esp_err_t ret;

        std::unique_ptr<nvs::NVSHandle> nvs_handle = this->open_namespace(ns, NVS_READONLY, ret);
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::get_string", "cannot open NVS namespace '%s' on volume '%s'. Error: 0x%x", ns.c_str(), CONFIG_NVS_VOLUME_NAME, ret);
            return def;
        }
        
        size_t size;
        ret = nvs_handle->get_item_size(ItemType::SZ, key.c_str(), size);
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::get_string", "cannot access key '%s'. Error: 0x%x", key.c_str(), ret);
            return def;
        }

        char * value = (char*)calloc(size, sizeof(char));
        ret = nvs_handle->get_string(key.c_str(), value, size);
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::get_string", "cannot access key '%s'. Error: 0x%x", key.c_str(), ret);
            return def;
        }

        std::string result = std::string(value);
        free(value);
        
        return result;
    }


    bool NVStorage::set_string(const std::string & ns, const std::string & key, const std::string & value) {
        esp_err_t ret;
        std::unique_ptr<nvs::NVSHandle> nvs_handle = this->open_namespace(ns, NVS_READWRITE, ret);
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::set_string", "cannot access namespace '%s'. Error: 0x%x", ns.c_str(), ret);
            return false;
        }
        if (value.size() < 4000) {
            ret = nvs_handle->set_string(key.c_str(), value.c_str());
        } else if (value.size() < 508000) {
            ret = nvs_handle->set_blob(key.c_str(), value.c_str(), value.size());
        } else {
            ESP_LOGI("NVStorage::set_string", "string too large.");
            return false;
        }
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::set_string", "cannot write to key '%s'. Error: 0x%x", key.c_str(), ret);
            return false;
        }
        
        ret = nvs_handle->commit();
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::set_item", "Couldn't commit to flash. Error: 0x%x", ret);
            return false;
        }
        return true;
    }


    bool NVStorage::set_item(const std::string & ns, const std::string & key, const std::string & value, ItemType type) {
        if (type == ItemType::ANY) {
            ESP_LOGE("NVStorage::set_item", "invalid item type.");
            return false;
        }
        if (type == ItemType::SZ || type == ItemType::BLOB_DATA || type == ItemType::BLOB)
            return this->set_string(ns, key, value);
        
        if (!is_numeric(value)) {
            ESP_LOGE("NVStorage::set_item", "numeric type but value is not numeric.");
            return false;
        }
        
        switch (type) {
            case ItemType::U8:
                return this->set_item<uint8_t>(ns, key, stoi(value));
            case ItemType::I8:
                return this->set_item<int8_t>(ns, key, stoi(value));
            case ItemType::U16:
                return this->set_item<uint16_t>(ns, key, stoi(value));
            case ItemType::I16:
                return this->set_item<int16_t>(ns, key, stoi(value));
            case ItemType::U32:
                return this->set_item<uint32_t>(ns, key, stoi(value));
            case ItemType::I32:
                return this->set_item<int32_t>(ns, key, stoi(value));
            case ItemType::U64:
                return this->set_item<uint64_t>(ns, key, stoi(value));
            case ItemType::I64:
                return this->set_item<int64_t>(ns, key, stoi(value));
            default:
            ESP_LOGE("NVStorage::set_item", "invalid type.");
            return false;
        }
    }


    bool NVStorage::erase_item(const std::string & ns, const std::string & key) {
        esp_err_t ret;
        std::unique_ptr<nvs::NVSHandle> nvs_handle = this->open_namespace(ns, NVS_READWRITE, ret);
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::erase_item", "cannot access namespace '%s'. Error: 0x%x", ns.c_str(), ret);
            return false;
        }

        ret = nvs_handle->erase_item(key.c_str());
        
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::erase_item", "Couldn't erase key '%s' in namespace '%s'. Error 0x%x.", key.c_str(), ns.c_str(), ret);
            return false;
        }

        ret = nvs_handle->commit();
        if (ret != ESP_OK) {
            ESP_LOGI("NVStorage::erase_item", "erasure of key '%s' couldn't be committed.", key.c_str());
            return false;
        }

        return true;
    }

}
