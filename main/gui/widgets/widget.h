/** \file widget.h
 *  \brief Header file for base widget class. This permits to integrate
 *  LVGL widgets with event handling subsystem.
 *  An object can trigger a publish event for a number of triggers that
 *  can be registered with the set_trigger method.
 *  The publish method is called and can publish a message to the registered
 *  data broker.
 *  If a parser stub is associated with the widget, it can receive
 *  data such as request ID, in order to handle replies by the host. 
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "comm/data_broker.h"
#include "comm/parser/parser_stub.h"
#include "lvglpp/core/object.h"

/** \namespace eobsws::gui::widgets
 *  \brief Widgets with pub-sub event handling.
 */
namespace eobsws::gui::widgets {
  
  /** \brief Template class meant to decorate LVGL widgets with event handlers.
   *  \tparam LvClass: LVGL widget class.
   */
  template<typename LvClass> class Widget : public LvClass {
  protected:
    /** \property std::shared_ptr<comm::DataBroker> db
     *  \brief Pointer to the associated data broker instance.
     *  While this is not used directly within the template class,
     *  it is likely to be used in derived classes to publish events.
     */
    std::shared_ptr<comm::DataBroker> db;

    /** \property std::weak_ptr<comm::parser::ParserStub> rep_wd
     *  \brief Pointer to the associated parser stub.
     *  While this is not used directly within the template class,
     *  it can be used in a derived class to allow widget to react
     *  to incoming events or request replies.
     */
    std::weak_ptr<comm::parser::ParserStub> rep_wd;

    /** \property std::string message_data
     *  \brief Message issued when widget action gets triggered.
     */
    std::string message_data = "";

    /** \property comm::MessageType message_type
     *  \brief Message type issued when widget action gets triggered.
     */
    comm::MessageType message_type = comm::MessageType::NoOutlet;

  public:
    using LvClass::LvClass; // we want LvClass constructors

    /** \fn void set_data_broker(std::shared_ptr<comm::DataBroker> db)
      *  \brief Set associated data broker.
      *  \param db: pointer to data broker instance.
      */
    void set_data_broker(std::shared_ptr<comm::DataBroker> db) {
      this->db = db;
    }

    /** \fn void set_parser_stub(std::weak_ptr<comm::parser::ParserStub> rep_wd)
      *  \brief Set associated parser stub.
      *  \param rep_wd: pointer to parser stub instance.
      */
    void set_parser_stub(std::weak_ptr<comm::parser::ParserStub> rep_wd) {
      this->rep_wd = rep_wd;
    }

    /** \fn void set_trigger(lv_event_code_t code)
      *  \brief Set which codes trigger an event on this widget.
      *  \param code: compound code as defined by LVGL (see LV_EVENT_* enum)
      */
    void set_trigger(lv_event_code_t code) {
      // remove existing callbacks
      do {} while (this->remove_event_cb());
      // create wrapper function around bound callback method
      auto f = [](lv_event_t * e) {
        auto eobj = reinterpret_cast<Widget*>(lv_event_get_user_data(e));
        eobj->publish(e);
      };
      // add created function as callback, with 'this' as user data
      this->add_event_cb(f, code, static_cast<void*>(this));
    }

    /** \fn void set_message_data(const std::string & data)
      *  \brief Set message issued when widget action gets triggered.
      *  \param data: message content.
      */
    void set_message_data(const std::string & data) {
        this->message_data = data;
    }

    /** \fn void set_message_type(comm::MessageType t)
      *  \brief Set message type issued when widget action gets triggered.
      *  \param t: message type.
      */
    void set_message_type(comm::MessageType t) {
        this->message_type = t;
    }

    /** \fn virtual void publish(lv_event_t * e)
      *  \brief A pure virtual method to be implemented in derived classes to define
      *  the action on event trigger.
      *  \param e: event data.
      */
    virtual void publish(lv_event_t * e) = 0;

  };

}
