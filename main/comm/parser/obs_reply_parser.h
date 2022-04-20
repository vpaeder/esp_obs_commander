/** \file obs_reply_parser.cpp
 *  \brief Header file for obs-websocket reply parser class.
 *  This takes events (MessageType::Event) issued by OBSParser.
 *
 *  Author: Vincent Paeder.
 *  License: MIT
 */
#pragma once
#include "parser.h"

namespace eobsws::comm::parser {
  
  /** \class OBSReplyParser
   *  \brief Class for parsing obs-websocket replies (opcodes 7 and 9).
   */
  class OBSReplyParser : public Parser {
  /**
   * @brief Need access to DataNode internals.
   * @relates DataNode
   */
  friend DataNode;
  
  public:
    /** \fn OBSReplyParser(std::shared_ptr<DataBroker> db)
     *  \brief Constructor.
     *  \param db: pointer to data broker to publish and subscribe to.
     */
    OBSReplyParser(std::shared_ptr<DataBroker> db);

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback for publish events from data broker.
     *  \param t: message type.
     *  \param data: data to process.
     *  \returns true if processing succeeded, false otherwise.
     */
    bool publish_callback(MessageType t, const std::string & data) override;

  };

}
