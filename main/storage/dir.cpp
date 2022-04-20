/** \file dir.cpp
 *  \brief Directory management class implementation.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "dir.h"
#include "esp_log.h"

namespace eobsws::storage {

    /** \fn static bool is_file_valid(const struct dirent * entry)
     *  \brief Tells if file is valid.
     *  \param entry: pointer to a directory entry.
     *  \returns true if file is valid, false otherwise.
     */
    static bool is_file_valid(const struct dirent * entry) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            for (auto c = entry->d_name; *c; ++c)
                if (*c >= 127) return false;
            return true;
        }
        return false;
    }
    
    Directory::Directory(std::shared_ptr<Partition> partition, const std::string & dir_path) {
        ESP_LOGI("Directory", "calling constructor for %x", (int)this);
        this->partition = partition;
        this->dir_path = dir_path;
    }

    Directory::~Directory() {
        ESP_LOGI("Directory", "calling destructor for %x", (int)this);
    }

    bool Directory::open() {
        ESP_LOGI("Directory", "opening directory for %x", (int)this);
        if (!(this->partition->is_mounted())) {
            ESP_LOGI("Directory", "partition for %x not mounted.", (int)this);
            this->close();
            return false;
        }
        
        if (this->fd != nullptr) {
            ESP_LOGI("Directory", "directory %x already opened.", (int)this);
            return false;
        }
        
        auto full_path = this->partition->get_full_path(this->dir_path);
        this->fd = DirPtrType(::opendir(full_path.c_str()), DirCloser());
        ESP_LOGI("Directory", "directory %x got directory descriptor %x.", (int)this, (int)(this->fd.get()));
        if (this->fd != nullptr) {
            pos_start = telldir(this->fd.get());// + 256;
            seekdir(this->fd.get(), this->pos_start);
            return true;
        }
        return false;
    }

    bool Directory::close() {
        if (this->fd != nullptr) {
            ESP_LOGI("Directory", "closing directory for %x; descriptor %x", (int)this, int(this->fd.get()));
            int ret = closedir(this->fd.get());
            if (ret != 0) return false;
            return true;
        }
        return false;
    }

    int Directory::get_num_files() const {
        if (this->fd == nullptr)
            return -1;
        
        struct dirent *entry;
        int count = 0;
        long saved_pos = telldir(this->fd.get());

        seekdir(this->fd.get(), this->pos_start); // rewind
        while ((entry = readdir(this->fd.get())) != nullptr) {
            if (is_file_valid(entry)) {
                ESP_LOGI("Directory", "found a file of type %d in %x, at position %lu. Name is %s (%d)",
                    entry->d_type, (int)this, telldir(this->fd.get()), entry->d_name, strlen(entry->d_name));
                count++;
            }
        }
        
        seekdir(this->fd.get(), saved_pos); // replace where it was before starting
        ESP_LOGI("Directory", "found %d files in %x", count, (int)this);
        return count;

    }
    bool Directory::get_file_info(const int position, std::string & file_name, unsigned char & file_type) const {
        if (this->fd == nullptr)
            return false;
        
        struct dirent *entry = nullptr;
        
        if (position < 0) {
            // case when one wants info for next file
            for (;;) {
                entry = readdir(this->fd.get());
                if (entry != nullptr && is_file_valid(entry)) break;
            }
        } else {
            // case when one wants info for a specific file
            if (this->get_num_files() < position) {
                ESP_LOGI("Directory", "number of files smaller than given position.");
                return false;
            }

            // move location pointer to beginning
            seekdir(this->fd.get(), this->pos_start);
            int count = position;
            while (count) {
                entry = readdir(this->fd.get());
                if (entry != nullptr && is_file_valid(entry)) count--;
            }
        }

        if (entry != nullptr) {
            file_type = entry->d_type;
            file_name.append(entry->d_name);
            return true;
        }
        return false;
    }

    bool Directory::is_open() const {
        return (this->fd != nullptr);
    }
}