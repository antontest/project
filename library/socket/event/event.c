#ifdef _WIN32 
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#endif
#include "event.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <thread/thread.h>
#include <utils/utils.h>
#include <linked_list/linked_list.h>
#else
#include <windows.h>
#include "utils.h"
#include "thread.h"
#include "linked_list.h"
#endif

#define DFT_MAX_EVT_SIZE 4
typedef struct event_pkg_t event_pkg_t;
struct event_pkg_t {
    /**
     * @brief socket descriptor listening on
     */
    SOCKET fd;

    /**
     * @brief event type
     */
    event_type_t type;

    /**
     * @brief socket event handler callback function 
     */
    void (*event_handler) (SOCKET fd, void *arg);

    /**
     * @brief callback function parameter
     */
    void *arg;
};

typedef struct callback_t callback_t;
struct callback_t {
    void (*handler) (void *arg);
    void *arg;
};


typedef struct private_event_t private_event_t;
struct private_event_t {
    /**
     * @brief public interface
     */
    event_t public;

    /**
     * @brief event listening on thread
     */
    thread_t *thread;

    /**
     * @brief event handle information
     */
    /**
     * @brief max fd of listening on
     */
    int max_fd;

    /**
     * @brief fd sets listening on
     */
    fd_set rfds;

    /**
     * @brief fd sets listening on
     */
    fd_set wfds;
#define select_rfds rfds
#define select_wfds wfds

    /**
     * @brief select or epoll timeout
     */
    unsigned int timeout;

    /**
     * @brief stand for add or delete, flag 1 when
     *        add event, flag -1, when delete event
     */
    int flag;

    /**
     * @brief select or epoll timeout handle 
     */
    callback_t timeout_handler;

    /**
     * @brief select or epoll error handle 
     */
    callback_t error_handler;

    /**
     * @brief event package
     */
    linked_list_t *evts;

    /**
     * @brief evt_index
     */
    int evt_index;
};
static private_event_t *local_free_pointer = NULL;

int find_evt_pkg_by_fd(void *item, void *key)
{
    event_pkg_t *pkg = (event_pkg_t *)item;
    fd_set fds = *(fd_set *)key;

    if (FD_ISSET(pkg->fd, &fds)) {
        return 0;
    }

    return 1;
}

static int find_evt_pkg_by_pkg(void *item, void *key)
{
    event_pkg_t *opkg = (event_pkg_t *)item;
    event_pkg_t *dpkg = (event_pkg_t *)key;

    if (opkg->fd == dpkg->fd && opkg->type == dpkg->type) {
        return 0;
    }

    return 1;
}

static volatile int thread_onoff = 1;
void *select_events_handler(private_event_t *this)
{
    int    ready_fds_cnt = 0;
    int    read_bytes    = 0;
    struct timeval tv    = {0};
    event_pkg_t *evt_pkg;
    event_pkg_t evt;
    SOCKET      evt_fd;
    fd_set      rfds, wfds;

    while (thread_onoff) {
        /**
         * set event fds
         */
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        rfds = this->select_rfds;
        wfds = this->select_wfds;

        /**
         * wait time
         */
        tv.tv_sec  = this->timeout / 1000;
        tv.tv_usec = this->timeout % 1000 * 1000;
    
        /**
         * wait socket event
         */
        ready_fds_cnt = select(this->max_fd + 1, &rfds, &wfds, NULL, &tv);
        switch (ready_fds_cnt) {
            case 0:
                if (this->timeout_handler.handler != NULL) this->timeout_handler.handler(this->timeout_handler.arg);
                break;
            case -1:
                if (this->error_handler.handler != NULL) this->error_handler.handler(this->error_handler.arg);
                thread_onoff = 0;
                break;
            default:
                while (ready_fds_cnt-- > 0) {
                    
                    this->evts->reset_enumerator(this->evts);
                    while (this->evts->enumerate(this->evts, (void **)&evt_pkg)) {
                        if (FD_ISSET(evt_pkg->fd, &rfds)) {
                            break;
                        }
                        evt_pkg = NULL;
                        
                    }
                    if (!evt_pkg) {
                        break;
                    }
                    evt_fd = evt_pkg->fd;

                    IOCTL_READ_BYTES(evt_fd, read_bytes);
                    evt.fd  = evt_fd;
                    evt_pkg = NULL;

                    if (!read_bytes) {
                        evt.type = EVENT_ON_ACCEPT;
                        this->evts->find_first(this->evts, (void **)&evt_pkg, &evt, find_evt_pkg_by_pkg);

                        if (!evt_pkg) {
                            evt.type = EVENT_ON_CLOSE;
                            this->evts->find_first(this->evts, (void **)&evt_pkg, &evt, find_evt_pkg_by_pkg);
                        }
                    } else {
                        evt.type = EVENT_ON_RECV;
                        this->evts->find_first(this->evts, (void **)&evt_pkg, &evt, find_evt_pkg_by_pkg);
                    }

                    if (evt_pkg && evt_pkg->event_handler) {
                        evt_pkg->event_handler(evt_pkg->fd, evt_pkg->arg);

                        /**
                         * remove closed fd
                         */
                        if (evt_pkg->type == EVENT_ON_CLOSE) {
                            FD_CLR(evt_pkg->fd, &this->rfds);
                            FD_CLR(evt_pkg->fd, &this->wfds);
                            this->evts->remove(this->evts, evt_pkg, NULL);

                            this->evts->reset_enumerator(this->evts);
                            while (this->evts->enumerate(this->evts, (void **)&evt_pkg)) {
                                if (evt_pkg->fd == evt_fd) {
                                    FD_CLR(evt_pkg->fd, &this->rfds);
                                    FD_CLR(evt_pkg->fd, &this->wfds);
                                    this->evts->remove(this->evts, evt_pkg, NULL);
                                    this->evts->reset_enumerator(this->evts);
                                }
                                evt_pkg = NULL;
                            }
                        }
                    }
                }

                break;
        }
    }

    return NULL;
}

