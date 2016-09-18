#ifndef __SOCKET_EVENT__
#define __SOCKET_EVENT__

#ifndef _WIN32
#include <utils/socket.h>
#else
#include "socket.h"
#endif /* _WIN32 */

typedef enum event_type_t event_type_t;
enum event_type_t
{
    EVENT_ON_ACCEPT  = 1     << 1,
    EVENT_ON_CONNECT = 1     << 2,
    EVENT_ON_RECV    = 1     << 3,
    EVENT_ON_CLOSE   = 1     << 4,
    EVENT_ON_ALL     = 0x111 << 1
};

typedef enum exception_type_t exception_type_t;
enum exception_type_t {
    EXCEPTION_TIMEOUT = 1,
    EXCEPTION_ERROR
};

typedef struct event_t event_t;
struct event_t {
    /**
     * @brief add socket event
     *
     * @param fd        fd listening on
     * @param type      type of listening
     * @param handler   event handler callback
     * @param arg       parameter of callback
     */
    int (*add) (event_t *this, SOCKET fd, event_type_t type, void (*handler) (SOCKET fd, void *arg), void *arg);

    /**
     * @brief delete socket event
     *
     * @param fd        fd listening on
     * @param type      type of listening
     */
    int (*delete) (event_t *this, SOCKET fd, event_type_t type);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (event_t *this);

    /**
     * @brief exception handle
     */
    void (*exception_handle) (event_t *this, exception_type_t type, void (*handler) (void *), void *arg);
};

/**
 * @brief create socket event instance 
 */
event_t *event_create(int timeout);

#endif /* __SOCKET_EVENT__ */
