/** \file digital_pin.h
 *  \brief Header file for digital pin handling.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <driver/gpio.h>


/** \namespace eobsws::hardware
 *  \brief Hardware drivers.
 */
namespace eobsws::hardware {

  /** \enum PinDirection
   *  \brief Pin direction.
   */
  enum class PinDirection : uint8_t {
      Input = 0, /**< input */
      Output = 1 /**< output */
  };

  /** \class DigitalPin
    *  \brief Class to read and write a digital pin.
    */
  class DigitalPin {
  private:
      /** \property gpio_num_t pin_num
       *  \brief GPIO pin number
       */
      gpio_num_t pin_num;

      /** \property PinDirection pin_dir
       *  \brief GPIO pin direction
       */
      PinDirection pin_dir;

      /** \property bool pull_down
       *  \brief State of pull-down resistor. True is enabled.
       */
      bool pull_down = false;

      /** \property bool pull_up
       *  \brief State of pull-up resistor. True is enabled.
       */
      bool pull_up = false;

      /** \property bool initialized
       *  \brief True if pin is initialized, false otherwise.
       */
      bool initialized = false;    

  public:
      /** \fn DigitalPin()
        *  \brief Constructor.
        */
      DigitalPin() = default;
      
      /** \fn void configure()
        *  \brief Configures pin.
        */
      void configure();

      /** \fn void set_pin(gpio_num_t pin_num)
        *  \brief Set GPIO pin number.
        *  \param pin_num: GPIO pin number.
        */
      void set_pin(const gpio_num_t pin_num);

      /** \fn void set_pin_direction(const PinDirection dir)
        *  \brief Set GPIO pin direction.
        *  \param dir: GPIO pin direction.
        */
      void set_pin_direction(const PinDirection dir);

      /** \fn void set_pull_down(const bool state)
        *  \brief Set GPIO pin pull-down resistor state.
        *  \param state: pull-down resistor state. True is enabled.
        */
      void set_pull_down(const bool state);

      /** \fn void set_pull_up(const bool state)
        *  \brief Set GPIO pin pull-up resistor state.
        *  \param state: pull-up resistor state. True is enabled.
        */
      void set_pull_up(const bool state);

      /** \fn bool read()
        *  \brief Read pin value.
        *  \returns pin value as bool.
        */
      bool read() const;

      /** \fn void write(bool value)
        *  \brief Write value to pin.
        *  \param value: value to write.
        */
      void write(const bool value);
  };

}
