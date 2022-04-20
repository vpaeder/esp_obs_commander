/** \file res_touch.cpp
 *  \brief Resistive touch screen handling routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "touch.h"
#include "esp_log.h"
#include "res_touch.h"

namespace eobsws::hardware::input {

    void ResistiveTouchPanel::set_pins(gpio_num_t pin_xl, gpio_num_t pin_xr, gpio_num_t pin_yd, gpio_num_t pin_yu) {
    this->pin_xl = pin_xl;
    this->pin_xr = pin_xr;
    this->pin_yd = pin_yd;
    this->pin_yu = pin_yu;
    }

    void ResistiveTouchPanel::initialize() {
        adc1_config_width(ADC_WIDTH_10Bit);
        esp_err_t ret;
        // search for ADC channels for pins XL and YD
        for (uint8_t chan=static_cast<uint8_t>(ADC1_CHANNEL_0); chan<static_cast<uint8_t>(ADC1_CHANNEL_MAX); chan++) {
            gpio_num_t pad;
            ret = adc1_pad_get_io_num(static_cast<adc1_channel_t>(chan), &pad);
            ESP_ERROR_CHECK(ret);
            if (pad == this->pin_xl)
                this->chan_xl = static_cast<adc1_channel_t>(chan);
            if (pad == this->pin_yd)
                this->chan_yd = static_cast<adc1_channel_t>(chan);
        }
        // next line isn't absolutely necessary, but ensures that ADC won't
        // turn off while touch device is hooked
        adc_power_acquire();
    }

    ResistiveTouchPanel::~ResistiveTouchPanel() {
        // if task handle exists, this means touch interrupt is active
        if (this->task_handle != nullptr)
            this->disable_touch_interrupt();
        adc_power_release();
    }

    uint16_t ResistiveTouchPanel::read_position(uint8_t idx) {
        uint16_t result = 0;
        if ((idx==0 && !this->flipped) || (idx==1 && this->flipped)) {
            // setup to read X position:
            //  - set XL to ground and XR to Vcc
            //  - set YU floating
            //  - read voltage on YD
            gpio_set_direction(this->pin_xl, GPIO_MODE_OUTPUT);
            gpio_set_direction(this->pin_xr, GPIO_MODE_OUTPUT);
            gpio_set_level(this->pin_xl, 0);
            gpio_set_level(this->pin_xr, 1);
            gpio_set_direction(this->pin_yu, GPIO_MODE_INPUT);
            gpio_set_pull_mode(this->pin_yu, GPIO_FLOATING);
            
            // attach YD pin to ADC
            adc1_config_channel_atten(this->chan_yd, ADC_ATTEN_11db);

            // measure raw value
            result = adc1_get_raw(this->chan_yd);
            // release YD pin from ADC
            rtc_gpio_deinit(this->pin_yd);
        } else if ((idx==1 && !this->flipped) || (idx==0 && this->flipped)) {
            // setup to read Y position:
            //  - set YD to ground and YU to Vcc
            //  - set XR floating
            //  - read voltage on XL
            gpio_set_direction(this->pin_yd, GPIO_MODE_OUTPUT);
            gpio_set_direction(this->pin_yu, GPIO_MODE_OUTPUT);
            gpio_set_level(this->pin_yd, 0);
            gpio_set_level(this->pin_yu, 1);
            gpio_set_direction(this->pin_xr, GPIO_MODE_INPUT);
            gpio_set_pull_mode(this->pin_xr, GPIO_FLOATING);
            
            // attach XL pin to ADC
            adc1_config_channel_atten(this->chan_xl, ADC_ATTEN_11db);

            // measure raw value
            result = adc1_get_raw(this->chan_xl);
            // release XL pin from ADC
            rtc_gpio_deinit(this->pin_xl);
        }
        return result;
    }

    uint16_t ResistiveTouchPanel::read_x_position() {
        return this->read_position(0);
    }

    uint16_t ResistiveTouchPanel::read_y_position() {
        return this->read_position(1);
    }

    uint16_t ResistiveTouchPanel::read_touch_pressure() {
        // setup to read pressure:
        //  - set XR to ground and YU to Vcc
        //  - set XL and YD floating
        //  - read voltage on XL and YD => difference = pressure
        gpio_set_direction(this->pin_yu, GPIO_MODE_OUTPUT);
        gpio_set_direction(this->pin_xr, GPIO_MODE_OUTPUT);
        gpio_set_level(this->pin_yu, 0);
        gpio_set_level(this->pin_xr, 1);
        gpio_set_direction(this->pin_yd, GPIO_MODE_INPUT);
        gpio_set_pull_mode(this->pin_yd, GPIO_FLOATING);
        
        // attach XL pin to ADC
        adc1_config_channel_atten(this->chan_xl, ADC_ATTEN_11db);
        uint16_t z1 = adc1_get_raw(this->chan_xl);
        // release XL pin from ADC
        rtc_gpio_deinit(this->pin_xl);
        
        // prepare to measre 2nd part
        gpio_set_direction(this->pin_xl, GPIO_MODE_INPUT);
        gpio_set_pull_mode(this->pin_xl, GPIO_FLOATING);

        // attach YD pin to ADC
        adc1_config_channel_atten(this->chan_yd, ADC_ATTEN_11db);
        
        // compute raw pressure
        uint16_t result = 1023 - (z1 - adc1_get_raw(this->chan_yd));
        // release YD pin from ADC
        rtc_gpio_deinit(this->pin_yd);
        return result;
    }

    esp_err_t ResistiveTouchPanel::setup_touch_detection() {
        // pull XR to ground
        if (gpio_set_direction(this->pin_xr, GPIO_MODE_OUTPUT) != ESP_OK) return ESP_FAIL;
        if (gpio_set_level(this->pin_xr, 0) != ESP_OK) return ESP_FAIL;
        // set YD as input with pull-up
        if (gpio_set_direction(this->pin_yd, GPIO_MODE_INPUT) != ESP_OK) return ESP_FAIL;
        if (gpio_set_pull_mode(this->pin_yd, GPIO_PULLUP_ONLY) != ESP_OK) return ESP_FAIL;
        // set XL and YU as floating inputs
        if (gpio_set_direction(this->pin_yu, GPIO_MODE_INPUT) != ESP_OK) return ESP_FAIL;
        if (gpio_set_direction(this->pin_xl, GPIO_MODE_INPUT) != ESP_OK) return ESP_FAIL;
        if (gpio_set_pull_mode(this->pin_yu, GPIO_FLOATING) != ESP_OK) return ESP_FAIL;
        if (gpio_set_pull_mode(this->pin_xl, GPIO_FLOATING) != ESP_OK) return ESP_FAIL;
        return ESP_OK;
    }

    esp_err_t ResistiveTouchPanel::enable_touch_interrupt() {
        ESP_LOGI("Touch","setup interrupt started.");
        if (this->task_handle != nullptr)
            return ESP_FAIL;
        // create event loop; this is used to read position
        // when a touch event occurred and store it in pos_x, pos_y and pressure
        this->loop_running = true;
        ESP_LOGI("Touch","create event task.");
        auto fevent = [](void* pvParameters) {
            auto obj = reinterpret_cast<ResistiveTouchPanel*>(pvParameters);
            obj->event_task();
            vTaskDelete(nullptr);
        };
        if (xTaskCreate(fevent, "touch_event_task", 2048, static_cast<void*>(this), 12, &(this->task_handle)) != pdPASS)
            return ESP_FAIL;
        // setup interrupt service for GPIO pins
        ESP_LOGI("Touch","install ISR service.");
        if (gpio_install_isr_service(ESP_INTR_FLAG_IRAM) != ESP_OK)
            return ESP_FAIL;
        // setup pins the right way
        ESP_LOGI("Touch","setup pins.");
        if (this->setup_touch_detection() != ESP_OK) return ESP_FAIL;
        // define what triggers an interrupt
        ESP_LOGI("Touch","enable ISR trigger.");
        if (gpio_set_intr_type(this->pin_yd, GPIO_INTR_NEGEDGE) != ESP_OK)
            return ESP_FAIL;
        // define interrupt handler for pin YD
        ESP_LOGI("Touch","set interrupt handler.");
        if (gpio_isr_handler_add(this->pin_yd, &ResistiveTouchPanel::interrupt_callback, static_cast<void*>(this)) != ESP_OK)
            return ESP_FAIL;
        
        ESP_LOGI("Touch","setup done.");
        return ESP_OK;
    }

    esp_err_t ResistiveTouchPanel::disable_touch_interrupt() {
        if (this->task_handle == nullptr) return ESP_FAIL;
        if (gpio_set_intr_type(this->pin_yd, GPIO_INTR_DISABLE) != ESP_OK) return ESP_FAIL;
        if (gpio_isr_handler_remove(this->pin_yd) != ESP_OK) return ESP_FAIL;
        gpio_uninstall_isr_service();
        // we don't need event loop anymore
        this->loop_running = false;
        // next line is needed here to unblock event loop;
        // this will trigger one reading before exiting
        xTaskNotifyGive(this->task_handle); // always returns pdPASS
        vTaskDelete(this->task_handle);
        this->task_handle = nullptr;
        return ESP_OK;
    }

    void IRAM_ATTR ResistiveTouchPanel::interrupt_callback(void * arg) {
        auto obj = static_cast<ResistiveTouchPanel*>(arg);
        obj->interrupt_handler();
    }

    void ResistiveTouchPanel::interrupt_handler() {
        vTaskNotifyGiveFromISR(this->task_handle, nullptr);
    }

    void ResistiveTouchPanel::event_task() {
        while (this->loop_running) {
            if (ulTaskNotifyTake(true, portMAX_DELAY)) {
                // disable interrupt on pin YD while reading data
                gpio_set_intr_type(this->pin_yd, GPIO_INTR_DISABLE);
                // checks that touch reading is correct before assuming a valid
                // touch event occurred
                uint16_t pressure = this->read_touch_pressure();
                uint16_t pos_x = this->read_x_position();
                uint16_t pos_y = this->read_y_position();
                if (pressure>0 && pos_x>0 && pos_y>0) {
                    this->pos_x = pos_x;
                    this->pos_y = pos_y;
                    this->pressure = pressure;
                    this->touch_occurred = true;
                }
                // re-enable touch interrupt
                this->setup_touch_detection();
                gpio_set_intr_type(this->pin_yd, GPIO_INTR_NEGEDGE);
            }
        }
    }

    bool ResistiveTouchPanel::get_touch_occurred() {
        // if touch interrupt is enabled, data gets stored by interrupt
        // when a touch event happened; otherwise, we probe touch pressure
        // and assume a touch event is occurring when z>CONFIG_TOUCH_TRIG_PRESSURE
        if (this->task_handle == nullptr) {
            this->pressure = this->read_touch_pressure();
            this->touch_occurred = this->pressure > CONFIG_TOUCH_TRIG_PRESSURE;
            if (this->touch_occurred) {
                this->pos_x = this->read_x_position();
                this->pos_y = this->read_y_position();
            }
        }
        return this->touch_occurred;
    }

    bool ResistiveTouchPanel::get_data(uint16_t * data) {
        // returns data only if touch occurred
        if (this->touch_occurred) {
            data[0] = this->pos_x*this->scale_x/1000 + this->offset_x;
            data[1] = this->pos_y*this->scale_y/1000 + this->offset_y;
            data[2] = this->pressure;
        }
        return this->touch_occurred;
    }
}
