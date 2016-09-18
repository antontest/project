#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "mutex.h"
#ifndef _WIN32
#include <utils/utils.h>
#include <pthread.h>
#include <stdint.h>
#else 
#include "utils.h"
#include <windows.h>
#endif

typedef struct private_mutex_t private_mutex_t;
typedef struct private_r_mutex_t private_r_mutex_t;

/**
 * private data of mutex
 */
struct private_mutex_t {

	/**
	 * public functions
	 */
	mutex_t public;

	/**
	 * wrapped pthread mutex
	 */
	MUTEX_HANDLE mutex;
};

/**
 * @brief lock threading
 */
METHOD(mutex_t, lock_, void, private_mutex_t *this)
{
	int err;

	err = MUTEX_LOCK(this->mutex);
	if (err)
	{
		fprintf(stdout, "!!! MUTEX LOCK ERROR: %s !!!", strerror(err));
	}
}

/**
 * @brief unlock threading
 */
METHOD(mutex_t, unlock_, void, private_mutex_t *this)
{
	int err;

	err = MUTEX_UNLOCK(this->mutex);
#ifndef _WIN32
	if (err)
	{
		fprintf(stdout, "!!! MUTEX UNLOCK ERROR: %s !!!", strerror(err));
	}
#endif
}

/**
 * @brief destroy threading
 */
METHOD(mutex_t, mutex_destroy_, void, private_mutex_t *this)
{
	MUTEX_DESTROY(this->mutex);
	free(this);
}

/**
 * @brief create a thread mutex/lock
 *
 * @return struct mutex
 */
mutex_t *mutex_create()
{
    private_mutex_t *this;

#ifndef _WIN32
    INIT(this,
        .public = {
            .lock = _lock_,
            .unlock = _unlock_,
            .destroy = _mutex_destroy_,
        },
    );

    pthread_mutex_init(&this->mutex, NULL);
#else
    INIT(this, private_mutex_t, 
        {
            lock_,
            unlock_, 
            mutex_destroy_,
        },
    );
    this->mutex = CreateMutex(NULL,false,NULL);
#endif
    return &this->public;
}

/**
 * @brief destroy mutex and free memory
 *
 * @param mutex
 */
void mutex_destroy(mutex_t *mutex)
{
    if (mutex == NULL) return;
    mutex->destroy(mutex);
    free(mutex);
}
