/** \file obs_parser.h
 *  \brief Header file for obs-websocket command parser class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "parser.h"

namespace eobsws::comm::parser {
  
  /** \class OBSParser
   *  \brief Class for parsing obs-websocket commands.
   */
  class OBSParser : public Parser {
  /**
   * @brief Need access to DataNode internals.
   * @relates DataNode
   */
  friend DataNode;
  
  public:
    /** \fn OBSParser(std::shared_ptr<DataBroker> db)
     *  \brief Constructor.
     *  \param db: pointer to data broker to publish and subscribe to.
     */
    OBSParser(std::shared_ptr<DataBroker> db);

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback for publish events from data broker.
     *  \param t: message type.
     *  \param data: data to process.
     *  \returns true if processing succeeded, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data) override;

  };

}
