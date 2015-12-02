/* pevent --- portable event objects
 * Copyright (C) 2015 Katayama Hirofumi MZ.
 * This file is released under the terms of the Modified BSD License.
 */

#ifndef KATAHIROMZ_PEVENT_H
#define KATAHIROMZ_PEVENT_H

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

/*--------------------------------------------------------------------------*/
/* define KATAHIROMZ_PEVENT_CPP11 if C++11 */

#ifndef KATAHIROMZ_PEVENT_CPP11
    #if defined(__cplusplus) && (__cplusplus >= 201103L)
        #define KATAHIROMZ_PEVENT_CPP11
    #endif
#endif

/*--------------------------------------------------------------------------*/
/* These definitions are optimized for Win32 */

typedef HANDLE pe_event_t;

#ifdef __cplusplus
    #include <cassert>
    inline pe_event_t pe_create_event(bool manual_reset, bool initial_state) {
        pe_event_t ret;
        ret = ::CreateEvent(NULL, manual_reset, initial_state, NULL);
        assert(ret != NULL);
        return ret;
    }
    inline bool pe_wait_for_event(pe_event_t handle, DWORD milliseconds) {
        assert(handle != NULL);
        return (::WaitForSingleObject(handle, milliseconds) == WAIT_TIMEOUT);
    }
    inline BOOL pe_close_event(pe_event_t handle) {
        assert(handle != NULL);
        return ::CloseHandle(handle);
    }
    inline BOOL pe_set_event(pe_event_t handle) {
        assert(handle != NULL);
        return ::SetEvent(handle);
    }
    inline BOOL pe_reset_event(pe_event_t handle) {
        assert(handle != NULL);
        return ::ResetEvent(handle);
    }
    inline BOOL pe_pulse_event(pe_event_t handle) {
        assert(handle != NULL);
        return ::PulseEvent(handle);
    }
#else
    #define pe_create_event(manual_reset,initial_state) \
        CreateEvent(NULL, (manual_reset), (initial_state), NULL)

    #define pe_wait_for_event(handle,milliseconds) \
        (WaitForSingleObject((handle), (milliseconds)) == WAIT_TIMEOUT)

    #define pe_close_event      CloseHandle
    #define pe_set_event        SetEvent
    #define pe_reset_event      ResetEvent
    #define pe_pulse_event      PulseEvent
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
        bool wait_for_event(DWORD milliseconds = -1) {
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
            return !!pe_set_event(m_handle);
        }

        bool reset() {
            return !!pe_reset_event(m_handle);
        }

        bool pulse() {
            return !!pe_pulse_event(m_handle);
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
