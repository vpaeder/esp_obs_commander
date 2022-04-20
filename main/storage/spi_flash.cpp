/** \file spi_flash.cpp
 *  \brief SPI flash partition manager implementation.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "sdkconfig.h"

#include "diskio_impl.h"
#include "esp_log.h"
#include "vfs_fat_internal.h"

#include "spi_flash.h"

namespace eobsws::storage {

    /** \fn static std::string get_path(const std::string & file_path)
     *  \brief Extracts path from a file path.
     *  \param file_path: file path string.
     *  \returns the part of the string that corresponds to the path.
     */
  static std::string get_path(const std::string & file_path) {
    auto pos = file_path.find_last_of("/\\");
    if (pos == std::string::npos)
      return "/";

    auto dir_path = file_path.substr(0, pos);
    ESP_LOGI("Partition", "extracted path: '%s' (length=%d)", dir_path.c_str(),
            dir_path.size());
    return dir_path;
  }

  SPIFlash::~SPIFlash() {
    ESP_LOGI("SPIFlash", "calling destructor for %x", (int)this);
    if (this->mounted)
      this->unmount();
  }

  void SPIFlash::set_max_open_files(const int max_files) {
    ESP_LOGI("SPIFlash", "setting max open files to %d", max_files);
    this->max_files = max_files;
  }

  std::string SPIFlash::get_full_path(const std::string & rel_path) {
    return this->mount_path + "/" + rel_path;
  }

  bool SPIFlash::mount() {
    if (this->mounted) {
      ESP_LOGI("SPIFlash", "partition %s is already mounted.", this->label.c_str());
      return false;
    }
    ESP_LOGI("SPIFlash", "mounting partition %s to %s", this->label.c_str(), this->mount_path.c_str());
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = this->max_files,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};

    esp_err_t err = esp_vfs_fat_spiflash_mount(this->mount_path.c_str(), this->label.c_str(),
                                              &mount_config, &this->wl_handle);
    ESP_LOGI("SPIFlash", "mounting function returned 0x%x", err);

    this->mounted = (err == ESP_OK);
    ESP_LOGI("SPIFlash", "mounting partition %s: %s", this->label.c_str(),
            this->mounted ? "success" : "failure");
    if (!this->mounted)
      this->unmount();
    return this->mounted;
  }

  bool SPIFlash::unmount() {
    ESP_LOGI("SPIFlash", "unmounting %s", this->mount_path.c_str());
    esp_err_t err =
        esp_vfs_fat_spiflash_unmount(this->mount_path.c_str(), this->wl_handle);
    if (err == ESP_OK) {
      this->mounted = false;
      ESP_LOGI("SPIFlash", "unmounting %s succeeded", this->mount_path.c_str());
      return true;
    } else {
      ESP_LOGI("SPIFlash", "unmounting %s failed", this->mount_path.c_str());
      return false;
    }
  }

  std::unique_ptr<storage::Directory> SPIFlash::opendir(const std::string & dir_path) {
    ESP_LOGI("SPIFlash", "opening directory %s on partition %s.", dir_path.c_str(), this->mount_path.c_str());
    // Note: File::opendir calls Partition::opendir_fd with dir_path argument,
    // therefore we must provide relative path and not full path here
    std::unique_ptr<storage::Directory> dir =
        std::make_unique<storage::Directory>(this->shared_from_this(), dir_path);
    dir->open();
    if (dir->is_open())
      return dir;
    ESP_LOGI(
        "SPIFlash",
        "opening directory %s on partition %s failed. Deleting directory object.",
        dir_path.c_str(), this->mount_path.c_str());
    return nullptr;
  }

  bool SPIFlash::makedir(const std::string & path) {
    auto full_path = this->get_full_path(path);
    int result = mkdir(full_path.c_str(), ACCESSPERMS) == 0;
    return (result == 0);
  }

  std::unique_ptr<storage::File> SPIFlash::open(const std::string & file_path,
                                                const char * mode) {
    ESP_LOGI("SPIFlash", "opening file %s on partition %s.", file_path.c_str(), this->mount_path.c_str());
    if (!this->path_is_valid(file_path)) {
      ESP_LOGI("SPIFlash", "directory for %s doesn't exist!", file_path.c_str());
      return nullptr;
    }
    // Note: File::open calls Partition::open_fd with file_path argument,
    // therefore we must provide relative path and not full path here
    std::unique_ptr<storage::File> f =
        std::make_unique<storage::File>(this->shared_from_this(), file_path);
    f->open(mode);
    if (f != nullptr && f->is_open())
      return f;
    ESP_LOGI("SPIFlash",
            "opening file %s on partition %s failed. Deleting file object.",
            file_path.c_str(), this->mount_path.c_str());
    return nullptr;
  }

  bool SPIFlash::remove(const std::string & file_path) {
    if (!this->path_is_valid(file_path)) {
      ESP_LOGI("SPIFlash", "directory for %s doesn't exist!", file_path.c_str());
      return false;
    }
    auto full_path = this->get_full_path(file_path);
    int result = ::remove(full_path.c_str());
    return (result == 0);
  }

  bool SPIFlash::file_exists(const std::string & file_path) {
    auto full_path = this->get_full_path(file_path);
    struct stat buffer;
    int result = stat(full_path.c_str(), &buffer);
    if (result != 0) {
      ESP_LOGI("SPIFlash", "file %s doesn't exist!", full_path.c_str());
      return false;
    }
    return true;
  }

  bool SPIFlash::path_is_valid(const std::string & file_path) {
    ESP_LOGI("SPIFlash", "testing validity of path for '%s'", file_path.c_str());
    auto extracted_path = get_path(file_path);

    if (extracted_path.size() == 0)
      return true;

    auto dir = this->opendir(extracted_path);
    if (dir == nullptr || (dir != nullptr && !(dir->is_open()))) {
      ESP_LOGI("SPIFlash", "path '%s' doesn't exist!", extracted_path.c_str());
      return false;
    }
    ESP_LOGI("SPIFlash", "path '%s' is valid.", extracted_path.c_str());
    return true;
  }

} // namespace storage
