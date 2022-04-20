/** \file st7789vi.cpp
 *  \brief Screen painting and handling routines with ST7789VI controller.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "driver/gpio.h" // for GPIO pins
#include "driver/ledc.h" // for backlight dimming
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "esp_log.h"

#include "st7789vi.h"

namespace eobsws::hardware::screen {

  /** \fn void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
   *  \brief This is the function called before an SPI transaction, to set WRX pin state.
   *  \param t: SPI transaction description with .user property set to 0 or 1
   */
  static void IRAM_ATTR lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
    auto user = static_cast<uint32_t*>(t->user);
    gpio_set_level(static_cast<gpio_num_t>(user[0]), static_cast<int>(user[1]));
  }


  ST7789VI_TFT::ST7789VI_TFT(std::unique_ptr<ST7789VI_Configuration> & tft_cfg) : Screen() {
    this->cfg = std::move(tft_cfg);
    this->queue_space = this->cfg->spi_queue_length;
    this->framebuffer_size = this->cfg->spi_max_transfer_size;
  }


  void ST7789VI_TFT::queue_data(const uint8_t *data, const uint32_t data_length) {
    esp_err_t ret;
    spi_transaction_t * t = this->spi_data_trans[this->queue_space-1];
    t->length = data_length*8;
    t->tx_buffer = data;
    t->rxlength = 0;
    t->flags = 0;
    ret = spi_device_queue_trans(this->spi_handle, t, portMAX_DELAY);
    assert(ret == ESP_OK);
    this->queue_space--;
    if (this->queue_space == 0)
      this->flush_spi_buffers();
  }


  void ST7789VI_TFT::write_data(const uint8_t *data, const uint32_t data_length) {
    this->queue_data(data, data_length);
    this->flush_spi_buffers();
  }


  void ST7789VI_TFT::write_command(const uint8_t command) {
    esp_err_t ret;
    spi_transaction_t t;
    uint32_t user[2] = {(uint32_t)(this->cfg->pin_dc), 0};
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &command;
    t.user = (void *)user; // data type: command (0)
    ret = spi_device_transmit(this->spi_handle, &t);
    assert(ret == ESP_OK);
  }


  void ST7789VI_TFT::write_command(const uint8_t command, const uint8_t *data,
                                  const uint16_t data_length) {
    this->write_command(command);
    this->write_data(data, data_length);
  }


  void ST7789VI_TFT::reset() {
    // reset cycle from ST7789VI datasheet v1.5, p.48
    // must pull reset pin from up to low and keep it low for at least 10us
    // hardware reset
    gpio_set_level(this->cfg->pin_reset, 1);
    vTaskDelay(5 / portTICK_RATE_MS);
    gpio_set_level(this->cfg->pin_reset, 0);
    vTaskDelay(20 / portTICK_RATE_MS);
    gpio_set_level(this->cfg->pin_reset, 1);
    vTaskDelay(5 / portTICK_RATE_MS);
    // software reset
    this->write_command(ST7789VI_SWRESET);
    vTaskDelay(150 / portTICK_RATE_MS);
  }


  void ST7789VI_TFT::wakeup() {
    this->write_command(ST7789VI_SLPOUT);
  }


  void ST7789VI_TFT::sleep() {
    this->write_command(ST7789VI_SLPIN);
  }


  void ST7789VI_TFT::set_inversion(const bool enabled) {
    this->write_command(enabled ? ST7789VI_INVON : ST7789VI_INVOFF);
  }


  void ST7789VI_TFT::set_partial(const bool enabled) {
    this->write_command(enabled ? ST7789VI_PTLON : ST7789VI_NORON);
  }


  void ST7789VI_TFT::set_display_state(const bool enabled) {
    this->write_command(enabled ? ST7789VI_DISPON : ST7789VI_DISPOFF);
    this->set_backlight_level(enabled ? 0 : 1024, 500);
  }


  void ST7789VI_TFT::set_idle_mode(const bool enabled) {
    this->write_command(enabled ? ST7789VI_IDMON : ST7789VI_IDMOFF);
  }


  void ST7789VI_TFT::set_draw_area(const uint16_t x0, const uint16_t y0, const uint16_t x1,
                                  const uint16_t y1) {
    const uint8_t col_addr_set[4] = {
        (uint8_t)((x0 >> 8) & 0x00ff), (uint8_t)(x0 & 0x00ff),
        (uint8_t)((x1 >> 8) & 0x00ff), (uint8_t)(x1 & 0x00ff)};
    this->write_command(ST7789VI_CASET, col_addr_set, 4); // column address range
    const uint8_t row_addr_set[4] = {
        (uint8_t)((y0 >> 8) & 0x00ff), (uint8_t)(y0 & 0x00ff),
        (uint8_t)((y1 >> 8) & 0x00ff), (uint8_t)(y1 & 0x00ff)};
    this->write_command(ST7789VI_RASET, row_addr_set, 4); // row address range
  }


  void ST7789VI_TFT::set_color_format(const uint8_t color_format) {
    uint8_t color_mode[1];
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT
      color_mode[0] = {ST7789VI_COLMOD_INT_65K | ST7789VI_COLMOD_CTRL_12BIT};
  #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
      color_mode[0] = {ST7789VI_COLMOD_INT_65K | ST7789VI_COLMOD_CTRL_16BIT};
  #else // CONFIG_COLOR_FORMAT
      color_mode[0] = {ST7789VI_COLMOD_INT_262K | ST7789VI_COLMOD_CTRL_18BIT};
  #endif // CONFIG_COLOR_FORMAT
    this->write_command(ST7789VI_COLMOD, (const uint8_t *)color_mode, 1);
  }


  void ST7789VI_TFT::set_orientation(const uint8_t orientation) {
    uint8_t orientation_data[1];
    if (orientation == 0) {
      this->screen_width = this->cfg->screen_height;
      this->screen_height = this->cfg->screen_width;
      orientation_data[0] = ST7789VI_MADCTL_MX | ST7789VI_MADCTL_MY;
    } else if (orientation == 1) {
      this->screen_width = this->cfg->screen_width;
      this->screen_height = this->cfg->screen_height;
      orientation_data[0] = ST7789VI_MADCTL_MY | ST7789VI_MADCTL_MV;
    } else if (orientation == 2) {
      this->screen_width = this->cfg->screen_width;
      this->screen_height = this->cfg->screen_height;
      orientation_data[0] = ST7789VI_MADCTL_MX | ST7789VI_MADCTL_MV;
    } else {
      this->screen_width = this->cfg->screen_height;
      this->screen_height = this->cfg->screen_width;
      orientation_data[0] = ST7789VI_MADCTL_MY;
    }
    this->write_command(ST7789VI_MADCTL, static_cast<const uint8_t *>(orientation_data), 1);
  }


  void ST7789VI_TFT::paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t & color) {
    // NOTE: to avoid reusing almost the same code as for
    // paint_area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, st7789vi_color_t * bitmap)
    // it would be possible to fill an array with one single value of type st7789vi_color_t,
    // but this is sub-optimal and takes a lot of memory => does that in a smarter way

    // optimized strategy to fill an uniform area with a single color: fill SPI buffers as much
    // as necessary, then send then until area is full.
    if (x0 > x1 || y0 > y1)
      return;

    this->flush_spi_buffers();
    this->set_draw_area(x0, y0, x1, y1);
    this->write_command(ST7789VI_RAMWR);

    uint32_t n_pixels = static_cast<uint32_t>(x1 - x0 + 1) * static_cast<uint32_t>(y1 - y0 + 1);
    st7789vi_color_t st_color = rgb_to_st(color);

    // calculate data chunk size
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT
    n_pixels >>= 1; // divide by 2 as we send 2 pixels on each loop
    uint32_t chunk_size = std::min(n_pixels*3, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 3;
    uint8_t rg = static_cast<uint8_t>(st_color >> 4);
    uint8_t br = static_cast<uint8_t>(st_color << 4) | static_cast<uint8_t>(st_color >> 8);
    uint8_t gb = static_cast<uint8_t>(st_color);
  #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
    uint32_t chunk_size = std::min(n_pixels*2, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 2;
    uint8_t rgbh = static_cast<uint8_t>(st_color >> 8);
    uint8_t rgbl = static_cast<uint8_t>(st_color);
  #else
    uint32_t chunk_size = std::min(n_pixels*3, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 3;
    uint8_t c0 = static_cast<uint8_t>(st_color >> 16);
    uint8_t c1 = static_cast<uint8_t>(st_color >> 8);
    uint8_t c2 = static_cast<uint8_t>(st_color);
  #endif

    // prepare data buffer
    uint8_t * data = this->data_buffers[this->queue_space-1];
    uint16_t count = 0;
    while (count < chunk_size) {
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT
      data[count++] = rg;
      data[count++] = br;
      data[count++] = gb;
  #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
      data[count++] = rgbh;
      data[count++] = rgbl;
  #else
      data[count++] = c0;
      data[count++] = c1;
      data[count++] = c2;
  #endif
    }

    do {
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
      count = std::min(chunk_size, 2*n_pixels);
      n_pixels -= count/2;
  #else
      count = std::min(chunk_size, 3*n_pixels);
      n_pixels -= count/3;
  #endif
      ESP_LOGI("ST7789_paint_pixels", "sending %d bytes. %d pixels remaining.", count, n_pixels);
      this->queue_data(data, count);
    } while (n_pixels);
    this->flush_spi_buffers();
  }

  
  void ST7789VI_TFT::paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t * bitmap) {
    if (x0 > x1 || y0 > y1)
      return;
    // translate buffer from RGB to ST7789VI color format.
    uint32_t n_pixels = static_cast<uint32_t>(x1 - x0 + 1) * static_cast<uint32_t>(y1 - y0 + 1);
    st7789vi_color_t * conv_bitmap = static_cast<st7789vi_color_t*>(malloc(n_pixels * sizeof(st7789vi_color_t)));
    while (n_pixels--) {
      *conv_bitmap++ = rgb_to_st(*bitmap);
      bitmap++;
    }
    conv_bitmap -= static_cast<uint32_t>(x1 - x0 + 1) * static_cast<uint32_t>(y1 - y0 + 1);
    this->paint_area(x0, y0, x1, y1, conv_bitmap);
    free(conv_bitmap);
  }


  void ST7789VI_TFT::paint_pixel(const uint16_t x, const uint16_t y, const uint32_t & color) {
    this->paint_area(x, y, x, y, color);
  }


  void ST7789VI_TFT::paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const st7789vi_color_t * bitmap) {
    if (x0 > x1 || y0 > y1)
      return;
    ESP_LOGI("ST7789VI", "Filling area from (%d, %d) to (%d, %d)", x0, y0, x1, y1);
    this->flush_spi_buffers();
    this->set_draw_area(x0, y0, x1, y1);
    this->write_command(ST7789VI_RAMWR);

    uint32_t n_pixels = static_cast<uint32_t>(x1 - x0 + 1) * static_cast<uint32_t>(y1 - y0 + 1);

    // calculate data chunk size
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT
    n_pixels >>= 1; // divide by 2 as we send 2 pixels on each loop
    uint32_t chunk_size = std::min(n_pixels*3, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 3;
  #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
    uint32_t chunk_size = std::min(n_pixels*2, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 2;
  #else
    uint32_t chunk_size = std::min(n_pixels*3, static_cast<uint32_t>(this->cfg->spi_max_transfer_size));
    chunk_size -= chunk_size % 3;
  #endif

    // send data
    do {
      ESP_LOGI("ST7789_paint_pixels", "using data buffer #%d.", this->queue_space);
      uint8_t * data = this->data_buffers[this->queue_space-1];
      uint16_t count = 0;
      while (count < chunk_size && n_pixels) {
  #if CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_12BIT      
        data[count++] = static_cast<uint8_t>(*bitmap >> 4); // RG
        data[count++] = static_cast<uint8_t>(*bitmap << 4) | static_cast<uint8_t>(*bitmap >> 8); // BR
        data[count++] = static_cast<uint8_t>(*bitmap++); // GB
  #elif CONFIG_COLOR_FORMAT == ST7789VI_COLMOD_CTRL_16BIT
        data[count++] = static_cast<uint8_t>(*bitmap >> 8);
        data[count++] = static_cast<uint8_t>(*bitmap++);
  #else
        data[count++] = static_cast<uint8_t>(*bitmap >> 16);
        data[count++] = static_cast<uint8_t>(*bitmap >> 8);
        data[count++] = static_cast<uint8_t>(*bitmap++);
  #endif
        n_pixels--;
      }
      ESP_LOGI("ST7789_paint_pixels", "sending %d bytes. %d pixels remaining.", count, n_pixels);
      this->queue_data(data, count);
    } while (n_pixels);
    this->flush_spi_buffers();
  }


  void ST7789VI_TFT::paint_screen(const uint32_t & color) {
    this->paint_area(0, 0, this->screen_width, this->screen_height, color);
  }


  void ST7789VI_TFT::set_backlight_level(const uint16_t level, const uint16_t transition_duration) {
    ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, level, transition_duration, LEDC_FADE_NO_WAIT);
  }


  void ST7789VI_TFT::initialize() {
    esp_err_t ret;
    // SPI bus configuration
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = this->cfg->pin_miso;
    buscfg.mosi_io_num = this->cfg->pin_mosi;
    buscfg.sclk_io_num = this->cfg->pin_clock;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = this->cfg->spi_max_transfer_size;
    // SPI device configuration
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = this->cfg->spi_clock_rate;
    devcfg.mode = 0;
    devcfg.spics_io_num = this->cfg->pin_chip_select;
    devcfg.queue_size = this->cfg->spi_queue_length;
    devcfg.pre_cb = lcd_spi_pre_transfer_callback; // callback to set WRX pin before data transfer
    // Initialize data buffers for SPI transfers
    this->data_buffers = static_cast<uint8_t**>(heap_caps_malloc(this->cfg->spi_queue_length*sizeof(uint8_t*), MALLOC_CAP_DMA));
    for (uint8_t i=0; i<this->cfg->spi_queue_length; i++)
      this->data_buffers[i] = static_cast<uint8_t*>(heap_caps_malloc(this->cfg->spi_max_transfer_size, MALLOC_CAP_DMA));
    // Initialize SPI reusable transactions
    this->spi_data_trans = static_cast<spi_transaction_t**>(malloc(this->cfg->spi_queue_length*sizeof(spi_transaction_t*)));
    for (uint8_t i=0; i<this->cfg->spi_queue_length; i++) {
      this->spi_data_trans[i] = static_cast<spi_transaction_t*>(malloc(sizeof(spi_transaction_t)));
      memset(this->spi_data_trans[i], 0, sizeof(spi_transaction_t));
      this->spi_data_trans[i]->user = static_cast<void*>(malloc(2*sizeof(uint32_t)));
      uint32_t user[2] = {static_cast<uint32_t>(this->cfg->pin_dc), 1};
      memcpy(this->spi_data_trans[i]->user, user, 2);
    }
    // Backlight dimmer configuration
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
    ledc_timer.freq_hz = 5000;
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ret = ledc_timer_config(&ledc_timer);
    ESP_ERROR_CHECK(ret);
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.gpio_num = this->cfg->pin_backlight;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ret = ledc_channel_config(&ledc_channel);
    ESP_ERROR_CHECK(ret);
    ledc_fade_func_install(0);
    ESP_ERROR_CHECK(ret);
    this->set_backlight_level(1024, 0); // turn backlight off
    
    // Initialize SPI bus
    ret = spi_bus_initialize(this->cfg->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    // Attach LCD to SPI bus
    ret = spi_bus_add_device(this->cfg->spi_host, &devcfg, &(this->spi_handle));
    ESP_ERROR_CHECK(ret);

    // Initialize non-SPI GPIOs
    gpio_set_direction(this->cfg->pin_dc, GPIO_MODE_OUTPUT);
    gpio_set_direction(this->cfg->pin_reset, GPIO_MODE_OUTPUT);

    // reset TFT
    this->reset();

    // send initialisation commands
    this->wakeup();
    this->set_color_format(CONFIG_COLOR_FORMAT);
    this->set_orientation(2);
    uint32_t black = 0;
    this->paint_screen(black); // fill screen with default color
    this->set_display_state(true);
  }


  void ST7789VI_TFT::flush_spi_buffers() {
    spi_transaction_t * rtrans;
    while (this->queue_space < this->cfg->spi_queue_length) {
      spi_device_get_trans_result(this->spi_handle, &rtrans, portMAX_DELAY);
      this->queue_space++;
    }
  }


  ST7789VI_TFT::~ST7789VI_TFT() {
    if (this->data_buffers != nullptr) {
      for (uint8_t i=0; i<this->cfg->spi_queue_length; i++) {
        free(this->data_buffers[i]);
        free(this->spi_data_trans[i]);
      }
      free(this->data_buffers);
      free(this->spi_data_trans);
    }
    spi_bus_remove_device(this->spi_handle);
    spi_bus_free(this->cfg->spi_host);
  }

}
