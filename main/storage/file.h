/** \file file.h
 *  \brief Header file for file management class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "partition.h"

#include <stdio.h>
#include <memory>

namespace eobsws::storage {

    /** \typedef FileCloser
     *  \brief Defines a deleter for FILE smart pointers.
     */
    using FileCloser = std::integral_constant<std::decay_t<decltype(fclose)>, fclose>;

    /** \class File
     *  \brief Class providing access to a file on a partition.
     */
    class File {
    private:
        /** \property std::shared_ptr<Partition> partition
         *  \brief Pointer to underlying partition object.
         */
        std::shared_ptr<Partition> partition = nullptr;

        /** \property std::string file_path
         *  \brief Relative file path within partition.
         */
        std::string file_path;

        /** \typedef FilePtrType
         *  \brief A shorthand for the file pointer type std::unique_ptr<FILE, FileCloser>
         */
        using FilePtrType = std::unique_ptr<FILE, FileCloser>;

        /** \property FilePtrType fd
         *  \brief Pointer to file descriptor.
         *  We could use a unique_ptr here but then fclose must be handled with care.
         */
        FilePtrType fd = FilePtrType(nullptr, FileCloser());
        
    public:
        /** \fn File(std::shared_ptr<Partition> partition, const std::string & file_path)
         *  \brief Constructor.
         *  \param partition: pointer to underlying partition object.
         *  \param file_path: relative file path within partition.
         */
        File(std::shared_ptr<Partition> partition, const std::string & file_path);

        /** \fn ~File()
         *  \brief Destructor.
         */
        ~File();

        /** \fn bool open(const char * mode)
         *  \brief Open file.
         *  \param mode: access mode (POSIX format)
         *  \returns true if file could be opened, false otherwise.
         */
        bool open(const char * mode);

        /** \fn bool close()
         *  \brief Close file.
         *  \returns true if file could be closed, false otherwise.
         */
        bool close();

        /** \fn size_t write(const std::string & data) const
         *  \brief Write data to file.
         *  \param data: data string.
         *  \returns number of bytes written, or -1 if failed.
         */
        size_t write(const std::string & data) const;

        /** \fn std::string read(size_t len) const
         *  \brief Read data from file.
         *  \param len: number of bytes to read.
         *  \returns bytes read, or empty string if failed.
         */
        std::string read(size_t len) const;

        /** \fn bool is_open() const
         *  \brief Tell if file is open.
         *  \returns true if file is open, false otherwise.
         */
        bool is_open() const;

        /** \fn size_t get_size() const
         *  \brief Get file size.
         *  \returns file size, or -1 if failed.
         */
        size_t get_size() const;

        /** \fn int get_pos() const
         *  \brief Get position in file.
         *  \returns position in file, or -1 if failed.
         */
        int get_pos() const;

        /** \fn bool seek(long int offset, int origin) const
         *  \brief Move position in file to given offset.
         *  \param offset: position to reach.
         *  \param origin: 0=beginning of file, 1=current position, 2=end of file.
         *  \returns true if succeeded, false if failed.
         */
        bool seek(long int offset, int origin) const;

        /** \fn std::string & get_file_path()
         *  \brief Get file path.
         *  \returns relative file path within partition.
         */
        std::string & get_file_path() { return this->file_path; }
    };
};
