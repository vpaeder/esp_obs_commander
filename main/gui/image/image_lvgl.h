/** \file image_lvgl.h
 *  \brief Header file for generic image class to LVGL bindings.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "image.h"
#include "image_png.h"
#include "lvglpp/draw/image.h"

namespace eobsws::gui::image {

    /** \brief A decorator for image classes to add a conversion function for LVGL.
     *  \tparam ImgClass: decorated image class.
     */
    template <typename ImgClass> class LvglDecorator : public ImgClass {
    private:
        /** \property std::shared_ptr<lvgl::draw::ImageDescriptor> dsc
         *  \brief Pointer to the stored LVGL image descriptor, if
         *  one has been generated.
         */
        std::shared_ptr<lvgl::draw::ImageDescriptor> dsc;

        #if LV_COLOR_DEPTH != 32
        /** \property std::vector<uint8_t> lv_data
         *  \brief Buffer for data converted to LVGL color format.
         *  Once this buffer is filled, the original buffer is cleared.
         *  I may integrate LVGL color format deeper, so that this
         *  buffer is not needed anymore.
         */
        std::vector<uint8_t> lv_data;
        #endif
    
    public:
        using ImgClass::ImgClass;
        

        /** \fn ~LvglDecorator()
         *  \brief Destructor. Need this because ImageDescriptor doesn't
         *  own the image buffer.
         */
        ~LvglDecorator() {
            if (this->dsc != nullptr) {
                // This prevents lv_img_buf_free to clear the buffer
                // as this is the task of the underlying image class.
                this->dsc->raw_ptr()->data_size = 0;
                this->dsc->raw_ptr()->data = nullptr;
            }
        }

        /** \fn std::shared_ptr<lvgl::draw::ImageDescriptor> lvgl_descriptor()
         *  \brief Generate data structure for LVGL.
         *  \returns LVGL image descriptor.
         */
        std::shared_ptr<lvgl::draw::ImageDescriptor> lvgl_descriptor() {
            // only create descriptor if we need to
            if (this->dsc == nullptr) {
                auto dsc = lvgl::LvPointer<lv_img_dsc_t, lv_img_buf_free>(new lv_img_dsc_t());
                dsc->header.always_zero = 0;
                dsc->header.w = this->width;
                dsc->header.h = this->height;
                dsc->data_size = this->width * this->height * LV_IMG_PX_SIZE_ALPHA_BYTE;
                dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA; 
                #if LV_COLOR_DEPTH == 32
                dsc->data = reinterpret_cast<const uint8_t*>(this->bitmap.data());
                #elif LV_COLOR_DEPTH == 16
                // need to convert from 32bit to RGB565 + A.
                this->lv_data.clear();
                this->lv_data.resize(this->bitmap.size()*3);
                for (size_t n = 0; n < this->bitmap.size(); n++) {
                    auto argb = this->bitmap[n];
                    uint16_t col = ((argb & 0xf80000) >> 8) | ((argb & 0xfc00) >> 5) | ((argb & 0xf8) >> 3);
                    this->lv_data[3*n] = col;
                    this->lv_data[3*n+1] = col >> 8;
                    this->lv_data[3*n+2] = argb >> 24;
                }
                dsc->data = reinterpret_cast<const uint8_t*>(this->lv_data.data());
                this->bitmap.resize(0);
                #elif LV_COLOR_DEPTH == 8
                // need to convert from 32bit to RGB332 + A.
                this->lv_data.clear();
                this->lv_data.resize(this->bitmap.size()*2);
                for (size_t n = 0; n < this->bitmap.size(); n++) {
                    auto argb = this->bitmap[n];
                    uint8_t col = ((argb & 0xe00000) >> 16) | ((argb & 0xe000) >> 11) | ((argb & 0xc0) >> 6);
                    this->lv_data[2*n] = col;
                    this->lv_data[2*n+1] = argb >> 24;
                }
                dsc->data = reinterpret_cast<const uint8_t*>(this->lv_data.data());
                this->bitmap.resize(0);
                #elif LV_COLOR_DEPTH == 1
                // need to convert from 32bit to RGB111 + A.
                this->lv_data.clear();
                this->lv_data.resize(this->bitmap.size()*2);
                for (size_t n = 0; n < this->bitmap.size(); n++) {
                    auto argb = this->bitmap[n];
                    uint8_t col = ((argb & 0xff0000) >> 16) | ((argb & 0xff00) >> 8) | argb);
                    this->lv_data[2*n] = col > 128;
                    this->lv_data[2*n+1] = argb >> 24;
                }
                dsc->data = reinterpret_cast<const uint8_t*>(this->lv_data.data());
                this->bitmap.resize(0);
                #endif
                this->dsc = std::make_shared<lvgl::draw::ImageDescriptor>(std::move(dsc));
            }
            return this->dsc;
        }
    };

    /** \typedef LvImagePNG
     *  \brief Shorthand for LvglDecorator<ImagePNG>
     */
    using LvImagePNG = LvglDecorator<ImagePNG>;

}
