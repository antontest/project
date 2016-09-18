#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#else
#include <windows.h>
#endif

#ifndef _WIN32
typedef pthread_t THREAD_HANDLE;
#define GET_THREAD_ID() pthread_self()
#define THREAD_JOIN(id) pthread_join(id, NULL)
#else
typedef HANDLE THREAD_HANDLE;
#define GET_THREAD_ID() GetCurrentThreadId()
#define THREAD_JOIN(id) WaitForSingleObject(id, INFINITE) 
#endif

typedef struct thread_t thread_t;

/**
 * Main function of a thread.
 *
 * @param arg			argument provided to constructor
 * @return				value provided to threads joining the terminating thread
 */
typedef void *(*thread_main_t)(void *arg);

/**
 * Cleanup callback function for a thread.
 *
 * @param arg			argument provided to thread_cleanup_push
 */
typedef void (*thread_cleanup_t)(void *arg);


/**
 * Thread wrapper implements simple, portable and advanced thread functions.
 *
 * @note All threads other than the main thread need either to be joined or
 * detached by calling the corresponding method.
 */
struct thread_t {
    /**
     * @brief start thread
     */
    int (*start)(thread_t *this, thread_main_t main, void *arg);

	/**
	 * Cancel this thread.
	 */
	void (*cancel)(thread_t *this);

	/**
	 * Send a signal to this thread.
	 *
	 * @param sig		the signal to be sent to this thread
	 */
	void (*kill)(thread_t *this, int sig);

	/**
	 * Detach this thread, this automatically destroys the thread object after
	 * the thread returned from its main function.
	 *
	 * @note Calling detach is like calling destroy on other objects.
	 */
	void (*detach)(thread_t *this);

	/**
	 * Join this thread, this automatically destroys the thread object
	 * afterwards.
	 *
	 * @note Calling join is like calling destroy on other objects.
	 *
	 * @return			the value returned from the thread's main function or
	 *					a call to exit.
	 */
	void *(*join)(thread_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (thread_t *this);

    /**
     * @brief get thread id
     */
    int (*get_id) (thread_t *this);
};


/**
 * Create a new thread instance.
 *
 * @param main			thread main function
 * @param arg			argument provided to the main function
 * @return				thread instance
 */
thread_t *thread_create(thread_main_t main, void *arg);

/**
 * @brief dthread_create for shared library load.
 */
thread_t *dthread_create();

#endif /** THREADING_THREAD_H_ @} */
