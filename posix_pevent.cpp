/* pevent --- portable event objects
 * Copyright (C) 2015 Katayama Hirofumi MZ.
 * This file is released under the terms of the Modified BSD License.
 */

/*--------------------------------------------------------------------------*/

#ifndef _WIN32

#include "stdafx.h"

#ifdef __cplusplus
    #include <cstdlib>
    #include <cassert>
    #include <cerrno>
#else
    #include <stdlib.h>
    #include <assert.h>
    #include <errno.h>
#endif

#include <sys/time.h>   /* for struct timeval, timespec */
#include <pthread.h>    /* for POSIX threads */

/*--------------------------------------------------------------------------*/
/* internal type */

typedef struct pe_event_impl_t {
    pthread_cond_t      m_condition;
    pthread_mutex_t     m_lock;
    bool                m_signaled;
    bool                m_auto_reset;
} pe_event_impl_t;

/*--------------------------------------------------------------------------*/
/* the definitions of C functions */

#ifdef __cplusplus
extern "C" {
#endif

pe_event_t pe_create_event(bool manual_reset, bool initial_state) {
    int result;
    pe_event_impl_t *e;
    pe_event_t event = NULL;

    e = (pe_event_impl_t *)malloc(sizeof(pe_event_impl_t));
    if (e != NULL) {
        result = pthread_cond_init(&e->m_condition, 0);
        assert(result == 0);
        result = result;

        result = pthread_mutex_init(&e->m_lock, 0);
        assert(result == 0);
        result = result;

        e->m_signaled = false;
        e->m_auto_reset = !manual_reset;

        if (initial_state) {
            bool ret = pe_set_event((pe_event_t)e);
            assert(ret);
            ret = ret;
        }
        event = (pe_event_t)e;
    }

    return event;
} /* pe_create_event */

bool pe_unlocked_wait(pe_event_t event, uint32_t milliseconds) {
    bool timeout;
    int result;
    pe_event_impl_t *e;
    struct timespec ts;
    struct timeval tv;
    uint64_t nanoseconds;
    const uint64_t kilo = 1000;
    const uint64_t mega = 1000 * 1000;
    const uint64_t giga = 1000 * 1000 * 1000;

    e = (pe_event_impl_t *)event;

    if (e->m_signaled) {
        if (e->m_auto_reset) {
            e->m_signaled = false;
        }
        timeout = false;
    } else {
        if (milliseconds != (uint32_t)(-1)) {
            gettimeofday(&tv, NULL);

            nanoseconds =
                (tv.tv_sec * giga) +
                (milliseconds * mega) +
                (tv.tv_usec * kilo);

            ts.tv_sec = nanoseconds / giga;
            ts.tv_nsec = nanoseconds % giga;

            do {
                result = pthread_cond_timedwait(&e->m_condition, &e->m_lock, &ts);
            } while ((result == 0) && !e->m_signaled);
        } else {
            do {
                result = pthread_cond_wait(&e->m_condition, &e->m_lock);
            } while ((result == 0) && !e->m_signaled);
        }

        timeout = (result == ETIMEDOUT);
        if (result == 0) {
            if (e->m_auto_reset) {
                e->m_signaled = false;
            }
        }
    }

    return timeout;
} /* pe_unlocked_wait */

bool pe_wait_for_event(pe_event_t event, uint32_t milliseconds) {
    bool timeout;
    int result;
    pe_event_impl_t *e;

    e = (pe_event_impl_t *)event;

    if (milliseconds > 0) {
        result = pthread_mutex_lock(&e->m_lock);
    } else {
        result = pthread_mutex_trylock(&e->m_lock);
        if (result == EBUSY) {
            return true;
        }
    }
    assert(result == 0);
    result = result;

    timeout = pe_unlocked_wait((pe_event_t)e, milliseconds);

    result = pthread_mutex_unlock(&e->m_lock);
    assert(result == 0);
    result = result;

    return timeout;
} /* pe_wait_for_event */

bool pe_close_event(pe_event_t event) {
    int result;
    pe_event_impl_t *e;

    e = (pe_event_impl_t *)event;

    result = pthread_cond_destroy(&e->m_condition);
    assert(result == 0);
    result = result;

    result = pthread_mutex_destroy(&e->m_lock);
    assert(result == 0);
    result = result;

    free(e);

    return true;
} /* pe_close_event */

bool pe_set_event(pe_event_t event) {
    int result;
    pe_event_impl_t *e;

    e = (pe_event_impl_t *)event;

    result = pthread_mutex_lock(&e->m_lock);
    assert(result == 0);
    result = result;

    e->m_signaled = true;

    if (e->m_auto_reset) {
        if (e->m_signaled) {
            result = pthread_mutex_unlock(&e->m_lock);
            assert(result == 0);
            result = result;

            result = pthread_cond_signal(&e->m_condition);
            assert(result == 0);
            result = result;
        }
    } else {
        result = pthread_mutex_unlock(&e->m_lock);
        assert(result == 0);
        result = result;

        result = pthread_cond_broadcast(&e->m_condition);
        assert(result == 0);
        result = result;
    }

    return true;
} /* pe_set_event */

bool pe_reset_event(pe_event_t event) {
    int result;
    pe_event_impl_t *e;

    e = (pe_event_impl_t *)event;

    result = pthread_mutex_lock(&e->m_lock);
    assert(result == 0);
    result = result;

    e->m_signaled = false;

    result = pthread_mutex_unlock(&e->m_lock);
    assert(result == 0);
    result = result;

    return true;
} /* pe_reset_event */

bool pe_pulse_event(pe_event_t event) {
    bool ret;

    ret = pe_set_event(event);
    assert(ret);
    ret = ret;

    ret = pe_reset_event(event);
    assert(ret);
    ret = ret;

    return true;
} /* pe_pulse_event */

#ifdef __cplusplus
} // extern "C"
#endif

/*--------------------------------------------------------------------------*/

#endif /* ndef _WIN32 */

/*--------------------------------------------------------------------------*/
