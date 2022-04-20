/** \file res_touch.h
 *  \brief Header file for resistive touch screen handling routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include <driver/gpio.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "touch.h"

namespace eobsws::hardware::input {
  
  /** \class ResistiveTouchPanel
    *  \brief Class to read touch events
    */
  class ResistiveTouchPanel : public TouchPanel {
  private:
      /** \property gpio_num_t pin_xl
       *  \brief GPIO pin connected to XL pad
       */
      gpio_num_t pin_xl;

      /** \property gpio_num_t pin_xr
       *  \brief GPIO pin connected to XR pad
       */
      gpio_num_t pin_xr;

      /** \property gpio_num_t pin_yd
       *  \brief GPIO pin connected to YD pad
       */
      gpio_num_t pin_yd;

      /** \property gpio_num_t pin_yu
       *  \brief GPIO pin connected to YU pad
       */
      gpio_num_t pin_yu;

      /** \property adc1_channel_t chan_xl
       *  \brief ADC channel for XL pin
       */
      adc1_channel_t chan_xl;

      /** \property adc1_channel_t chan_yd
       *  \brief ADC channel for YD pin
       */
      adc1_channel_t chan_yd;

      /** \property int16_t offset_x
       *  \brief Offset in x direction applied to raw value:
       *  value_x = raw_x * scale_x / 1000 + offset_x
       */
      int16_t offset_x = 0;

      /** \property int16_t offset_y
       *  \brief Offset in y direction applied to raw value:
       *  value_y = raw_y * scale_y / 1000 + offset_y
       */
      int16_t offset_y = 0;

      /** \property int16_t scale_x
       *  \brief Scale factor applied to raw x value:
       *  value_x = raw_x * scale_x / 1000 + offset_x
       */
      int16_t scale_x = 1000;

      /** \property int16_t scale_y
       *  \brief Scale factor applied to raw y value:
       *  value_y = raw_y * scale_y / 1000 + offset_y
       */
      int16_t scale_y = 1000;

      /** \property bool flipped
       *  \brief If true, x and y directions are swapped.
       */
      bool flipped = false;

      /** \fn static void interrupt_callback(void* arg)
        *  \brief Interrupt handler for touch events.
        *  \param arg: pin number that triggered interrupt.
        */
      static void interrupt_callback(void* arg);

  protected:
      /** \fn void interrupt_handler()
        *  \brief Interrupt handler for touch events.
        *  \param arg: pin number that triggered interrupt.
        */
      void interrupt_handler();

      /** \fn void event_task()
        *  \brief Event task processing interrupts.
        */
      void event_task() override;

      /** \fn uint16_t read_position(uint8_t idx)
        *  \brief Read raw position in given direction (changes based on flipped state).
        *  \param idx: axis index.
        *  \returns 16bit raw position value.
        */
      uint16_t read_position(uint8_t idx);

  public:
      /** \fn ResistiveTouchPanel()
        *  \brief Default constructor.
        */
      ResistiveTouchPanel() = default;

      /** \fn ResistiveTouchPanel(gpio_num_t pin_xl, gpio_num_t pin_xr, gpio_num_t pin_yd, gpio_num_t pin_yu)
        *  \brief Constructor with parameters.
        *  \param pin_xl: XL pin number
        *  \param pin_xr: XR pin number
        *  \param pin_yd: YD pin number
        *  \param pin_yu: YU pin number
        */
      ResistiveTouchPanel(gpio_num_t pin_xl, gpio_num_t pin_xr, gpio_num_t pin_yd, gpio_num_t pin_yu) {
        this->set_pins(pin_xl, pin_xr, pin_yd, pin_yu);
      }

      /** \fn ~ResistiveTouchPanel()
        *  \brief Destructor.
        */
      ~ResistiveTouchPanel();

      /** \fn void set_pins(gpio_num_t pin_xl, gpio_num_t pin_xr, gpio_num_t pin_yd, gpio_num_t pin_yu)
        *  \brief Set GPIO pins. Must be called before initializing.
        *  \param pin_xl: XL pin number
        *  \param pin_xr: XR pin number
        *  \param pin_yd: YD pin number
        *  \param pin_yu: YU pin number
        */
      void set_pins(gpio_num_t pin_xl, gpio_num_t pin_xr, gpio_num_t pin_yd, gpio_num_t pin_yu);

      /** \fn void set_scale(int16_t x_value, int16_t y_value)
        *  \brief Set scaling factors in units of 1000*pixel.
        *  \param x_value: scaling factor for x direction.
        *  \param y_value: scaling factor for y direction.
        */
      void set_scale(int16_t x_value, int16_t y_value) { this->scale_x = x_value; this->scale_y = y_value; }
      /** \fn void set_x_scale(int16_t value)
        *  \brief Set scaling factor for horizontal direction in units of 1000*pixel.
        *  \param value: scaling factor for x direction.
        */
      void set_x_scale(int16_t value) { this->scale_x = value; }
      /** \fn void set_y_scale(int16_t value)
        *  \brief Set scaling factor for vertical direction in units of 1000*pixel.
        *  \param value: scaling factor for y direction.
        */
      void set_y_scale(int16_t value) { this->scale_y = value; }

      /** \fn void set_offset(int16_t x_value, int16_t y_value)
        *  \brief Set offsets in pixel.
        *  \param x_value: offset for x direction.
        *  \param y_value: offset for y direction.
        */
      void set_offset(int16_t x_value, int16_t y_value) { this->offset_x = x_value; this->offset_y = y_value; }
      /** \fn void set_x_offset(int16_t value)
        *  \brief Set offset for horizontal direction in pixel.
        *  \param value: offset for x direction.
        */
      void set_x_offset(int16_t value) { this->offset_x = value; }
      /** \fn void set_y_offset(int16_t value)
        *  \brief Set offset for vertical direction in pixel.
        *  \param value: offset for y direction.
        */
      void set_y_offset(int16_t value) { this->offset_y = value; }

      /** \fn void set_orientation(bool flipped)
        *  \brief Set touch panel orientation.
        *  \param flipped: if true, panel is rotated by 90° clockwise.
        */
      void set_orientation(bool flipped) { this->flipped = flipped; }

      /** \fn void initialize()
        *  \brief Initialize ADC channels. Must be called before using read functions.
        */
      void initialize();
      
      /** \fn uint16_t read_x_position()
        *  \brief Read raw X position.
        *  \returns 16bit raw position value.
        */
      uint16_t read_x_position() override;

      /** \fn uint16_t read_y_position()
        *  \brief Read raw Y position.
        *  \returns 16bit raw position value.
        */
      uint16_t read_y_position() override;

      /** \fn uint16_t read_touch_pressure()
        *  \brief Read raw touch pressure.
        *  \returns 16bit raw pressure value.
        */
      uint16_t read_touch_pressure() override;

      /** \fn esp_err_t setup_touch_detection()
        *  \brief Set up pins in order to detect touch events.
        *  \returns ESP_OK if touch detection could be set up, ESP_FAIL otherwise.
        */
      esp_err_t setup_touch_detection();

      /** \fn esp_err_t enable_touch_interrupt()
        *  \brief Enable interrupt for touch events. Touch parameters will be
        *  stored in pos_x, pos_y and pressure properties and can be accessed
        *  with get_data method.
        *  \returns ESP_OK if interrupt could be enabled, ESP_FAIL otherwise.
        */
      esp_err_t enable_touch_interrupt();

      /** \fn esp_err_t disable_touch_interrupt()
        *  \brief Disable interrupt for touch events.
        *  \returns ESP_OK if interrupt could be disabled, ESP_FAIL otherwise.
        */
      esp_err_t disable_touch_interrupt();

      /** \fn bool get_data(uint16_t * data)
        *  \brief Get data stored by event task.
        *  \param data: array of type uint16_t[3] to store results.
        *  \returns true if valid data is available, false otherwise.
        */
      bool get_data(uint16_t * data) override;

      /** \fn bool get_touch_occurred()
        *  \brief Tell if touch event occurred.
        *  \returns true if a touch event occurred, false otherwise.
        */
      bool get_touch_occurred() override;

  };

}
