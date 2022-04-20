/** \file parser.h
 *  \brief Header file for data parser base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <tuple>

#include "parser_stub.h"
#include "../data_node.h"
#include "storage/partition.h"

/** \namespace eobsws::comm::parser
 *  \brief Data parsers.
 */
namespace eobsws::comm::parser {
  
  /** \class Parser
   *  \brief Class parsing incoming data.
   */
  class Parser : public DataNode {
  /**
   * @brief Need access to DataNode internals.
   * @relates DataNode
   */
  friend DataNode;
  
  protected:
    /** \property std::vector< std::weak_ptr<ParserStub> > stubs
     *  \brief Collection of parser stubs used for parsing incoming data.
     *  See comments in data_broker.h for why I chose std::vector.
     */
    std::vector< std::weak_ptr<ParserStub> > stubs;

    /** \fn bool publish_callback(MessageType t, const std::string & data)
     *  \brief Callback for publish events from data broker.
     *  \param t: message type.
     *  \param data: data to process.
     *  \returns true if processing succeeded, false otherwise.
     */
    virtual bool publish_callback(MessageType t, const std::string & data) = 0;

    /** \fn void clean_up_stubs()
     *  \brief Clean up registered parser stubs from deleted stubs.
     */
    void clean_up_stubs() {
      auto pred = [](auto pr) -> bool { return pr.expired(); };
      std::remove_if(this->stubs.begin(), this->stubs.end(), pred);
    }

    /** \fn void abort_stubs()
     *  \brief Aborts current command chain in all stubs.
     */
    void abort_stubs() {
      for (auto stub: this->stubs)
        stub.lock()->abort();
    }

    /** \fn std::shared_ptr<ParserStub> find_stub_for_command(const std::string & cmd) const
     *  \brief Find stub adequate for given command, within registered parser stubs.
     *  \param cmd: command string.
     *  \returns pointer to found stub, or nullptr if none was found.
     */
    std::shared_ptr<ParserStub> find_stub_for_command(const std::string & cmd) const {
      auto pred = [cmd](auto pr) -> bool { return pr.lock()->can_handle_command(cmd); };
      auto pr_it = std::find_if(std::begin(this->stubs), std::end(this->stubs), pred);
      return (pr_it == std::end(this->stubs)) ? nullptr : (*pr_it).lock();
    }

  public:
    /** \fn Parser(std::shared_ptr<DataBroker> db)
     *  \brief Constructor.
     *  \param db: pointer to a data broker instance.
     */
    Parser(std::shared_ptr<DataBroker> db) : DataNode(db) {}
    
    /** \fn void register_parser_stub(std::weak_ptr<ParserStub> stub)
     *  \brief Register given parser stub with parser.
     *  \param stub: pointer to the parser stub to register.
     */
    void register_parser_stub(std::weak_ptr<ParserStub> stub) {
      if (!stub.expired()){
        // I use emplace_back as I don't want a copy of stub to live in my stubs list,
        // otherwise using weak_ptr is pointless
        this->stubs.emplace_back(stub);
        stub.lock()->set_message_type(this->out_message_type);
      }
    }

    /** \fn void set_output_message_type(MessageType t) override
     *  \brief Set output message type. This is the kind of messages the node issues.
     *  \param t: message type.
     */
    void set_output_message_type(MessageType t) override {
      DataNode::set_output_message_type(t);
        for (auto & s : this->stubs)
          if (!s.expired()) s.lock()->set_message_type(t);
    }

  };

}
