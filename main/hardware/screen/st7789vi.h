/** \file st7789vi.h
 *  \brief Header file for screen painting and handling routines with ST7789VI controller.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <memory>

#include "sdkconfig.h"
#include "screen.h"
#include "st7789vi_defs.h"

/** \typedef st7789vi_color_t
 *  \brief This selects the number format required to hold a ST7789VI color representation.
 */
#if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT || CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
using st7789vi_color_t = uint16_t;
#else
using st7789vi_color_t = uint32_t;
#endif // CONFIG_COLOR_FORMAT

namespace eobsws::hardware::screen {

    #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT
    // target format: 444
    /** \fn inline uint16_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b)
      *  \brief Conversion function from RGB to 12bit screen color format.
      *  \param r: red component.
      *  \param g: green component.
      *  \param b: blue component.
      *  \returns 12bit color.
      */
    inline uint16_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b) {
      return ((r & 0xf0) << 4) | (g & 0xf0) | (b >> 4);
    }
    /** \fn uint16_t rgb_to_st(uint32_t argb)
      *  \brief Conversion function from ARGB to 12bit screen color format.
      *  \param argb: 32bit ARGB color.
      *  \returns 12bit color.
      */
    inline uint16_t rgb_to_st(uint32_t argb) {
      return ((argb & 0xf00000) >> 12) | ((argb >> 8) & 0xf0) | ((argb & 0xf0) >> 4);
    }

    #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
    // target format: 565
    /** \fn inline uint16_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b)
      *  \brief Conversion function from RGB to 16bit screen color format.
      *  \param r: red component.
      *  \param g: green component.
      *  \param b: blue component.
      *  \returns 16bit color.
      */
    inline uint16_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b) {
      return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
    }
    /** \fn uint16_t rgb_to_st(uint32_t argb)
      *  \brief Conversion function from ARGB to 16bit screen color format.
      *  \param argb: 32bit ARGB color.
      *  \returns 16bit color.
      */
    inline uint16_t rgb_to_st(uint32_t argb) {
      return ((argb & 0xf80000) >> 8) | ((argb & 0xfc00) >> 5) | ((argb & 0xf8) >> 3);
    }

    #else
    // target format: 666
    /** \fn inline uint32_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b)
      *  \brief Conversion function from RGB to 18bit screen color format.
      *  \param r: red component.
      *  \param g: green component.
      *  \param b: blue component.
      *  \returns 18bit color.
      */
    inline uint32_t rgb_to_st(uint8_t r, uint8_t g, uint8_t b) {
      return (((uint32_t)r & 0x00fc) << 16) | ((g & 0x00fc) << 8) | (b & 0x00fc);
    }
    /** \fn uint32_t rgb_to_st(uint32_t argb)
      *  \brief Conversion function from ARGB to 18bit screen color format.
      *  \param argb: 32bit ARGB color.
      *  \returns 18bit color.
      */
    inline uint16_t rgb_to_st(uint32_t argb) {
      return (argb & 0xfc0000) | (argb & 0xfc00) | (argb & 0xfc);
    }

    #endif // CONFIG_COLOR_FORMAT


  /** \class ST7789VI_Configuration
    *  \brief Class storing configuration for ST7789VI_TFT class.
    */
  struct ST7789VI_Configuration {
    /** \property spi_host_device_t spi_host
     *  \brief SPI host
     */
    spi_host_device_t spi_host = HSPI_HOST;

    /** \property gpio_num_t pin_reset
     *  \brief GPIO pin connected to TFT reset pin
     */
    gpio_num_t pin_reset;

    /** \property gpio_num_t pin_clock
     *  \brief GPIO pin connected to TFT SPI clock pin
     */
    gpio_num_t pin_clock;

    /** \property gpio_num_t pin_miso
     *  \brief GPIO pin connected to TFT SPI MISO pin.
     */
    gpio_num_t pin_miso;

    /** \property gpio_num_t pin_mosi
     *  \brief GPIO pin connected to TFT SPI MOSI pin
     */
    gpio_num_t pin_mosi;

    /** \property gpio_num_t pin_chip_select
     *  \brief GPIO pin connected to TFT SPI chip select pin
     */
    gpio_num_t pin_chip_select;

    /** \property gpio_num_t pin_dc
     *  \brief GPIO pin connected to TFT data/command selection pin
     */
    gpio_num_t pin_dc;

    /** \property gpio_num_t pin_backlight
     *  \brief GPIO pin connected to TFT backlight dimming circuit
     */
    gpio_num_t pin_backlight;

    /** \property uint32_t spi_clock_rate
     *  \brief SPI clock rate in Hz
     */
    uint32_t spi_clock_rate;

    /** \property uint16_t spi_max_transfer_size
     *  \brief Maximum SPI transfer size
     */
    uint16_t spi_max_transfer_size;

    /** \property uint8_t spi_queue_length
     *  \brief SPI TX queue length
     */
    uint8_t spi_queue_length;

    /** \property uint16_t screen_width
     *  \brief Screen width
     */
    uint16_t screen_width;

    /** \property uint16_t screen_height
     *  \brief Screen height
     */
    uint16_t screen_height;

  };
  

  /** \class ST7789VI_TFT
    *  \brief Class handling communication with a ST7789VI TFT controller
    */
  class ST7789VI_TFT : public Screen {
  private:
    /** \property spi_device_handle_t spi_handle
     *  \brief Handle of SPI device.
     */
    spi_device_handle_t spi_handle;

    /** \property std::unique_ptr<ST7789VI_Configuration> cfg
     *  \brief Configuration data.
     */
    std::unique_ptr<ST7789VI_Configuration> cfg;
    
    /** \property uint8_t queue_space
     *  \brief Space left in SPI queue
     */
    uint8_t queue_space = CONFIG_SPI_QUEUE_LENGTH;

    /** \property uint8_t ** data_buffers
     *  \brief Data buffers for SPI transactions (total size = spi_max_transfer_size * spi_queue_length)
     */
    uint8_t ** data_buffers;

    /** \property uint8_t ** spi_data_trans
     *  \brief SPI transaction objects for data transactions (count = spi_queue_length)
     */
    spi_transaction_t ** spi_data_trans;
    
    /** \fn void flush_spi_buffers()
      *  \brief Flush SPI buffers that are queued for transmission.
      */
    void flush_spi_buffers();

  public:
      /** \fn ST7789VI_TFT(std::unique_ptr<ST7789VI_Configuration> & tft_cfg)
        *  \brief Constructor.
        *  \param tft_cfg: pointer to a configuration container.
        */
      ST7789VI_TFT(std::unique_ptr<ST7789VI_Configuration> & tft_cfg);

      /** \fn ~ST7789VI_TFT()
        *  \brief Destructor.
        */
      ~ST7789VI_TFT();
      
      /** \fn void queue_data(const uint8_t *data, const uint32_t data_length)
        *  \brief Queue an array of data bytes to be written on the serial bus.
        *  \param data: array of bytes to write.
        *  \param data_length: length of array.
        */
      void queue_data(const uint8_t *data, const uint32_t data_length) override;

      /** \fn void write_data(const uint8_t *data, const uint32_t data_length)
        *  \brief Write an array of data bytes to the serial bus.
        *  \param data: array of bytes to write.
        *  \param data_length: length of array.
        */
      void write_data(const uint8_t *data, const uint32_t data_length) override;

      /** \fn void write_command(const uint8_t command)
        *  \brief Write a command without arguments to the serial bus.
        *  \param command: command code.
        */
      void write_command(const uint8_t command) override;
      /** \fn void write_command(const uint8_t command, const uint8_t * data, const uint16_t data_length)
        *  \brief Write a command with 8bit data arguments to the serial bus.
        *  \param command: command code.
        *  \param data: data to send as command argument.
        *  \param data_length: number of bytes in the array.
        */
      void write_command(const uint8_t command, const uint8_t * data, const uint16_t data_length) override;

      /** \fn void reset()
        *  \brief Reset TFT.
        */
      void reset();
      
      /** \fn void wakeup()
        *  \brief Wake up screen.
        */
      void wakeup();
      
      /** \fn void sleep()
        *  \brief Send screen to sleep.
        */
      void sleep();
      
      /** \fn void initialize()
        *  \brief Initialize TFT.
        */
      void initialize();
    
      /** \fn void set_inversion(const bool enabled)
        *  \brief Set screen inversion state.
        *  \param enabled: if true, screen colors are inverted; if false, colors are normal.
        */
      void set_inversion(const bool enabled);
      
      /** \fn void set_partial(const bool enabled)
        *  \brief Set or unset partial display mode.
        *  \param enabled: true = partial mode, false = normal mode.
        */
      void set_partial(const bool enabled);
      
      /** \fn void set_display_state(const bool enabled)
        *  \brief Set display state.
        *  \param enabled: true = active state, false = idle state.
        */
      void set_display_state(const bool enabled);
      
      /** \fn void set_idle_mode(const bool enabled)
        *  \brief Set idle mode.
        *  \param enabled: true = idle state, false = active state.
        */
      void set_idle_mode(const bool enabled);
      
      /** \fn void set_color_format(const uint8_t color_format)
        *  \brief Set color format.
        *  \param color_format: code for color format (ST7789VI_COLMOD_CTRL_12BIT, ST7789VI_COLMOD_CTRL_16BIT, ST7789VI_COLMOD_CTRL_18BIT)
        */
      void set_color_format(const uint8_t color_format);
      
      /** \fn void set_orientation(const uint8_t orientation)
        *  \brief Set screen orientation.
        *  \param orientation: 0 = portrait, 1 = landscape
        */
      void set_orientation(const uint8_t orientation);
      
      /** \fn void set_draw_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1)
        *  \brief Set boundaries of drawing area.
        *  \param x0: left position.
        *  \param y0: top position.
        *  \param x1: right position.
        *  \param y1: bottom position.
        */
      void set_draw_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1);

      /** \fn void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t * bitmap)
        *  \brief Fill area in given boundaries.
        *  \param x0: left position.
        *  \param y0: top position.
        *  \param x1: right position.
        *  \param y1: bottom position.
        *  \param bitmap: array of pixel colors, of size (x1-x0+1)*(y1-y0+1)
        */
      void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t * bitmap) override;

      /** \fn void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t & color)
        *  \brief Fill area in given boundaries.
        *  \param x0: left position.
        *  \param y0: top position.
        *  \param x1: right position.
        *  \param y1: bottom position.
        *  \param color: RGBA color.
        */
      void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t & color) override;

      /** \fn void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const st7789vi_color_t * bitmap)
        *  \brief Fill pixels in given area with given bitmap.
        *  \param x0: left position.
        *  \param y0: top position.
        *  \param x1: right position.
        *  \param y1: bottom position.
        *  \param bitmap: buffer containing (x1-x0+1)*(y1-y0+1) pixels in the appropriate color format
        */
      void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const st7789vi_color_t * bitmap);

      /** \fn void paint_pixel(const uint16_t x, const uint16_t y, const uint32_t & color)
        *  \brief Fill pixel at position (x,y).
        *  \param x: horizontal position.
        *  \param y: vertical position.
        *  \param color: RGBA color.
        */
      void paint_pixel(const uint16_t x, const uint16_t y, const uint32_t & color) override;

      /** \fn void paint_screen(const uint32_t & color)
        *  \brief Fill entire screen with given color.
        *  \param color: RGBA color.
        */
      void paint_screen(const uint32_t & color);

      /** \fn void set_backlight_level(const uint16_t level, const uint16_t transition_duration)
        *  \brief Set TFT backlight brightness.
        *  \param level: brightness level (0 = maximum, 1024 = minimum)
        *  \param transition_duration: duration of transition, in ms.
        */
      void set_backlight_level(const uint16_t level, const uint16_t transition_duration);
  };

}
