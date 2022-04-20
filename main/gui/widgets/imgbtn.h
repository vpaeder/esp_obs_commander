/** \file imgbtn.h
 *  \brief Header file for image button widget class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "esp_log.h"
#include <unordered_map>
#include "gui/image/image.h"
#include "gui/image/image_png.h"
#include "lvglpp/widgets/imgbtn.h"
#include "widget.h"

namespace eobsws::gui::widgets {

    /** \enum ImagePosition
     *  \brief Image positions for ImageButton types.
     */
    enum class ImagePosition {
        Left = 0, /**< left */
        Middle = 1, /**< middle (may be repeated to fill size if necessary) */
        Right = 2 /**< right */
    };

    /** \brief LVGL ImageButton class with event handler.
     *  \tparam ImageClass: image class supported by widget.
     */
    template <class ImageClass> class ImageButton : public Widget<lvgl::widgets::ImageButton> {
    private:
        /** \property std::unordered_map<lv_imgbtn_state_t, std::shared_ptr< gui::image::LvglDecorator<ImageClass> > > image_left
         *  \brief Pointer to the source of the left image.
         */
        std::unordered_map<lv_imgbtn_state_t, std::shared_ptr<gui::image::LvglDecorator<ImageClass>>> image_left;

        /** \property std::unordered_map<lv_imgbtn_state_t, std::shared_ptr< gui::image::LvglDecorator<ImageClass> > > image_middle
         *  \brief Pointer to the source of the middle image.
         */
        std::unordered_map<lv_imgbtn_state_t, std::shared_ptr<gui::image::LvglDecorator<ImageClass>>> image_middle;

        /** \property std::unordered_map<lv_imgbtn_state_t, std::shared_ptr< gui::image::LvglDecorator<ImageClass> > > image_right
         *  \brief Pointer to the source of the right image.
         */
        std::unordered_map<lv_imgbtn_state_t, std::shared_ptr<gui::image::LvglDecorator<ImageClass>>> image_right;

    public:
        using Widget::Widget;

        /** \fn virtual void publish(lv_event_t * e)
         *  \brief This is the action triggered when the button gets pressed.
         *  \param e: event data.
         */
        virtual void publish(lv_event_t * e) override {
            lv_event_code_t code = lv_event_get_code(e);
            ESP_LOGI("ImageButton", "got event; sending data: %s", this->message_data.c_str());
            if(code == LV_EVENT_CLICKED || code == LV_EVENT_RELEASED)
                this->db->publish(this->message_type, this->message_data);
        }

        /** \fn void refresh_src(ImagePosition pos, lv_imgbtn_state_t state)
         *  \brief Refreshes image from linked image source.
         */
        void refresh_src(ImagePosition pos, lv_imgbtn_state_t state) {
            if (pos == ImagePosition::Left) {
                if (this->image_left.count(state)==1) {
                    auto desc = this->image_left[state]->lvgl_descriptor();
                    this->set_src_left_img(state, *desc);
                }
            } else if (pos == ImagePosition::Middle) {
                if (this->image_middle.count(state)==1) {
                    auto desc = this->image_middle[state]->lvgl_descriptor();
                    this->set_src_mid_img(state, *desc);
                }
            } else if (pos == ImagePosition::Right) {
                if (this->image_right.count(state)==1) {
                    auto desc = this->image_right[state]->lvgl_descriptor();
                    this->set_src_right_img(state, *desc);
                }
            }
        }

        /** \fn void set_src(ImagePosition pos, lv_imgbtn_state_t state, std::shared_ptr<storage::Partition> part, const std::string & file_name)
         *  \brief Sets image source from file.
         *  \param pos: image position.
         *  \param state: button state.
         *  \param part: pointer to underlying partition.
         *  \param file_name: image file name.
         */
        void set_src(ImagePosition pos, lv_imgbtn_state_t state, std::shared_ptr<storage::Partition> part, const std::string & file_name) {
            auto file = std::make_shared<storage::File>(part, file_name);
            auto img = std::make_shared<gui::image::LvglDecorator<ImageClass>>(file);
            this->set_src(pos, state, img);
        }

        /** \fn void set_src(ImagePosition pos, lv_imgbtn_state_t state, std::shared_ptr<gui::image::LvglDecorator<ImageClass>> img)
         *  \brief Sets image source from image.
         *  \param pos: image position.
         *  \param state: button state.
         *  \param img: pointer to a decorated image object.
         */
        void set_src(ImagePosition pos, lv_imgbtn_state_t state, std::shared_ptr<gui::image::LvglDecorator<ImageClass>> img) {
            if (pos == ImagePosition::Left) {
                this->image_left[state] = img;
            } else if (pos == ImagePosition::Middle) {
                this->image_middle[state] = img;
            } else if (pos == ImagePosition::Right) {
                this->image_right[state] = img;
            }
            this->refresh_src(pos, state);
        }
    };

    /** \typedef ImageButtonPNG
     *  \brief Shorthand for ImageButton<gui::image::ImagePNG>
     */
    using ImageButtonPNG = ImageButton<gui::image::ImagePNG>;
}
