/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef _sc_event_private_h_
#define _sc_event_private_h_

#include "../sc_types.h"

/* Events life cycle:
 * - create event - set reference count to 1
 * - emit event - if there are no SC_EVENT_REQUEST_DESTROY flag, then ref sc_event and add it into pending queue
 * - destroy event - wait set flag SC_EVENT_REQUEST_DESTROY and wait until 
 *   all pending calls of this event would be processed (ref count == 1).
 *   After that destroy event.
 */

#define SC_EVENT_REQUEST_DESTROY  (1 << 31)
#define SC_EVENT_REF_COUNT_MASK (~SC_EVENT_REQUEST_DESTROY)

#pragma pack(push, 1)
/*! Structure that contains information about event
 */
struct _sc_event
{
    //! Pointer to context that used with this event
    sc_memory_context * ctx;
    //! sc-addr of listened sc-element
    sc_addr element;
    //! Event type
    sc_event_type type;
    //! Pointer to user data
    sc_pointer data;
    //! Pointer to callback function, that calls on event emit (for backward compatibility)
    fEventCallback callback;
    //! Pointer to callback function, that calls on event emit
    fEventCallbackEx callback_ex;
    //! Pointer to callback function, that calls, when subscribed sc-element deleted
    fDeleteCallback delete_callback;
    //! Reference count (just references from queue). Higest bit used for SC_EVENT_REQUEST_DESTROY
    volatile sc_uint32 ref_count;
    //! Context lock 
    volatile sc_memory_context * ctx_lock;
    
};
#pragma pack(pop)


//! Function to initialize sc-events module
sc_bool sc_events_initialize();

//! Function to shutdown sc-events module
void sc_events_shutdown();

//! Waits while all emited events will be processed, then returns. After calling that function all new emited events will be ignored
void sc_events_stop_processing();

/*! Notificate about sc-element deletion.
 * @param element sc-addr of deleted sc-element
 * @remarks This function call deletion callback function for event.
 * And destroy all events for deleted sc-element
 */
sc_result sc_event_notify_element_deleted(sc_addr element);

/*! Emit event with \p type for sc-element \p el with argument \p arg
 * @param ctx pointer to context, that emits event
 * @param el sc-addr of element that emitting event
 * @param el_access Acces level of \p el
 * @param type Emitting event type
 * @param edge sc-addr of added/remove edge (just for specified events)
 * @param other_el sc-addr of the second element of edge. If \p el is a source, then \p other_el is a target.
 * If \p el is a target, then \p other_el is a source.
 * @return If event emitted without any errors, then return SC_OK; otherwise return SC_ERROR code
 */
sc_result sc_event_emit(sc_memory_context const * ctx, sc_addr el, sc_access_levels el_acces, sc_event_type type, sc_addr edge, sc_addr other_el);

/* Remove reference from event.
 * If event marked for destroy and reference count == 0,
 * then it would be free. In this case function returns SC_TRUE; otherwise - SC_FALSE
 */
sc_bool sc_event_try_free(sc_memory_context const * ctx, sc_event * evt);

/* Lock sc-event by context
 */
void sc_event_lock(sc_event * evt, sc_memory_context const * ctx);

void sc_event_unlock(sc_event * evt, sc_memory_context const * ctx);

#endif