static void signal_handler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            local_free_pointer->public.destroy(&local_free_pointer->public);
            exit(1);
            break;
        default:
            break;
    }
}

METHOD(event_t, add_, int, private_event_t *this, SOCKET fd, event_type_t type, void (*handler) (SOCKET fd, void *arg), void *arg)
{
    event_pkg_t *pkg = NULL;
    event_pkg_t evt  = {0};
    int new_flag     = 0;

    if (!handler || fd < 1) return -1;

    evt.fd   = fd;
    evt.type = type;
    this->evts->find_first(this->evts, (void **)&pkg, &evt, find_evt_pkg_by_pkg);
    if (!pkg) {
        pkg = (event_pkg_t *)malloc(sizeof(event_pkg_t));
        new_flag = 1;
    }

    /**
     * event package init
     */
    pkg->fd   = fd;
    pkg->type = type;
    pkg->arg  = arg;
    pkg->event_handler = handler;

    if (new_flag) {
        this->evts->insert_last(this->evts, pkg);
        FD_SET(fd, &this->rfds);
        FD_SET(fd, &this->wfds);

        /**
         * set select max fd and fdsets
         */
        if (fd > this->max_fd) {
            this->max_fd = fd;
        }
    }

    this->flag = 1;
    return 0;
}

METHOD(event_t, delete_, int, private_event_t *this, SOCKET fd, event_type_t type)
{
    event_pkg_t *pkg = NULL;
    event_pkg_t dpkg = {0};

    dpkg.fd   = fd;
    dpkg.type = type;
    this->evts->find_first(this->evts, (void **)&pkg, &dpkg, find_evt_pkg_by_pkg);
    if (pkg) {
        FD_CLR(fd, &this->rfds);
        FD_CLR(fd, &this->wfds);
        this->evts->remove(this->evts, pkg, NULL);
    }
    return 0;
}

METHOD(event_t, destroy_, void, private_event_t *this)
{
    if (this->thread != NULL) {
        thread_onoff = 0;
#ifndef _WIN32
        usleep(100);
#else 
        Sleep(100);
#endif
        this->thread->cancel(this->thread);
    }
    if (this->evts) {
        this->evts->clear(this->evts);
        this->evts->destroy(this->evts);
    }
   
    free(this);
}

METHOD(event_t, exception_handle_, void, private_event_t *this, exception_type_t type, void (*handler) (void *), void *arg)
{
    switch (type) {
        case EXCEPTION_TIMEOUT:
            this->timeout_handler.handler = handler;
            this->timeout_handler.arg = arg;
            break;
        case EXCEPTION_ERROR:
            this->error_handler.handler = handler;
            this->error_handler.arg = arg;
            break;
        default:
            break;
    }
}

static int start_event_capture(private_event_t *this)
{
    void *handler = NULL;
    
    /**
     * act signal
     */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /**
     * act socket event
     */
    handler = (void *)select_events_handler;
    this->thread = thread_create(handler, this);
    if (!this->thread) return -1;

    return 0;
}

event_t *event_create(int timeout)
{
    private_event_t *this;

#ifndef _WIN32
    INIT(this,
        .public = {
            .add     = _add_,
            .delete  = _delete_,
            .destroy = _destroy_,
            .exception_handle = _exception_handle_,
        },
        .thread     = NULL,
        .flag       = 0,
        .timeout    = timeout < 0 ? 0 : timeout,
        .evts       = linked_list_create(),
    );
#else 
    INIT(this, private_event_t, 
        {
            add_,
            delete_,
            destroy_,
            exception_handle_,
        },
        NULL,
        0,
        0,
        0,
        timeout < 0 ? 0 : timeout,
        0,
        NULL,
        NULL,
        NULL,
        0,
    );

    this->evts = linked_list_create();
#endif

    if (start_event_capture(this) < 0) {
#ifndef _WIN32
        _destroy_(this);
#else 
        destroy_(this);
#endif
        return NULL;
    }

    local_free_pointer = this;
    return &this->public;
}
