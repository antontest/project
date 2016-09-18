#include <stdio.h>

#include "thread.h"
#ifndef _WIN32
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/utils.h>
#include <mutex/mutex.h>
#include <bsem/bsem.h>
#else 
#include "utils.h"
#include "mutex.h"
#include "bsem.h"
#include <windows.h>
#endif


typedef struct private_thread_t private_thread_t;

struct private_thread_t {
	/**
	 * Public interface.
	 */
	thread_t public;

	/**
	 * Human-readable ID of this thread.
	 */
	unsigned int id;

	/**
	 * ID of the underlying thread.
	 */
	THREAD_HANDLE thread_id;

	/**
	 * Main function of this thread (NULL for the main thread).
	 */
	thread_main_t main;

	/**
	 * Argument for the main function.
	 */
	void *arg;

	/**
	 * Mutex to make modifying thread properties safe.
	 */
	mutex_t *mutex;

	/**
	 * Semaphore used to sync the creation/start of the thread.
	 */
	bsem_t *created;

	/**
	 * TRUE if this thread has been detached or joined, i.e. can be cleaned
	 * up after terminating.
	 */
	bool detached_or_joined;

	/**
	 * TRUE if the threads has terminated (cancelled, via thread_exit or
	 * returned from the main function)
	 */
	bool terminated;

};

typedef struct {
	/**
	 * Cleanup callback function.
	 */
	thread_cleanup_t cleanup;

	/**
	 * Argument provided to the cleanup function.
	 */
	void *arg;

} cleanup_handler_t;


/**
 * Next thread ID.
 */
static unsigned int next_id;

/**
 * Mutex to safely access the next thread ID.
 */
static mutex_t *id_mutex;

#define HAVE_PTHREAD_CANCEL
#ifndef HAVE_PTHREAD_CANCEL
/* if pthread_cancel is not available, we emulate it using a signal */
#ifdef ANDROID
#define SIG_CANCEL SIGUSR2
#else
#define SIG_CANCEL (SIGRTMIN+7)
#endif

/* the signal handler for SIG_CANCEL uses pthread_exit to terminate the
 * "cancelled" thread */
void cancel_signal_handler(int sig)
{
#ifndef _WIN32
	pthread_exit(NULL);
#else
#endif
}
#endif


/**
 * Destroy an internal thread object.
 *
 * @note The mutex of this thread object has to be locked, it gets unlocked
 * automatically.
 */
static void thread_destroy(private_thread_t *this)
{
	if (!this->terminated || !this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		return;
	}
	this->mutex->unlock(this->mutex);
	this->mutex->destroy(this->mutex);
	this->created->destroy(this->created);
	free(this);
}

METHOD(thread_t, cancel, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
#ifndef _WIN32
	pthread_cancel(this->thread_id);
#else
#endif
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, kill_, void,
	private_thread_t *this, int sig)
{
	this->mutex->lock(this->mutex);
#ifndef _WIN32
	pthread_kill(this->thread_id, sig);
#else 
#endif
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, detach, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
#ifndef _WIN32
	pthread_detach(this->thread_id);
#else
#endif
	this->detached_or_joined = TRUE;
	thread_destroy(this);
}

METHOD(thread_t, join, void*,
	private_thread_t *this)
{
	THREAD_HANDLE thread_id;
	void *val = NULL;

	this->mutex->lock(this->mutex);
	if (this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! CANNOT JOIN DETACHED THREAD !!!");
		return NULL;
	}

	thread_id = this->thread_id;
	this->detached_or_joined = TRUE;
	if (this->terminated)
	{
		/* thread has terminated before the call to join */
		thread_destroy(this);
    }
	else
	{
		/* thread_destroy is called when the thread terminates normally */
		this->mutex->unlock(this->mutex);
	}
#ifndef _WIN32
	pthread_join(thread_id, &val);
#else 
    WaitForSingleObject(thread_id, INFINITE);
#endif

#ifndef _WIN32
	return val;
#else
	return NULL;
#endif /* _WIN32 */
}

/**
 * Main cleanup function for threads.
 */
static void thread_cleanup(private_thread_t *this)
{
	this->mutex->lock(this->mutex);
	this->terminated = TRUE;
	thread_destroy(this);
	this = NULL;
}

METHOD(thread_t, destroy_, void, private_thread_t *this)
{
    thread_cleanup(this);
}

METHOD(thread_t, get_id_, int, private_thread_t *this)
{
    return this->id;
}

/**
 * Main function wrapper for threads.
 */
static void *thread_main(private_thread_t *this)
{
	void *res;

	this->created->wait(this->created);

	/* TODO: this is not 100% portable as pthread_t is an opaque type (i.e.
	 * could be of any size, or even a struct) */
	/*
#ifdef HAVE_GETTID
	fprintf(stdout, "created thread %.2d [%u]",
		 this->id, gettid());
#else
	fprintf(stdout, "created thread %.2d [%lx]",
		 this->id, (u_long)this->thread_id);
#endif
    */

	res = this->main(this->arg);
    this = NULL;
    return res;
}

METHOD(thread_t, start_, int, private_thread_t *this, thread_main_t main, void *arg)
{
    this->main = main;
    this->arg  = arg;

#ifndef _WIN32
	if (pthread_create(&this->thread_id, NULL, (void *)thread_main, this) != 0)
#else
	if ((this->thread_id = CreateThread(NULL, 0, (void*)thread_main, this, 0, NULL)) == NULL)
#endif
	{
		fprintf(stdout, "failed to create thread!");
		this->mutex->lock(this->mutex);
		thread_destroy(this);
		return 1;
	}

	id_mutex->lock(id_mutex);
	this->id = next_id++;
	id_mutex->unlock(id_mutex);
	this->created->post(this->created);

    return 0;
}

/**
 * Create an internal thread object.
 */
static private_thread_t *thread_create_internal()
{
	private_thread_t *this;

#ifndef _WIN32
	INIT(this,
		.public = {
            .start   = _start_,
			.cancel  = _cancel,
			.kill    = _kill_,
			.detach  = _detach,
			.join    = _join,
			.destroy = _destroy_,

			.get_id  = _get_id_,
		},
		.mutex = mutex_create(),
	);
#else
	INIT(this, private_thread_t,
		{
            start_,
			cancel,
			kill_,
			detach,
			join,
			destroy_,

			get_id_,
		},
		0,
		0,
		NULL,
		NULL,
		mutex_create(),
		0,
		0,
		0,
	);
#endif
	this->created = bsem_create(0);
	id_mutex = mutex_create();

	return this;
}

/**
 * Described in header.
 */
thread_t *thread_create(thread_main_t main, void *arg)
{
	private_thread_t *this = thread_create_internal();

	this->main = main;
	this->arg = arg;

#ifndef _WIN32
	if (pthread_create(&this->thread_id, NULL, (void *)thread_main, this) != 0)
#else
	if ((this->thread_id = CreateThread(NULL, 0, (void*)thread_main, this, 0, NULL)) == NULL)
#endif
	{
		fprintf(stdout, "failed to create thread!");
		this->mutex->lock(this->mutex);
		thread_destroy(this);
		return NULL;
	}
	id_mutex->lock(id_mutex);
	this->id = next_id++;
	id_mutex->unlock(id_mutex);
	this->created->post(this->created);

	return &this->public;
}

thread_t *dthread_create()
{
	private_thread_t *this = thread_create_internal();

	if (this) {
        return &this->public;
    }

    return NULL;
}

