/** \file image.h
 *  \brief Header file for image widget class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "lvglpp/widgets/image/image.h"
#include "gui/image/image_lvgl.h"
#include "gui/image/image_png.h"
#include "storage/partition.h"
#include "storage/file.h"
#include "widget.h"

namespace eobsws::gui::widgets {
    /** \brief LVGL Image class with event handler.
     *  \tparam ImageClass: image class supported by widget.
     */
    template <class ImageClass> class Image : public Widget<lvgl::widgets::Image> {
    private:
        /** \property std::shared_ptr< gui::image::LvglDecorator<ImageClass> > image
         *  \brief Pointer to the image source.
         */
        std::shared_ptr<gui::image::LvglDecorator<ImageClass>> image;

    public:
        using Widget::Widget;

        /** \fn void publish(lv_event_t * e)
         *  \brief This is an action triggered by image (no idea what it'd be).
         *  \param e: event data.
         */
        void publish(lv_event_t * e) override {}

        /** \fn void refresh_src()
         *  \brief Refreshes image from linked image source.
         */
        void refresh_src() {
            auto desc = this->image->lvgl_descriptor();
            lvgl::widgets::Image::set_src(*desc);
        }

        /** \fn void set_src(std::shared_ptr<storage::Partition> part, const std::string & file_name)
         *  \brief Sets image source from file.
         *  \param part: pointer to underlying partition.
         *  \param file_name: image file name.
         */
        void set_src(std::shared_ptr<storage::Partition> part, const std::string & file_name) {
            auto file = std::make_shared<storage::File>(part, file_name);
            this->image = std::make_shared<gui::image::LvglDecorator<ImageClass>>(file);
            this->refresh_src();
        }

        /** \fn void set_src(std::shared_ptr<gui::image::LvglDecorator<ImageClass>> img)
         *  \brief Sets image source from image.
         *  \param img: pointer to a decorated image object.
         */
        void set_src(std::shared_ptr<gui::image::LvglDecorator<ImageClass>> img) {
            this->image = img;
            this->refresh_src();
        }

    };

    /** \typedef ImagePNG
     *  \brief Shorthand for Image<gui::image::ImagePNG>
     */
    using ImagePNG = Image<gui::image::ImagePNG>;

}
