/** \file spi_flash.h
 *  \brief Header file for SPI flash partition manager.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "storage/partition.h"
#include "storage/file.h"
#include "storage/dir.h"

namespace eobsws::storage {
  /** \class SPIFlash
   *  \brief Class to access an SPI flash partition.
   */
  class SPIFlash : public Partition, public std::enable_shared_from_this<SPIFlash> {
  private:
      /** \property int max_files
       *  \brief Maximum number of opened files at the same time.
       */
      int max_files = 10;

      /** \property wl_handle_t wl_handle
       *  \brief Handle for partition.
       */
      wl_handle_t wl_handle = WL_INVALID_HANDLE;

  public:
      /** \fn SPIFlash()
       *  \brief Constructor.
       */
      SPIFlash(const std::string & label, const std::string & mount_path) : Partition(label, mount_path) {}

      /** \fn ~SPIFlash()
       *  \brief Destructor.
       */
      ~SPIFlash();

      /** \fn void set_max_open_files(const int max_files)
       *  \brief Set maximum number of files open at the same time.
       *  \param max_files: maximum number of open files.
       */
      void set_max_open_files(const int max_files);

      /** \fn bool mount()
       *  \brief Mount partition.
       *  \returns true if partition was mounted succesfully, false otherwise.
       */
      bool mount() override;

      /** \fn bool unmount()
       *  \brief Unmount partition.
       *  \returns true if partition was unmounted succesfully, false otherwise.
       */
      bool unmount() override;

      /** \fn std::unique_ptr<Directory> opendir(const std::string & path)
       *  \brief Open directory at given path.
       *  \param path: directory path.
       *  \returns pointer to directory if succesful, or nullptr otherwise.
       */
      std::unique_ptr<Directory> opendir(const std::string & path) override;

      /** \fn bool makedir(const std::string & path)
       *  \brief Create directory with given path.
       *  \param path: directory path.
       *  \returns true if directory could be created, false otherwise.
       */
      bool makedir(const std::string & path) override;
      
      /** \fn std::unique_ptr<File> open(const std::string & file_path, const char * mode)
       *  \brief Create file at given path.
       *  \param file_path: file path.
       *  \param mode: opening mode (r = read, w = read/write, b = binary, + = append)
       *  \returns pointer to file if succesful, or nullptr otherwise.
       */
      std::unique_ptr<File> open(const std::string & file_path, const char * mode) override;

      /** \fn bool remove(const std::string & file_path)
       *  \brief Delete file at given path.
       *  \param file_path: file path.
       *  \returns true if file could be removed, false otherwise.
       */
      bool remove(const std::string & file_path) override;

      /** \fn bool file_exists(const std::string & file_path)
       *  \brief Tell if file at given path exists.
       *  \param file_path: file path.
       *  \returns true if file exists, false otherwise.
       */
      bool file_exists(const std::string & file_path) override;

      /** \fn bool path_is_valid(const std::string & file_path)
       *  \brief Tell if given string is a valid file path.
       *  \param file_path: file path.
       *  \returns true if path is valid, false otherwise.
       */
      bool path_is_valid(const std::string & file_path);

      /** \fn std::string & get_full_path(const std::string & rel_path)
       *  \brief Generate absolute path from relative path.
       *  \param rel_path: relative path.
       *  \returns absolute path.
       */
      std::string get_full_path(const std::string & rel_path) override;
  };
}
 
