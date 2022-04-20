/** \file serial_parser_stub.h
 *  \brief Header file for USB command parsers.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "parser_stub.h"
#include "storage/partition.h"
#include "storage/nvs.h"

/** \namespace eobsws::comm::parser::serial
 *  \brief Serial command parser stubs.
 */
namespace eobsws::comm::parser::serial {
  using namespace eobsws;
  
  /** \class ATCommand
   *  \brief Enum class for AT commands.
   */
  struct ATCommand {
    static const std::string
    GetBufferSize, ///< request buffer size
    Abort, ///< abort current command
    PutFile, ///< initiate put file command
    GetFile, ///< initiate get file command
    PutData, ///< write data into file opened with PutFile
    GetData, ///< read data from file opened with GetFile
    ListDir, ///< start listing directory content
    NextFile, ///< request next file from directory opened with ListDir
    MakeDir, ///< create directory
    Delete, ///< delete file or directory
    SetConf, ///< set configuration key in non-volatile storage
    GetConf, ///< get configuration key from non-volatile storage
    DelConf, ///< delete configuration key from non-volatile storage
    GetFirmwareVersion; ///< get firmware version
  };

  /** \class ATReply
   *  \brief Enum class for AT command replies.
   */
  struct ATReply {
    static const std::string
    Ok, ///< tell that command was succesful
    Error, ///< an error occurred
    Busy, ///< device is busy with another command
    Unknown, ///< device received an unknown/unexpected command
    Size, ///< prefix for size value
    Data, ///< prefix for data
    NumFiles, ///< prefix for number of files
    File, ///< prefix for file info
    Value, ///< prefix for configuration key value
    BufferSize, ///< prefix for buffer size
    FirmwareVersion; ///< prefix for firmware version
  };

  /** \class PartitionParserStub
   *  \brief Base class for parser stubs requiring access to a storage partition.
   */
  class PartitionParserStub : public ParserStub {
  protected:
    /** \property std::shared_ptr<storage::Partition> partition
     *  \brief Pointer to a partition handler.
     */
    std::shared_ptr<storage::Partition> partition;

  public:
    /** \fn PartitionParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    PartitionParserStub(std::shared_ptr<storage::Partition> partition) : partition(partition) {}

    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {};
  };

  /** \class FileParserStub
   *  \brief Base class for parser stubs requiring access to a file on a partition.
   */
  class FileParserStub : public PartitionParserStub {
  protected:
    /** \property std::unique_ptr<storage::File> file
     *  \brief Pointer to a file handler.
     */
    std::unique_ptr<storage::File> file;

    /** \property uint8_t phase
     *  \brief Parser phase (0=open file, 1=read or write)
     */
    uint8_t phase = 0;
    
    /** \property std::size_t remaining_bytes
     *  \brief Remaining number of bytes before the end of read/write phase.
     */
    std::size_t remaining_bytes;

    /** \fn bool open_file(const std::string & data, const char * mode)
     *  \brief Open a file.
     *  \param data: file name.
     *  \param mode: access mode.
     *  \returns true if file could be opened, false otherwise.
     */
    bool open_file(const std::string & data, const char * mode);

  public:
    /** \fn FileParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    FileParserStub(std::shared_ptr<storage::Partition> partition) : PartitionParserStub(partition) {}

    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {
      this->file = nullptr;
      this->phase = 0;
    }
  };

  /** \class PutFileParserStub
   *  \brief Class to store data to file from serial AT commands.
   */
  class PutFileParserStub : public FileParserStub {
  private:
    /** \property inline static const std::string default_command
     *  \brief Default parser command (this is the command for phase 0).
     */
    const std::string default_command = ATCommand::PutFile;

  public:
    /** \fn PutFileParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    PutFileParserStub(std::shared_ptr<storage::Partition> partition) : FileParserStub(partition)
      { this->command = this->default_command; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;

    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {
      FileParserStub::abort();
      this->command = this->default_command;
    }
  };

  /** \class GetFileParserStub
   *  \brief Class to get data from file with serial AT commands.
   */
  class GetFileParserStub : public FileParserStub {
  private:
    /** \property inline static const std::string default_command
     *  \brief Default parser command (this is the command for phase 0).
     */
    const std::string default_command = ATCommand::GetFile;

  public:
    /** \fn GetFileParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    GetFileParserStub(std::shared_ptr<storage::Partition> partition) : FileParserStub(partition)
      { this->command = this->default_command; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;
    
    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {
      FileParserStub::abort();
      this->command = this->default_command;
    }
  };
  
  /** \class ListDirParserStub
   *  \brief Class to read a directory with serial AT commands.
   */
  class ListDirParserStub : public PartitionParserStub {
  private:
    /** \property std::unique_ptr<storage::Directory> dir
     *  \brief Pointer to a directory handler.
     */
    std::unique_ptr<storage::Directory> dir;

