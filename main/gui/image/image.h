/** \file image.h
 *  \brief Header file for base image class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <stdio.h>
#include <vector>
#include "esp_log.h"
#include "storage/file.h"

/** \namespace eobsws::gui::image
 *  \brief Image handlers.
 */
namespace eobsws::gui::image {
    
    /// Bitmap type
    using Bitmap = std::vector<uint32_t>;

    /** \class Image
     *  \brief Base class for image objects.
     */
    class Image {
    protected:
        /** \property uint32_t width
         *  \brief Image width
         */
        uint32_t width = 0;

        /** \property uint32_t height
         *  \brief Image height
         */
        uint32_t height = 0;

        /** \property std::vector<uint32_t> bitmap
         *  \brief Buffer containing image data
         */
        Bitmap bitmap;

    public:
        /** \fn Image()
         *  \brief Default constructor.
         */
        Image() {
            ESP_LOGI("Image", "calling constructor for 0x%x.", reinterpret_cast<unsigned int>(this));
        }

        /** \fn ~Image()
         *  \brief Destructor.
         */
        ~Image() {
            ESP_LOGI("Image", "calling destructor for 0x%x.", reinterpret_cast<unsigned int>(this));
        }

        /** \fn Image(std::shared_ptr<storage::Partition> part, const std::string & file_name)
         *  \brief Constructor. Create an image object reading a file.
         *  \param part: pointer to underlying partition.
         *  \param file_name: image file name.
         */
        Image(std::shared_ptr<storage::Partition> part, const std::string & file_name) {}

        /** \fn Bitmap & get_bitmap()
         *  \brief Provide access to stored bitmap.
         *  \returns reference to internal bitmap.
         */
        Bitmap & get_bitmap() { return this->bitmap; }

        /** \fn uint32_t get_width()
         *  \brief Get image width.
         *  \returns stored image width.
         */
        uint32_t get_width() { return this->width; }

        /** \fn uint32_t get_height()
         *  \brief Get image height.
         *  \returns stored image height.
         */
        uint32_t get_height() { return this->height; }

    };

}
