/** \file image_png.h
 *  \brief Header file for PNG image class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#define PNGLE_NO_GAMMA_CORRECTION

#include <memory>
#include "esp_err.h"
#include "image.h"
#include "pngle.h"
#include "storage/partition.h"
#include "storage/file.h"

namespace eobsws::gui::image {
    
    /** \typedef ImagePNGCloser
     *  \brief Defines a deleter for pngle_t smart pointers.
     */
    using ImagePNGCloser = std::integral_constant<std::decay_t<decltype(pngle_destroy)>, pngle_destroy>;

    /** \class ImagePNG
     *  \brief Class for PNG image objects.
     */
    class ImagePNG : public Image {
    private:
        /** \property std::unique_ptr<FILE> file
         *  \brief Pointer to file descriptor used during file parsing.
         */
        std::shared_ptr<storage::File> file;

        /** \property std::unique_ptr<pngle_t, pngle_destroy> pngle
         *  \brief Pointer to a PNGLE instance used during file parsing.
         */
        std::unique_ptr<pngle_t, ImagePNGCloser > pngle;

        /** \property bool ready
         *  \brief True if file decoding is ready, false otherwise.
         */
        bool ready = false;

        /** \fn void initialize()
         *  \brief Initialize PNGLE.
         */
        void initialize();

        /** \fn void on_init(pngle_t *pngle, uint32_t w, uint32_t h)
         *  \brief Callback used by PNGLE in initialization phase.
         *  \param pngle: pointer to PNGLE instance.
         *  \param w: image width.
         *  \param h: image height.
         */
        void on_init(pngle_t *pngle, uint32_t w, uint32_t h);

        /** \fn void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
         *  \brief Callback used by PNGLE in decoding phase.
         *  \param pngle: pointer to PNGLE instance.
         *  \param x: pixel coordinate in 1st dimension.
         *  \param y: pixel coordinate in 2nd dimension.
         *  \param w: image width.
         *  \param h: image height.
         *  \param rgba: array with rgba components
         */
        void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]);

        /** \fn void on_done(pngle_t *pngle)
         *  \brief Callback used by PNGLE in finalization phase.
         *  \param pngle: pointer to PNGLE instance.
         */
        void on_done(pngle_t *pngle);

        /** \fn int read_next_chunk()
         *  \brief Read PNG file chunks sequentially.
         *  \returns 0 if successful, -1 on error.
         */
        int read_next_chunk();

    public:
        using Image::Image;

        /** \fn ImagePNG(std::shared_ptr<storage::Partition> part, const std::string & file_name)
         *  \brief Constructor. Create an image object reading a PNG file.
         *  \param part: pointer to underlying partition.
         *  \param file_name: image file name.
         */
        ImagePNG(std::shared_ptr<storage::Partition> part, const std::string & file_name);

        /** \fn esp_err_t from_file(std::shared_ptr<storage::File> file)
         *  \brief Read a PNG from an already open file. Overwrite data if some is already present.
         *  \param file: pointer to PNG file.
         *  \returns ESP_OK if successful, ESP_FAIL on error.
         */
        esp_err_t from_file(std::shared_ptr<storage::File> file);

        /** \fn esp_err_t from_file(std::shared_ptr<storage::Partition> part, const std::string & file_name)
         *  \brief Read a PNG from a file. Overwrite data if some is already present.
         *  \param part: pointer to underlying partition.
         *  \param file_name: image file name.
         *  \returns ESP_OK if successful, ESP_FAIL on error.
         */
        esp_err_t from_file(std::shared_ptr<storage::Partition> part, const std::string & file_name);

        /** \fn void reset()
         *  \brief Reset image object.
         */
        void reset();

    };

}
