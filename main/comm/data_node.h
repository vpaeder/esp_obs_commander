/** \file data_node.h
 *  \brief Header file for data node base class. This is used to build clients
 *  for a pub-sub data broker.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "data_broker.h"

/** \namespace eobsws::comm
 *  \brief This namespace contains all the things related to communication:
 *  pub-sub message handler, UART handler, WiFi handler, associated command processors
 */
namespace eobsws::comm {
  /** \class DataNode
    *  \brief Base class for connection handlers. 
    *  It provides scaffolding to manage a connection channel together
    *  with a DataBroker instance.
    *  Overload event_task to create specific processor.
    */
  class DataNode {
    protected:
    /** \property std::shared_ptr<DataBroker> db
      *  \brief Pointer to a data broker object dispatching data to/from connection handler.
      */
    std::shared_ptr<comm::DataBroker> db;

    /** \property TaskHandle_t task_handle
      *  \brief Handle of event task.
      */
    TaskHandle_t task_handle = nullptr;

    /** \property bool loop_running
      *  \brief If true, event loop is running. If set to false, event loop stops.
      */
    bool loop_running;

    /** \property MessageType in_message_type
      *  \brief Message type accepted by pipe as input
      */
    MessageType in_message_type = MessageType::NoOutlet;

    /** \property MessageType out_message_type
      *  \brief Message type issued by pipe as output
      */
    MessageType out_message_type = MessageType::NoOutlet;
    
    /** \typedef FuncType
      *  \brief Function signature expected by data broker.
      */
    using FuncType = std::function<bool(MessageType, const std::string &)>;

    /** \property std::shared_ptr<FuncType> callback_func
      *  \brief Pointer to function registered with data broker.
      */
    std::shared_ptr<FuncType> callback_func;

    /** \property template <typename Instance> std::shared_ptr<FuncType> convert_callback(Instance * obj)
      *  \brief Helper function to convert publish_callback to function pointer.
      *  \tparam Instance: class of which the callback is a member.
      *  \param obj: class instance pointer.
      */
    template <typename Instance> std::shared_ptr<FuncType> convert_callback(Instance * obj) {
      using namespace std::placeholders;
      FuncType f = std::bind(&Instance::publish_callback, obj, _1, _2);
      this->callback_func = std::make_shared<FuncType>(f);
      return this->callback_func;
    }

    public:
    /** \fn DataNode(std::shared_ptr<DataBroker> db)
      *  \brief Constructor.
      */
    DataNode(std::shared_ptr<DataBroker> db) : db(db) {}

    /** \fn ~DataNode()
      *  \brief Destructor.
      */
    ~DataNode() {
      this->stop_task();
      if (this->task_handle != nullptr)
        vTaskDelete(this->task_handle);
    }

    /** \fn void stop_task()
      *  \brief Stop event loop used to handle connection. Instance can be deleted after this.
      */
    void stop_task() { this->loop_running = false; }

    /** \fn void set_input_message_type(MessageType t)
      *  \brief Set input message type. This is the kind of messages the node accepts.
      *  \param t: message type.
      */
    virtual void set_input_message_type(MessageType t) { this->in_message_type = t; }

    /** \fn void set_output_message_type(MessageType t)
      *  \brief Set output message type. This is the kind of messages the node issues.
      *  \param t: message type.
      */
    virtual void set_output_message_type(MessageType t) { this->out_message_type = t; }
  };

}
