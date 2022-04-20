/** \file serial_parser.h
 *  \brief Header file for serial parser.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <tuple>

#include "../data_node.h"
#include "serial_parser_stub.h"
#include "parser.h"
#include "storage/partition.h"

namespace eobsws::comm::parser {
  
  /** \class SerialParser
   *  \brief Class handling serial AT commands and data transfer.
   */
  class SerialParser : public Parser {
  /**
   * @brief Need access to DataNode internals.
   * @relates DataNode
   */
  friend DataNode;
    
  private:
    /** \property std::shared_ptr<storage::Partition> partition
     *  \brief Pointer to partition used for file transfer commands.
     */
    std::shared_ptr<storage::Partition> partition;

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback for publish events from data broker.
     *  \param t: message type.
     *  \param data: data to process.
     *  \returns true if processing succeeded, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data) override;

  public:
    /** \fn SerialParser(std::shared_ptr<DataBroker> db)
     *  \brief Constructor.
     *  \param db: pointer to data broker to publish and subscribe to.
     */
    SerialParser(std::shared_ptr<DataBroker> db);
    
  };

}
