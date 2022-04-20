/** \file touch.h
 *  \brief Header file for generic touch screen handling routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** \namespace eobsws::hardware::input
 *  \brief Input drivers.
 */
namespace eobsws::hardware::input {
  
  /** \class TouchPanel
    *  \brief Class to read touch events
    */
  class TouchPanel {
  protected:
      /** \property bool touch_occurred
       *  \brief If true, a new touch event has occurred.
       */
      bool touch_occurred = false;

      /** \property TaskHandle_t task_handle
       *  \brief Reference to the event task handle, if one is running, nullptr otherwise.
       */
      TaskHandle_t task_handle = nullptr;

      /** \property bool loop_running
       *  \brief Event loop will keep running until this is set to false.
       */
      bool loop_running;

      /** \property uint16_t pos_x
       *  \brief Raw value for X position, stored by event loop when touch event occurred.
       */
      uint16_t pos_x;
      /** \property uint16_t pos_y
       *  \brief Raw value for Y position, stored by event loop when touch event occurred.
       */
      uint16_t pos_y;
      /** \property uint16_t pressure
       *  \brief Raw value for touch pressure, stored by event loop when touch event occurred.
       */
      uint16_t pressure;

      /** \fn void event_task()
        *  \brief Event task processing interrupts.
        */
      virtual void event_task() = 0;

  public:
      /** \fn TouchPanel()
        *  \brief Constructor.
        */
      TouchPanel() = default;

      /** \fn ~TouchPanel()
        *  \brief Destructor.
        */
      ~TouchPanel() {
        // if task handle exists, this means touch interrupt is active
        if (this->task_handle != nullptr)
            vTaskDelete(this->task_handle);
      }

      /** \fn uint16_t read_x_position()
        *  \brief Read raw X position.
        *  \returns 16bit raw position value.
        */
      virtual uint16_t read_x_position() = 0;

      /** \fn uint16_t read_y_position()
        *  \brief Read raw Y position.
        *  \returns 16bit raw position value.
        */
      virtual uint16_t read_y_position() = 0;

      /** \fn uint16_t read_touch_pressure()
        *  \brief Read raw touch pressure.
        *  \returns 16bit raw pressure value.
        */
      virtual uint16_t read_touch_pressure() = 0;

      /** \fn bool get_touch_occurred()
        *  \brief Return touch_occurred flag.
        *  \returns true if a touch event occurred, false otherwise.
        */
      virtual bool get_touch_occurred() { return this->touch_occurred; }

      /** \fn bool get_data(uint16_t * data)
        *  \brief Get data stored by event task.
        *  \param data: array of type uint16_t[3] to store results.
        *  \returns true if valid data is available, false otherwise.
        */
      virtual bool get_data(uint16_t * data) = 0;

      /** \fn void reset_touch_occurred_flag()
        *  \brief Reset flag indicating that a touch event occurred. It is not done
        *  automatically as it may be that multiple tasks want to access touch
        *  data when an event occurred.
        */
      virtual void reset_touch_occurred_flag() { this->touch_occurred = false; }
  };

}
