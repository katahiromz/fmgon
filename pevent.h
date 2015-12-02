/* pevent --- portable event objects
 * Copyright (C) 2015 Katayama Hirofumi MZ.
 * This file is released under the terms of the Modified BSD License.
 */

#ifndef KATAHIROMZ_PEVENT_H
#define KATAHIROMZ_PEVENT_H

/*--------------------------------------------------------------------------*/
/* for NULL */

#ifdef __cplusplus
    #include <cstddef>
#else
    #include <stddef.h>
#endif

/*--------------------------------------------------------------------------*/
/* define KATAHIROMZ_PEVENT_CPP11 if C++11 */

#ifndef KATAHIROMZ_PEVENT_CPP11
    #if defined(__cplusplus) && (__cplusplus >= 201103L)
        #define KATAHIROMZ_PEVENT_CPP11
    #endif
#endif

/*--------------------------------------------------------------------------*/
/* for stdint.h and stdbool.h */

#if defined(__cplusplus) && (__cplusplus >= 201103L)
    #include <cstdint>
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
    #include <stdint.h>
    #include <stdbool.h>
#else
    /* portable stdint.h for int*_t and uint*_t */
    #include "pstdint.h"
    /* portable stdbool.h for bool, false and true */
    #include "pstdbool.h"
#endif

/*--------------------------------------------------------------------------*/
/* pe_optional */

#ifdef __cplusplus
    #define pe_optional(arg)    = arg
#else
    #define pe_optional(arg)    /* empty */
#endif

/*--------------------------------------------------------------------------*/
/* pe_event_t --- handle of an event object */

#ifdef _WIN32
    typedef void *pe_event_t;
#else
    typedef struct pe_event_struct {
        char dummy;
    } pe_event_struct;
    typedef pe_event_struct *pe_event_t;
#endif

/*--------------------------------------------------------------------------*/
/* C functions */

#ifdef __cplusplus
extern "C" {
#endif

pe_event_t pe_create_event(
    bool manual_reset   pe_optional(false),
    bool initial_state  pe_optional(false));

/* NOTE: wait_for_event() returns true if timeout. */
bool pe_wait_for_event(
    pe_event_t event, uint32_t milliseconds pe_optional(-1));

bool pe_close_event(pe_event_t event);
bool pe_set_event(pe_event_t event);
bool pe_reset_event(pe_event_t event);
bool pe_pulse_event(pe_event_t event);

#ifdef __cplusplus
} // extern "C"
#endif

/*--------------------------------------------------------------------------*/
/* C++ interface */

#ifdef __cplusplus
    struct PE_event {
        //
        // members
        //
        pe_event_t m_handle;

        //
        // constructors
        //
        PE_event() : m_handle(NULL) { }

        PE_event(bool manual_reset, bool initial_state)
            : m_handle(pe_create_event(manual_reset, initial_state)) { }

        #ifdef KATAHIROMZ_PEVENT_CPP11
            //
            // move constructors
            //
            PE_event(PE_event&& e) : m_handle(e.m_handle) {
                e.m_handle = NULL;
            }

            PE_event& operator=(PE_event&& e) {
                m_handle = e.m_handle;
                e.m_handle = NULL;
                return *this;
            }
        #endif

        //
        // destructor
        //
        virtual ~PE_event() {
            close();
        }

        //
        // attributes
        //
        operator bool() const {
            return m_handle != NULL;
        }

        bool operator!() const {
            return m_handle == NULL;
        }

        //
        // actions
        //
        bool create(bool manual_reset = false, bool initial_state = false) {
            close();
            m_handle = pe_create_event(manual_reset, initial_state);
            return (m_handle != NULL);
        }

        // NOTE: wait_for_event() returns true if timeout.
        bool wait_for_event(uint32_t milliseconds = -1) {
            return pe_wait_for_event(m_handle, milliseconds);
        }

        bool close() {
            if (m_handle != NULL) {
                pe_close_event(m_handle);
                m_handle = NULL;
                return true;
            }
            return false;
        }

        bool set() {
            return pe_set_event(m_handle);
        }

        bool reset() {
            return pe_reset_event(m_handle);
        }

        bool pulse() {
            return pe_pulse_event(m_handle);
        }

    private:
        // NOTE: PE_event cannot be copyed.
        PE_event(const PE_event& e);
        PE_event& operator=(const PE_event& e);
    }; // struct PE_event
#endif  /* def __cplusplus */

/*--------------------------------------------------------------------------*/

#endif  /* ndef KATAHIROMZ_PEVENT_H */

/*--------------------------------------------------------------------------*/
