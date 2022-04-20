/** \file partition_lvgl.h
 *  \brief Header file for binding a generic partition class with LVGL.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "lvglpp/misc/fs.h"
#include "partition.h"
#include "file.h"
#include "dir.h"

namespace eobsws::storage {

    /** \brief A class extending the LVGL filesystem class to use storage::Partition
     *  as a backend.
     *  \tparam PartitionClass: a class derived from storage::Partition
     */
    template <class PartitionClass> class PartitionLVGL : public lvgl::FileSystem {
    private:
        /** \property std::shared_ptr<PartitionClass> part
         *  \brief Stored pointer to the underlying partition instance.
         */
        std::shared_ptr<PartitionClass> part;

    protected:
        /** \property bool ready_cb()
         *  \brief Function called to inquire if partition is ready.
         *  \returns true if partition is ready, false otherwise.
         */
        bool ready_cb() override {
            return this->part->is_mounted();
        }

        /** \fn void * open_cb(const char * path, lv_fs_mode_t mode)
         *  \brief Opens a file on the partition.
         *  \param path: file path.
         *  \param mode: access mode.
         *  \returns pointer to the file descriptor, or nullptr if failed.
         */
        void * open_cb(const char * path, lv_fs_mode_t mode) override {
            ESP_LOGI("PartitionLVGL", "opening file %s", path.c_str());
            std::unique_ptr<File> f;
            auto str = std::string(path);
            switch (mode) {
                case LV_FS_MODE_RD:
                    f = this->part->open(str, "rb");
                    break;
                case LV_FS_MODE_WR | LV_FS_MODE_RD:
                    f = this->part->open(str, "rb+");
                    break;
                case LV_FS_MODE_WR:
                    f = this->part->open(str, "wb");
                    break;
            }
            // store file until close; return raw pointer for LVGL
            auto ret = static_cast<void*>(f.get());
            f.release();
            ESP_LOGI("PartitionLVGL", "file opened with descriptor 0x%x", (int)ret);
            return ret;
        }

        /** \fn lv_fs_res_t close_cb(void * file_p)
         *  \brief Closes a file.
         *  \param file_p: pointer to the file descriptor.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t close_cb(void * file_p) override {
            auto f = reinterpret_cast<File*>(file_p);
            ESP_LOGI("PartitionLVGL", "closing file 0x%x", (int)file_p);
            delete f;
            return LV_FS_RES_OK;
        }

        /** \fn lv_fs_res_t read_cb(void * file_p, void * buf, uint32_t btr, uint32_t * br)
         *  \brief Reads data from a file.
         *  \param file_p: pointer to the file descriptor.
         *  \param buf: pointer to the recipient data buffer.
         *  \param btr: number of bytes to read.
         *  \param br: number of bytes actually read.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t read_cb(void * file_p, void * buf, uint32_t btr, uint32_t * br) override {
            auto f = reinterpret_cast<File*>(file_p);
            ESP_LOGI("PartitionLVGL", "reading %d bytes from file with descriptor 0x%x", btr, (int)f);
            auto rd = f->read(btr);
            if (rd.size() > 0) {
                *br = rd.size();
                std::copy(rd.begin(), rd.end(), reinterpret_cast<char*>(buf));
                return LV_FS_RES_OK;
            }
            return LV_FS_RES_UNKNOWN;
        }

        /** \fn lv_fs_res_t write_cb(void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
         *  \brief Writes data to a file.
         *  \param file_p: pointer to the file descriptor.
         *  \param buf: pointer to the recipient data buffer.
         *  \param btw: number of bytes to write.
         *  \param bw: number of bytes actually written.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t write_cb(void * file_p, const void * buf, uint32_t btw, uint32_t * bw) override {
            auto f = reinterpret_cast<File*>(file_p);
            ESP_LOGI("PartitionLVGL", "writing %d bytes to file with descriptor 0x%x", btw, (int)f);
            *bw = f->write(std::string(reinterpret_cast<const char*>(buf), btw));
            if (*bw == btw) return LV_FS_RES_OK;
            return LV_FS_RES_UNKNOWN;
        }

        /** \fn lv_fs_res_t seek_cb(void * file_p, uint32_t pos, lv_fs_whence_t whence)
         *  \brief Moves access pointer within file.
         *  \param file_p: pointer to the file descriptor.
         *  \param pos: position to move to.
         *  \param whence: mode; absolute (LV_FS_SEEK_SET), relative (LV_FS_SEEK_CUR), 
         *  or absolute from end of file (LV_FS_SEEK_END).
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t seek_cb(void * file_p, uint32_t pos, lv_fs_whence_t whence) override {
            auto f = reinterpret_cast<File*>(file_p);
            ESP_LOGI("PartitionLVGL", "moving to position %d in file with descriptor 0x%x", pos, (int)f);
            if (f->seek(pos, static_cast<int>(whence))) return LV_FS_RES_OK;
            return LV_FS_RES_UNKNOWN;
        }

        /** \fn lv_fs_res_t tell_cb(void * file_p, uint32_t * pos_p)
         *  \brief Gets position of access pointer within file.
         *  \param file_p: pointer to the file descriptor.
         *  \param pos_p: resulting position.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t tell_cb(void * file_p, uint32_t * pos_p) override {
            auto f = reinterpret_cast<File*>(file_p);
            ESP_LOGI("PartitionLVGL", "getting position in file with descriptor 0x%x", (int)f);
            *pos_p = f->get_pos();
            if (*pos_p>=0) return LV_FS_RES_OK;
            return LV_FS_RES_UNKNOWN;
        }

        /** \fn void * dir_open_cb(const char * path)
         *  \brief Opens a directory on the partition.
         *  \param path: directory path.
         *  \returns pointer to the directory descriptor.
         */
        void * dir_open_cb(const char * path) override {
            ESP_LOGI("PartitionLVGL", "opening directory %s", path);
            auto d = this->part->opendir(std::string(path));
            // store directory until close; return raw pointer for LVGL
            auto ret = static_cast<void*>(d.get());
            d.release();
            ESP_LOGI("PartitionLVGL", "directory opened with descriptor 0x%x", (int)ret);
            return ret;
        }

        /** \fn lv_fs_res_t dir_read_cb(void * rddir_p, char * fn)
         *  \brief Reads next entry from a directory.
         *  \param rddir_p: pointer to the directory descriptor.
         *  \param fn: pointer to the recipient buffer for entry path.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t dir_read_cb(void * rddir_p, char * fn) override {
            auto d = reinterpret_cast<Directory*>(rddir_p);
            ESP_LOGI("PartitionLVGL", "read directory with descriptor 0x%x", (int)d);
            std::string file_name;
            unsigned char ftype;
            d->get_file_info(-1, file_name, ftype);
            if (file_name.size()>0) {
                std::copy(file_name.begin(), file_name.end(), reinterpret_cast<char*>(fn));
                return LV_FS_RES_OK;
            }
            return LV_FS_RES_UNKNOWN;
        }

        /** \fn lv_fs_res_t dir_close_cb(void * rddir_p)
         *  \brief Closes a directory.
         *  \param rddir_p: pointer to the directory descriptor.
         *  \returns result code: LV_FS_RES_OK if successful, LV_RES_* otherwise.
         */
        lv_fs_res_t dir_close_cb(void * rddir_p) override {
            auto d = reinterpret_cast<Directory*>(rddir_p);
            ESP_LOGI("PartitionLVGL", "closing directory with descriptor 0x%x", (int)d);
            auto res = d->close() ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
            delete d;
            return res;
        }

    public:
        using lvgl::FileSystem::FileSystem;

        // we don't want to be able to create an instance without an associated partition
        PartitionLVGL(char letter) = delete;

        /** \fn PartitionLVGL(std::shared_ptr<PartitionClass> part, char letter)
         *  \brief Constructor.
         *  \param part: pointer to the underlying partition instance.
         *  \param letter: drive letter to access the partition.
         */
        PartitionLVGL(std::shared_ptr<PartitionClass> part, char letter) : part(part) {
            this->initialize(letter);
        }
    };

}
