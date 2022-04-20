/** \file analog_pin.h
 *  \brief Header file for analog pin handling.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <driver/gpio.h>
#include <driver/adc.h>

/** \namespace eobsws::hardware
 *  \brief Hardware drivers.
 */
namespace eobsws::hardware {
  
  /** \class AnalogPin
    *  \brief Class to read and monitor an analog pin.
    */
  class AnalogPin {
  private:
      /** \property gpio_num_t pin_num
       *  \brief GPIO pin number
       */
      gpio_num_t pin_num;

      /** \property adc_unit_t adc_unit
       *  \brief ADC unit for pin
       */
      adc_unit_t adc_unit;

      /** \property adc_channel_t adc_channel
       *  \brief ADC channel for pin
       */
      adc_channel_t adc_channel;

      /** \property adc_bits_width_t adc_width
       *  \brief ADC bit width
       */
      adc_bits_width_t adc_width = ADC_WIDTH_10Bit;

      /** \property adc_atten_t adc_atten
       *  \brief ADC channel attenuation
       */
      adc_atten_t adc_atten = ADC_ATTEN_0db;

      /** \property uint16_t n_meas
       *  \brief Number of measurements to average.
       */
      uint16_t n_meas = 1;

      /** \property uint16_t meas_tolerance
       *  \brief Measurement tolerance. If a new measurement is within +/-meas_tolerance
       *  of the previous one, it is considered to be unchanged.
       */
      uint16_t meas_tolerance = 0;

      /** \property uint16_t value
       *  \brief Latest raw value.
       */
      uint16_t value = 0;

      /** \property bool initialized
       *  \brief True if pin is initialized, false otherwise.
       */
      bool initialized = false;    

  public:
      /** \fn AnalogPin()
        *  \brief Constructor.
        */
      AnalogPin() = default;
      
      /** \fn ~AnalogPin()
        *  \brief Destructor.
        */
      ~AnalogPin();

      /** \fn void initialize()
        *  \brief Initialzes pin.
        */
      void initialize();

      /** \fn void set_pin(gpio_num_t pin_num)
        *  \brief Set GPIO pin to read from.
        *  \param pin_num: GPIO pin number to read from.
        */
      void set_pin(const gpio_num_t pin_num);

      /** \fn void set_width(const adc_bits_width_t width)
        *  \brief Set ADC bit width (see ESP IDF documentation for accepted values)
        *  \param width: bit width
        */
      void set_width(const adc_bits_width_t width);

      /** \fn void set_attenuation(const adc_atten_t atten)
        *  \brief Set ADC attenuation (see ESP IDF documentation for accepted values)
        *  \param atten: attenuation
        */
      void set_attenuation(const adc_atten_t atten);

      /** \fn void set_measurement_count(const uint16_t n_meas)
        *  \brief Set number of measurements averaged to produce one reading.
        *  \param n_meas: number of measurements
        */
      void set_measurement_count(const uint16_t n_meas);

      /** \fn void set_tolerance(const uint16_t meas_tolerance)
        *  \brief Set tolerance allowed towards previous measurement. Value won't update
        *         until new measurement exceed previous measurement +/- tolerance.
        *  \param meas_tolerance: measurement tolerance
        */
      void set_tolerance(const uint16_t meas_tolerance);

      /** \fn bool read(uint16_t * result)
        *  \brief Read pin value and tell if value has changed from previous measurement.
        *  \param result: pointer to where the result will be stored.
        *  \returns True if value has changed, false otherwise.
        */
      bool read(int * result);

      /** \fn bool has_changed()
        *  \brief Read pin value and tell if value has changed from previous measurement.
        *  \returns True if value has changed, false otherwise.
        */
      bool has_changed();

      /** \fn uint16_t get_value()
        *  \brief Return last valid measurement.
        *  \returns last valid measurement, or 0 if none is stored.
        */
      uint16_t get_value();
  };

}