    /** \property uint8_t phase
     *  \brief Parser phase (0=open directory, 1=read file info)
     */
    uint8_t phase = 0;

    /** \property inline static const std::string default_command
     *  \brief Default parser command (this is the command for phase 0).
     */
    const std::string default_command = ATCommand::ListDir;

    /** \property std::size_t remaining_files
     *  \brief Remaining number of files to be listed.
     */
    std::size_t remaining_files;

  public:
    /** \fn ListDirParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    ListDirParserStub(std::shared_ptr<storage::Partition> partition) : PartitionParserStub(partition)
      { this->command = this->default_command; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;

    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {
      this->dir = nullptr;
      this->phase = 0;
      this->command = this->default_command;
    }

  };

  /** \class DeleteFileParserStub
   *  \brief Class to delete a file or a directory with serial AT command.
   */
  class DeleteFileParserStub : public PartitionParserStub {
  public:
    /** \fn DeleteFileParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    DeleteFileParserStub(std::shared_ptr<storage::Partition> partition) : PartitionParserStub(partition)
      { this->command = ATCommand::Delete; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & file_name) override;
    
  };

  /** \class MakedirParserStub
   *  \brief Class to make a directory from a serial AT command.
   */
  class MakedirParserStub : public PartitionParserStub {
  public:
    /** \fn MakedirParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a partition handler.
     */
    MakedirParserStub(std::shared_ptr<storage::Partition> partition) : PartitionParserStub(partition)
      { this->command = ATCommand::MakeDir; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & dir_name) override;
  };

  /** \class NVSParserStub
   *  \brief Base class used to access a non-volatile storage partition.
   */
  class NVSParserStub : public ParserStub {
  protected:
    /** \property std::shared_ptr<storage::NVStorage> partition
     *  \brief Pointer to a non-volatile storage partition handler.
     */
    std::shared_ptr<storage::NVStorage> partition;

  public:
    /** \fn NVSParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a non-volatile storage partition handler.
     */
    NVSParserStub(std::shared_ptr<storage::NVStorage> partition) : partition(partition) {}
        
    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {};
  };

  /** \class SetConfigParserStub
   *  \brief Class to set a configuration item from a serial AT command.
   */
  class SetConfigParserStub : public NVSParserStub {
  public:
    /** \fn SetConfigParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a non-volatile storage partition handler.
     */
    SetConfigParserStub(std::shared_ptr<storage::NVStorage> partition) : NVSParserStub(partition)
      { this->command = ATCommand::SetConf; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;
  };

  /** \class GetConfigParserStub
   *  \brief Class to get a configuration item with a serial AT command.
   */
  class GetConfigParserStub : public NVSParserStub {
  public:
    /** \fn GetConfigParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a non-volatile storage partition handler.
     */
    GetConfigParserStub(std::shared_ptr<storage::NVStorage> partition) : NVSParserStub(partition)
      { this->command = ATCommand::GetConf; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;
  };

  /** \class DelConfigParserStub
   *  \brief Class to delete a configuration item with a serial AT command.
   */
  class DelConfigParserStub : public NVSParserStub {
  public:
    /** \fn DelConfigParserStub(std::shared_ptr<storage::Partition> partition)
     *  \brief Constructor.
     *  \param partition: pointer to a non-volatile storage partition handler.
     */
    DelConfigParserStub(std::shared_ptr<storage::NVStorage> partition) : NVSParserStub(partition)
      { this->command = ATCommand::DelConf; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & data) override;
  };

  /** \class GetBufSizeParserStub
   *  \brief Class to retrieve the serial buffer size with a serial AT command.
   */
  class GetBufSizeParserStub : public ParserStub {
  public:
    /** \fn GetBufSizeParserStub()
     *  \brief Constructor.
     */
    GetBufSizeParserStub() { this->command = ATCommand::GetBufferSize; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & dir_name) override;
        
    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {};
  };

  /** \class GetFirmwareVersionParserStub
   *  \brief Class to retrieve the firmware version.
   */
  class GetFirmwareVersionParserStub : public ParserStub {
  public:
    /** \fn GetFirmwareVersionParserStub()
     *  \brief Constructor.
     */
    GetFirmwareVersionParserStub() { this->command = ATCommand::GetFirmwareVersion; }

    /** \fn ParserTuple parse(const std::string & data)
     *  \brief Parse given data and return result.
     *  \param data: data to parse.
     *  \returns result compiled as ParserTuple.
     */
    ParserTuple parse(const std::string & dir_name) override;
        
    /** \fn void abort()
     *  \brief Abort current command chain.
     */
    void abort() override {};
  };

}
