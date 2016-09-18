/**
 * @defgroup bsem bsem
 * @{ @ingroup threading
 */

#ifndef __BSEM_H__
#define __BSEM_H__

/**
 * sem
 */
#ifndef _WIN32
#include <semaphore.h>
typedef sem_t SEM_HANDLE;
#define SEM_INIT(sem, value) sem_init(&(sem), 0, value)
#define SEM_WAIT(sem) sem_wait(&(sem))
#define SEM_POST(sem) sem_post(&(sem))
#define SEM_DESTROY(sem) sem_destroy(&sem)
#else
typedef void * SEM_HANDLE;
#define SEM_INIT(sem, value) (sem) = CreateSemaphore(NULL, value, value + 1, NULL)
#define SEM_WAIT(sem) WaitForSingleObject(sem, INFINITE)
#define SEM_POST(sem) ReleaseSemaphore(sem, 1, NULL)
#define SEM_DESTROY(sem) CloseHandle(sem)
#endif /* _WIN32 */


typedef struct bsem_t bsem_t;

/**
 * A bsem is basically an integer whose value is never allowed to be
 * lower than 0.  Two operations can be performed on it: increment the
 * value by one, and decrement the value by one.  If the value is currently
 * zero, then the decrement operation will blcok until the value becomes
 * greater than zero.
 */
struct bsem_t {

	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases.
	 */
	void (*wait)(bsem_t *this);

#ifndef _WIN32
	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases, or the
	 * call times out.
	 *
	 * @param timeout		timeout im ms
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	int (*timed_wait)(bsem_t *this, unsigned int timeout);

	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases, or the
	 * call times out.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param tv			absolute time until timeout
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	int (*timed_wait_abs)(bsem_t *this, struct timeval tv);
#endif

	/**
	 * Increase the value by one. If the value becomes greater than zero, then
	 * another thread waiting will be woken up.
	 */
	void (*post)(bsem_t *this);

	/**
	 * Destroy a bsem and free its resources.
	 */
	void (*destroy)(bsem_t *this);
};

/**
 * Create a bsem instance.
 *
 * @param value		initial value (typically 0)
 * @return			bsem instance
 */
bsem_t *bsem_create(unsigned int value);

/**
 * @brief destroy binary semaphore and free memory
 *
 * @param cond
 */
void bsem_destroy(bsem_t *bsem);

#endif /** THREADING_bsem_H_ @} */

