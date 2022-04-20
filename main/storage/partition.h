/** \file partition.h
 *  \brief Header file for generic partition class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include <memory>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

namespace eobsws::storage {

    class File;
    class Directory;

    /** \class Partition
     *  \brief Class to handle a generic filesystem partition.
     */
    class Partition {
    protected:
        /** \property std::string mount_path
         *  \brief Path where partition will be mounted.
         */
        std::string mount_path;

        /** \property std::string label
         *  \brief Partition label. This is the name of the partition that will be mounted.
         */
        std::string label;

        /** \property bool mounted
         *  \brief True if partition is mounted, false otherwise.
         */
        bool mounted = false;

    public:
        /** \fn Partition()
         *  \brief Constructor.
         */
        Partition(const std::string & label, const std::string & mount_path) {
            this->label = label;
            this->mount_path = mount_path;
        }

        /** \fn bool mount()
         *  \brief Mount partition.
         *  \returns true if partition was mounted succesfully, false otherwise.
         */
        virtual bool mount() = 0;

        /** \fn bool unmount()
         *  \brief Unmount partition.
         *  \returns true if partition was unmounted succesfully, false otherwise.
         */
        virtual bool unmount() = 0;

        /** directory operations **/

        /** \fn std::unique_ptr<Directory> opendir(const std::string & path)
         *  \brief Open directory at given path.
         *  \param path: directory path.
         *  \returns pointer to directory if succesful, or nullptr otherwise.
         */
        virtual std::unique_ptr<Directory> opendir(const std::string & path) = 0;

        /** \fn bool makedir(const std::string & path)
         *  \brief Create directory at given path.
         *  \param path: directory path.
         *  \returns true if directory could be created, false otherwise.
         */
        virtual bool makedir(const std::string & path) = 0;

        /** file operations **/

        /** \fn std::unique_ptr<File> open(const char * file_path, const char * mode)
         *  \brief Create file at given path.
         *  \param file_path: file path.
         *  \param mode: opening mode (r = read, w = read/write, b = binary, + = append)
         *  \returns pointer to file if succesful, or nullptr otherwise.
         */
        virtual std::unique_ptr<File> open(const std::string & file_path, const char * mode) = 0;
        
        /** \fn bool remove(const char * file_path)
         *  \brief Delete file at given path.
         *  \param file_path: file path.
         *  \returns true if file could be removed, false otherwise.
         */
        virtual bool remove(const std::string & file_path) = 0;

        /** \fn bool file_exists(const char * file_path)
         *  \brief Tell if file at given path exists.
         *  \param file_path: file path.
         *  \returns true if file exists, false otherwise.
         */
        virtual bool file_exists(const std::string & file_path) = 0;

        /** \fn char * get_full_path(const char * rel_path)
         *  \brief Generate absolute path from relative path.
         *  \param rel_path: relative path.
         *  \returns absolute path.
         */
        virtual std::string get_full_path(const std::string & rel_path) = 0;

        /** \fn bool is_mounted()
         *  \brief Tell if partition is mounted.
         *  \returns true if partition is mounted, false otherwise.
         */
        bool is_mounted() { return this->mounted; }

    };
}
 
