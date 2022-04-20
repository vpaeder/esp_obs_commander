/** \file image_png.cpp
 *  \brief PNG image class implementation.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <assert.h>
#include "esp_log.h"

#include "image_png.h"

namespace eobsws::gui::image {

    void ImagePNG::initialize() {
        ESP_LOGI("ImagePNG", "initializing Pngle for 0x%x.", reinterpret_cast<unsigned int>(this));
        if (this->pngle != nullptr) {
            ESP_LOGE("ImagePNG", "Pngle already initialized.");
            return;
        }
        this->pngle = std::unique_ptr<pngle_t, ImagePNGCloser >(pngle_new(), ImagePNGCloser());
        if (this->pngle == nullptr) {
            ESP_LOGE("ImagePNG", "Pngle couldn't be initialized.");
            return;
        }
        // setting Pngle callbacks
        pngle_set_user_data(this->pngle.get(), static_cast<void*>(this));
        auto init_cb = [](pngle_t* pngle, uint32_t w, uint32_t h) {
            auto obj = reinterpret_cast<ImagePNG*>(pngle_get_user_data(pngle));
            obj->on_init(pngle, w, h);
        };
        pngle_set_init_callback(pngle.get(), init_cb);

        auto draw_cb = [](pngle_t* pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t *rgba) {
            auto obj = reinterpret_cast<ImagePNG*>(pngle_get_user_data(pngle));
            obj->on_draw(pngle, x, y, w, h, rgba);
        };
        pngle_set_draw_callback(pngle.get(), draw_cb);

        auto done_cb = [](pngle_t* pngle) {
            auto obj = reinterpret_cast<ImagePNG*>(pngle_get_user_data(pngle));
            obj->on_done(pngle);
        };
        pngle_set_done_callback(pngle.get(), done_cb);

        ESP_LOGI("ImagePNG", "Pngle initialized for 0x%x.", reinterpret_cast<unsigned int>(this));
        
    }

    ImagePNG::ImagePNG(std::shared_ptr<storage::Partition> part, const std::string & file_name) {
        this->from_file(part, file_name);
    }

    esp_err_t ImagePNG::from_file(std::shared_ptr<storage::Partition> part, const std::string & file_name) {
        auto file = std::make_shared<storage::File>(part, file_name);
        ESP_LOGI("ImagePNG", "loading image from file '%s'.", file->get_file_path().c_str());
        if (this->pngle == nullptr)
            this->initialize();
        if (this->pngle == nullptr)
            return ESP_FAIL;
        
        this->file = file;
        if (this->file->open("rb") == false) {
            ESP_LOGE("ImagePNG", "file '%s' couldn't be opened.", file->get_file_path().c_str());
            return ESP_FAIL;
        }
        
        // keep feeding file until width and height are set
        // reading header
        auto buf = this->file->read(8);
        pngle_feed(this->pngle.get(), buf.c_str(), buf.size());
        // reading chunks until width and height are set
        ESP_LOGI("ImagePNG", "reading image metadata.");
        while (this->width == 0 || this->height == 0) {
            if (read_next_chunk() != 0) {
                ESP_LOGE("ImagePNG", "chunk reading failed.");
                return ESP_FAIL;
            }
        }
        ESP_LOGI("ImagePNG", "image size: %d x %d.", this->width, this->height);
        
        this->bitmap.clear();
        this->bitmap.reserve(this->width * this->height);

        ESP_LOGI("ImagePNG", "reading image content.");
        while (!this->ready)
            this->read_next_chunk();

        ESP_LOGI("ImagePNG", "file '%s' parsed succesfully.", file->get_file_path().c_str());
        return ESP_OK;
    }

    int ImagePNG::read_next_chunk() {
        if (this->ready) {
            ESP_LOGE("ImagePNG", "No chunk to read.");
            return -1;
        }
        // if a chunk has been partly read, we carry on reading;
        // else, we assume we are at the beginning of a new chunk
        ESP_LOGI("ImagePNG", "Reading chunk size...");
        // chunk structure: length (4 bytes) | chunk type (4 bytes) | chunk data (length) | CRC (4 bytes)
        // we read 4 bytes to get length and add 8 bytes to account for type and CRC
        auto buf = this->file->read(8);
        int chunk_length = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]) + 4; // add 4 for CRC
        if (chunk_length < 0)
            return -1;
        ESP_LOGI("ImagPNG", "chunk size: %d bytes", chunk_length);
        // reading type
        int chunk_type = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
        ESP_LOGI("ImagePNG", "chunk type: 0x%x", chunk_type);
        ESP_LOGI("ImagePNG", "pngle_feed result: %d", pngle_feed(this->pngle.get(), buf.c_str(), 8));
        while (chunk_length > 0) {
            ESP_LOGI("ImagePNG", "reading %d bytes...", std::min(chunk_length, 1024));
            auto chunk = this->file->read(std::min(chunk_length, 1024));
            ESP_LOGI("ImagePNG", "%d bytes read.", chunk.size());
            chunk_length -= chunk.size();
            ESP_LOGI("ImagePNG", "%d bytes remaining", chunk_length);
            int fed = pngle_feed(this->pngle.get(), chunk.c_str(), chunk.size());
            ESP_LOGI("ImagePNG", "pngle_feed result: %d", fed);
            if (fed<0)
                return -1;
        }
        return 0;
    }

    void ImagePNG::on_init(pngle_t *pngle, uint32_t w, uint32_t h) {
        if (w>0 && h>0) {
            this->width = w;
            this->height = h;
        }
    }

    void ImagePNG::on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]) {
        ESP_LOGD("ImagePNG", "decoder 0x%x got pixel %d,%d with rgba color (%02x,%02x,%02x,%02x)",
            reinterpret_cast<unsigned int>(this), x, y, rgba[2], rgba[1], rgba[0], rgba[3]);
        this->bitmap.push_back( (rgba[0] << 16) | (rgba[1] << 8) | rgba[2] | (rgba[3] << 24) );
    }

    void ImagePNG::on_done(pngle_t *pngle) {
        this->ready = true;
        this->file->close();
        this->pngle = nullptr;
        ESP_LOGI("ImagePNG", "object 0x%x has finished loading.", reinterpret_cast<unsigned int>(this));
    }

    void ImagePNG::reset() {
        ESP_LOGI("ImagePNG", "resetting object 0x%x.", reinterpret_cast<unsigned int>(this));
        this->width = 0;
        this->height = 0;
        this->bitmap.clear();
        this->file = nullptr;
        this->pngle = nullptr;
    }

}
