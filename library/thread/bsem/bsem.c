#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>
#ifndef _WIN32
#include <utils/utils.h>
#include <bsem.h>
#include <sys/time.h>
#include <semaphore.h>
#else
#include "utils.h"
#include "bsem.h"
#include <windows.h>
#endif

typedef struct private_bsem_t private_bsem_t;

/**
 * private data of a bsem
 */
struct private_bsem_t {
	/**
	 * public interface
	 */
	bsem_t public;

	SEM_HANDLE sem;
};

METHOD(bsem_t, wait_, void, private_bsem_t *this)
{
    SEM_WAIT(this->sem); 
}

#ifndef _WIN32
/**
 * Get a timestamp from a monotonic time source.
 *
 * While the time()/gettimeofday() functions are affected by leap seconds
 * and system time changes, this function returns ever increasing monotonic
 * time stamps.
 *
 * @param tv		timeval struct receiving monotonic timestamps, or NULL
 * @return			monotonic timestamp in seconds
 */
static long time_monotonic_static(struct timeval *tv)
{
#if defined(HAVE_CLOCK_GETTIME) && \
    (defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
     defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
    /* as we use time_monotonic_static() for condvar operations, we use the
     * monotonic time source only if it is also supported by pthread. */
    struct time_t ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        if (tv)
        {
            tv->tv_sec = ts.tv_sec;
            tv->tv_usec = ts.tv_nsec / 1000;
        }
        return ts.tv_sec;
    }
#endif /* HAVE_CLOCK_GETTIME && (...) */
    /* Fallback to non-monotonic timestamps:
     * On MAC OS X, creating monotonic timestamps is rather difficult. We
     * could use mach_absolute_time() and catch sleep/wakeup notifications.
     * We stick to the simpler (non-monotonic) gettimeofday() for now.
     * But keep in mind: we need the same time source here as in condvar! */
    if (!tv)
    {
        return time(NULL);
    }
    if (gettimeofday(tv, NULL) != 0)
    {	/* should actually never fail if passed pointers are valid */
        return -1;
    }
    return tv->tv_sec;
}

METHOD(bsem_t, timed_wait_abs, int, private_bsem_t *this, struct timeval tv)
{
	struct timespec ts;

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000;

	/* there are errors other than ETIMEDOUT possible, but we consider them
	 * all as timeout */
	return sem_timedwait(&this->sem, &ts) == -1;
}

METHOD(bsem_t, timed_wait, int, private_bsem_t *this, unsigned int timeout)
{
	struct timeval tv, add;

	add.tv_sec = timeout / 1000;
	add.tv_usec = (timeout % 1000) * 1000;

	time_monotonic_static(&tv);
	timeradd(&tv, &add, &tv);

	return timed_wait_abs(this, tv);
}
#endif

METHOD(bsem_t, post, void, private_bsem_t *this)
{
    SEM_POST(this->sem); 
}

METHOD(bsem_t, destroy, void, private_bsem_t *this)
{
    SEM_DESTROY(this->sem); 
    free(this);
}

/*
 * Described in header
 */
bsem_t *bsem_create(unsigned int value)
{
	private_bsem_t *this;

#ifndef _WIN32
	INIT(this,
		.public = {
			.wait           = _wait_,
			.timed_wait     = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.post           = _post,
			.destroy        = _destroy,
		},
	);

	sem_init(&this->sem, 0, value);
#else
    INIT(this, private_bsem_t, 
        {
            wait_, 
            post,
            destroy,
        },
    );

    this->sem = CreateSemaphore(NULL, value, value + 1, NULL);
#endif

	return &this->public;
}

/**
 * @brief destroy binary semaphore and free memory
 *
 * @param cond
 */
void bsem_destroy(bsem_t *bsem)
{
    if (bsem == NULL) return;
    bsem->destroy(bsem);
    free(bsem);
}
