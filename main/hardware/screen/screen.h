/** \file screen.h
 *  \brief Header file for screen handling base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

/** \namespace eobsws::hardware::screen
 *  \brief Display drivers.
 */
namespace eobsws::hardware::screen {

  /** \class Screen
   *  \brief Base class handling communication with a display unit
   */
    class Screen {
        protected:
        /** \property uint16_t screen_width
         *  \brief Screen width
         */
        uint16_t screen_width;

        /** \property uint16_t screen_height
         *  \brief Screen height
         */
        uint16_t screen_height;

        /** \property uint32_t framebuffer_size
         *  \brief Size of frame buffer, in bytes
         */
        uint32_t framebuffer_size;

        public:
        /** \fn Screen()
         *  \brief Constructor.
         */
        Screen() = default;
    
        /** \fn virtual void queue_data(const uint8_t *data, const uint32_t data_length)
         *  \brief Queue an array of data bytes to be written on the serial bus.
         *  \param data: array of bytes to write.
         *  \param data_length: length of array.
         */
        virtual void queue_data(const uint8_t *data, const uint32_t data_length) = 0;

        /** \fn virtual void write_data(const uint8_t *data, const uint32_t data_length)
         *  \brief Write an array of data bytes to the serial bus.
         *  \param data: array of bytes to write.
         *  \param data_length: length of array.
         */
        virtual void write_data(const uint8_t *data, const uint32_t data_length) = 0;

        /** \fn virtual void write_command(const uint8_t command)
         *  \brief Write a command without arguments to the serial bus.
         *  \param command: command code.
         */
        virtual void write_command(const uint8_t command) = 0;
        /** \fn virtual void write_command(const uint8_t command, const uint8_t * data, const uint16_t data_length)
         *  \brief Write a command with 8bit data arguments to the serial bus.
         *  \param command: command code.
         *  \param data: data to send as command argument.
         *  \param data_length: number of bytes in the array.
         */
        virtual void write_command(const uint8_t command, const uint8_t * data, const uint16_t data_length) = 0;
        
        /** \fn virtual void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t &color)
         *  \brief Fill area in given boundaries.
         *  \param x0: left position.
         *  \param y0: top position.
         *  \param x1: right position.
         *  \param y1: bottom position.
         *  \param color: RGBA color
         */
        virtual void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t &color) = 0;

        /** \fn virtual void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t * bitmap)
         *  \brief Fill area in given boundaries.
         *  \param x0: left position.
         *  \param y0: top position.
         *  \param x1: right position.
         *  \param y1: bottom position.
         *  \param bitmap: array of pixel colors, of size (x1-x0+1)*(y1-y0+1)
         */
        virtual void paint_area(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint32_t * bitmap) = 0;

        /** \fn virtual void paint_pixel(const uint16_t x, const uint16_t y, const uint32_t & color)
         *  \brief Fill pixel at position (x,y).
         *  \param x: horizontal position.
         *  \param y: vertical position.
         *  \param color: color to paint pixel with.
         */
        virtual void paint_pixel(const uint16_t x, const uint16_t y, const uint32_t & color) = 0;

        /** \fn uint16_t get_screen_width() const
         *  \brief Get screen width.
         *  \returns screen width.
         */
        uint16_t get_screen_width() const { return this->screen_width; }

        /** \fn uint16_t get_screen_height() const
         *  \brief Get screen height.
         *  \returns screen height.
         */
        uint16_t get_screen_height() const { return this->screen_height; }

        /** \fn uint16_t get_framebuffer_size() const
         *  \brief Get frame buffer size.
         *  \returns frame buffer size, in bytes.
         */
        uint32_t get_framebuffer_size() const { return this->framebuffer_size; }
    };

}
