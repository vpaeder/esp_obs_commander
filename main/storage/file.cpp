/** \file file.cpp
 *  \brief File management class implementation.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "file.h"
#include "esp_log.h"

namespace eobsws::storage {
    
    File::File(std::shared_ptr<Partition> partition, const std::string & file_path) {
        ESP_LOGI("File", "calling constructor for %x", (int)this);
        this->partition = partition;
        this->file_path = file_path;
    }

    File::~File() {
        ESP_LOGI("File", "calling destructor for %x", (int)this);
    }

    bool File::open(const char * mode) {
        ESP_LOGI("File", "opening file for %x in mode %s", (int)this, mode);
        if (!this->partition) {
            ESP_LOGI("File", "invalid partition.");
            return false;
        }
        if (!(this->partition->is_mounted())) {
            ESP_LOGI("File", "partition for %x not mounted.", (int)this);
            return false;
        }
        
        if (this->fd != nullptr) {
            ESP_LOGI("File", "file %x already opened.", (int)this);
            return false;
        }
        
        auto full_path = this->partition->get_full_path(this->file_path);
        ESP_LOGI("File", "absolute file path is '%s'", full_path.c_str());

        this->fd = FilePtrType(fopen(full_path.c_str(), mode), FileCloser());
        ESP_LOGI("File", "file %x got file descriptor %x.", (int)this, (int)(this->fd.get()));
        return (this->fd != nullptr);
    }

    bool File::close() {
        if (this->fd != nullptr) {
            ESP_LOGI("File", "closing file for %x; descriptor %x", (int)this, int(this->fd.get()));
            int ret = fclose(this->fd.get());
            if (ret != 0) return false;
            return true;
        }
        return false;
    }

    size_t File::write(const std::string & data) const {
        if (!(this->is_open() ) ) return -1;
        ESP_LOGI("File", "writing %d bytes of data to file %x", data.size(), (int)this);
        return fwrite(data.c_str(), sizeof(char), data.size(), this->fd.get());
    }

    std::string File::read(size_t len) const {
        if ( !(this->is_open()) ) return std::string{};
        // if len == 0, take file size
        if (len == 0) len = this->get_size();
        std::string result(len, '\0');
        ESP_LOGI("File", "reading %d bytes of data from file %x", len, (int)this);
        if (fread(&result[0], sizeof(char), len, this->fd.get()) != -1) {
            result.shrink_to_fit();
            return result;
        }
        return std::string{};
    }

    size_t File::get_size() const {
        if (!(this->is_open() ) ) return -1;
        ESP_LOGI("File", "getting size of file %x", (int)this);
        fpos_t curpos;
        fgetpos(this->fd.get(), &curpos);
        fseek(this->fd.get(), 0L, SEEK_END);
        size_t size = ftell(this->fd.get());
        fsetpos(this->fd.get(), &curpos);
        return size;
    }

    int File::get_pos() const {
        if (!(this->is_open() ) ) return -1;
        fpos_t curpos;
        fgetpos(this->fd.get(), &curpos);
        return curpos;
    }

    bool File::seek(long int offset, int origin) const {
        if (!(this->is_open() ) ) return false;
        ESP_LOGI("File", "moving to position %d in file with descriptor 0x%x", (int)offset, (int)this->fd.get());
        return fseek(this->fd.get(), offset, origin) == 0;
    }

    bool File::is_open() const {
        return (this->fd != nullptr);
    }
}