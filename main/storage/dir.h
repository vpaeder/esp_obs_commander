/** \file dir.h
 *  \brief Header file for directory management class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <memory>

#include "partition.h"

/** \namespace eobsws::storage
 *  \brief Storage drivers.
 */
namespace eobsws::storage {

    /** \typedef DirCloser
     *  \brief Defines a deleter for DIR smart pointers.
     */
    using DirCloser = std::integral_constant<std::decay_t<decltype(closedir)>, closedir>;

    /** \class Directory
     *  \brief Class providing access to a directory on a partition.
     */
    class Directory {
    private:
        /** \property std::shared_ptr<Partition> partition
         *  \brief Pointer to underlying partition object.
         */
        std::shared_ptr<Partition> partition = nullptr;

        /** \property std::string dir_path
         *  \brief Relative directory path within partition.
         */
        std::string dir_path;

        /** \typedef DirPtrType
         *  \brief A shorthand for the directory pointer type std::unique_ptr<DIR, DirCloser>
         */
        using DirPtrType = std::unique_ptr<DIR, DirCloser>;

        /** \property DirPtrType fd
         *  \brief Pointer to directory descriptor.
         *  We could use a unique_ptr here but then closedir must be handled with care.
         */
        DirPtrType fd = DirPtrType(nullptr, DirCloser());

        /** \property long pos_start
         *  \brief Position within file descriptors (0 = 1st file descriptor).
         */
        long pos_start = -1;

    public:
        /** \fn Directory(std::shared_ptr<Partition> partition, const std::string & dir_path)
         *  \brief Constructor.
         *  \param partition: pointer to underlying partition object.
         *  \param dir_path: relative directory path within partition.
         */
        Directory(std::shared_ptr<Partition> partition, const std::string & dir_path);

        /** \fn ~Directory()
         *  \brief Destructor.
         */
        ~Directory();

        /** \fn bool open()
         *  \brief Open directory.
         *  \returns true if directory could be opened, false otherwise.
         */
        bool open();

        /** \fn bool close()
         *  \brief Close directory.
         *  \returns true if directory could be closed, false otherwise.
         */
        bool close();

        /** \fn bool is_open()
         *  \brief Tell if directory is open.
         *  \returns true if directory is open, false otherwise.
         */
        bool is_open() const;

        /** \fn int get_num_files()
         *  \brief Tells the number of files in directory.
         *  \returns number of files in directory, or -1 if an error occurred.
         */
        int get_num_files() const;

        /** \fn bool get_file_info(std::string & file_name, const std::string & path, const int position)
         *  \brief Gives the file name and type at given position.
         *  \param position: position to read.
         *  \param file_name: returned file name, if any can be found.
         *  \param file_type: returned file type.
         *  \returns true if operation succeeded, false otherwise.
         */
        bool get_file_info(const int position, std::string & file_name, unsigned char & file_type) const;
    };
};
