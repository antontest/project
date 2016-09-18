#include "pool.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <thread/thread.h>
#include <bsem/bsem.h>
#include <mutex/mutex.h>
#include <utils/utils.h>
#include <linked_list/linked_list.h>
#else 
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>>
#include <windows.h>
#include "thread.h"
#include "bsem.h"
#include "mutex.h"
#include "utils.h"
#include "linked_list.h"
#endif

typedef struct private_pool_t private_pool_t;
struct private_pool_t {
    /**
     * @brief public interface
     */
    pool_t public;

    /**
     * @brief pool whether created succ
     */
    int created;

    /**
     * @brief enable or disable thread manager
     */
    int enable_thread_manager;

    /**
     * @brief control task manager thread stop
     */
    int task_manager_stop;

    /**
     * @brief control thread manager thread stop
     */
    int thread_manager_stop;

    /**
     * @brief thread pool size
     */
    int min_size;

    /**
     * @brief thread pool size
     */
    int cur_size;

    /**
     * @brief thread pool size
     */
    int max_size;

    /**
     * @brief list of busy thread
     */
    linked_list_t *busy_thread_list;
    
    /**
     * @brief list of idle thread
     */
    linked_list_t *idle_thread_list;

    /**
     * @brief task in this pool
     */
    linked_list_t *task_list;

    /**
     * @brief thread of managing task
     */
    thread_t *task_list_manager;

    /**
     * @brief thread of managing thread
     */
    thread_t *thread_list_manager;

    /**
     * @brief lock of thread list;
     */
    mutex_t *thread_list_lock;

    /**
     * @brief lock of task list;
     */
    mutex_t *task_list_lock;

    /**
     * @brief deal with task add and remove
     */
    bsem_t *has_work;

    /**
     * @brief deal with task add and remove
     */
    bsem_t *has_idle_thread;
};
#define pidle_thread_list this->idle_thread_list
#define pbusy_thread_list this->busy_thread_list
#define ptask_list        this->task_list
#define pool_min_size     this->min_size
#define pool_cur_size     this->cur_size
#define pool_max_size     this->max_size
#define task_lock         this->task_list_lock
#define task_manager      this->task_list_manager
#define thread_manager    this->thread_list_manager
#define pool_has_work     this->has_work
#define has_idle_pthread  this->has_idle_thread
#define pthread_list_lock this->thread_list_lock

static linked_list_t *pool_list = NULL;
typedef struct thread_task_t thread_task_t;
struct thread_task_t {
    void (*work) (void *);
    void *arg;
};

thread_task_t *create_thread_task(void (*work) (void *), void *arg)
{
    thread_task_t *this;

#ifndef _WIN32
    INIT(this,
        .work = work,
        .arg  = arg,
    );
#else
    INIT(this, thread_task_t, 
        work,
        arg,
    );
#endif

    return this;
}

typedef enum thread_state_t thread_state_t;
enum thread_state_t {
    THREAD_IDLE = 0,
    THREAD_WORKING
};

typedef struct thread_pkg_t thread_pkg_t;
struct thread_pkg_t {
    /**
     * @brief thread id
     */
    int id;

    /**
     * @brief control thread stop
     */
    int stop;

    /**
     * @brief thread state, idle or working
     */
    thread_state_t state;

    /**
     * @brief thread task
     */
    thread_task_t *task;

    /**
     * @brief task lock
     */
    mutex_t *lock;

    /**
     * @brief thread
     */
    thread_t *thread;

    /**
     * @brief wait_job
     */
    bsem_t *wait_job;

    /**
     * @brief belong to
     */
    private_pool_t *pool;

    /**
     * @brief thread start time
     */
    tclock_t start_time;

    /**
     * @brief thread idle time
     */
    tclock_t idle_time;
};
#define thread_pool_thread_list_lock this->pool->thread_list_lock
#define thread_pool_has_idle_thread  this->pool->has_idle_thread

thread_pkg_t *create_thread_pkg()
{
    thread_pkg_t *this;

#ifndef _WIN32
    INIT(this,
        .id       = -1,
        .stop     = 0,
        .wait_job = NULL,
        .thread   = NULL,
    );
#else
    INIT(this, thread_pkg_t, 
        -1,
        0,
        THREAD_IDLE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        0,
    );
#endif

    return this;
}

/**
 * @brief thread handler 
 */
static void thread_handler(thread_pkg_t *this)
{
    while (!this->stop) {
        /**
         * thread waiting for job
         */ 
        this->wait_job->wait(this->wait_job);
        if (this->stop) break;

        /**
         * if has work to do
         */
        if (this->task && this->task->work != NULL) {
            this->lock->lock(this->lock);
            this->state = THREAD_WORKING;
            this->task->work(this->task->arg);
            free(this->task);
            this->task = NULL;

            thread_pool_thread_list_lock->lock(thread_pool_thread_list_lock);
            GETCURRTIME(this->idle_time);
            this->state = THREAD_IDLE;
            thread_pool_thread_list_lock->unlock(thread_pool_thread_list_lock);
            this->lock->unlock(this->lock);
        } else {
            this->wait_job->post(this->wait_job);
        }
    }
}

/**
 * @brief create new thread to thread pool
 */
static int thread_pkg_init(private_pool_t *this)
{
    thread_pkg_t *thread = NULL;

    /**
     * create thread information package
     */
    thread = create_thread_pkg();
    if (!thread) return -1;

    /**
     * create thread task
     */
    thread->task = create_thread_task(NULL, NULL);
    if (!thread->task) return -1;

    /**
     * create thread bsem and lock
     */
    thread->wait_job = bsem_create(0);
    if (!thread->wait_job) return -1;
    thread->lock = mutex_create();
    if (!thread->lock) return -1;

    /**
     * start task thread
     */
    thread->thread = thread_create((void *)thread_handler, thread);
    if (!thread->thread) return -1;

    /**
     * init thread information
     */
    thread->id    = thread->thread->get_id(thread->thread);
    thread->state = THREAD_IDLE;
    thread->pool  = this;
    GETCURRTIME(thread->start_time);

    /**
     * add thread to pool thread list
     */
    pthread_list_lock->lock(pthread_list_lock);
    pidle_thread_list->insert_last(pidle_thread_list, thread);
    pool_cur_size++;
    pthread_list_lock->unlock(pthread_list_lock);
    has_idle_pthread->post(has_idle_pthread);

    return 0;
}

static void thread_pkg_deinit(private_pool_t *this, thread_pkg_t *thread)
{
    /**
     * remove thread from pool thread list
     */
    if (!thread) return;

    /**
     * stop thread
     */
    thread->stop = 1;
    USLEEP(10);
    thread->wait_job->post(thread->wait_job);
    USLEEP(10);

    /**
     * free thread memory
     */
    thread->wait_job->destroy(thread->wait_job);
    thread->lock->destroy(thread->lock);
    free(thread);

    thread = NULL;
    pool_cur_size--;
}

METHOD(pool_t, destroy_, void, private_pool_t *this)
{
    int task_cnt   = 0;
    int thread_cnt = 0;
    thread_task_t *task  = NULL;
    thread_pkg_t *thread = NULL;

    /**
     * destroy manager thread 
     */
    if (thread_manager) {
        this->thread_manager_stop = 1;
    }

    /**
     * destroy manager thread and free memory
     */
    if (task_manager) {
        this->task_manager_stop = 1;
        if (has_idle_pthread) has_idle_pthread->post(has_idle_pthread);
        if (pool_has_work) pool_has_work->post(pool_has_work);

        USLEEP(10);
        task_manager->cancel(task_manager);
    }

    /**
     * destroy thread pool
     */
    if (pidle_thread_list) {
        pthread_list_lock->lock(pthread_list_lock);
        thread_cnt = pidle_thread_list->get_count(pidle_thread_list);
        pthread_list_lock->unlock(pthread_list_lock);

        while (thread_cnt-- > 0) {
            pthread_list_lock->lock(pthread_list_lock);
            pidle_thread_list->top(pidle_thread_list, (void **)&thread);
            pthread_list_lock->unlock(pthread_list_lock);
            thread_pkg_deinit(this, thread);

			thread = NULL;
        }
        free(pidle_thread_list);
    }
    if (pbusy_thread_list) {
        pthread_list_lock->lock(pthread_list_lock);
        thread_cnt = pbusy_thread_list->get_count(pbusy_thread_list);
        pthread_list_lock->unlock(pthread_list_lock);

        while (thread_cnt-- > 0) {
            pthread_list_lock->lock(pthread_list_lock);
            pbusy_thread_list->top(pbusy_thread_list, (void **)&thread);
            pthread_list_lock->unlock(pthread_list_lock);
            thread_pkg_deinit(this, thread);
			thread = NULL;
        }
        free(pbusy_thread_list);
    }

    /**
     * free task
     */
    if (ptask_list) {
        task_cnt = ptask_list->get_count(ptask_list);
        while (task_cnt-- > 0) {
            ptask_list->remove_first(ptask_list, (void **)&task);
            if (task) free(task);

			task = NULL;
        }
        free(ptask_list);
    }

    /**
     * free lock and wait_job
     */
    if (pthread_list_lock) pthread_list_lock->destroy(pthread_list_lock);
    if (task_lock) task_lock->destroy(task_lock);
    if (pool_has_work) pool_has_work->destroy(pool_has_work);
    if (has_idle_pthread) has_idle_pthread->destroy(has_idle_pthread);
    free(this);
}

/**
 * @brief thread manager handler 
 */
static void task_manager_handler(private_pool_t *this)
{
    thread_pkg_t *thread = NULL;

    while (!this->task_manager_stop) {   
        /**
         * waiting for jobs and idle thread
         */
        pool_has_work->wait(pool_has_work);
        has_idle_pthread->wait(has_idle_pthread);
        if (this->task_manager_stop) break;

        /**
         * find idle task
         */

        /**
         * get one thread, if working, then continue util find idle
         */
        thread = NULL;
        pthread_list_lock->lock(pthread_list_lock);
        pidle_thread_list->top(pidle_thread_list, (void **)&thread);
        pthread_list_lock->unlock(pthread_list_lock);
        if (!thread) continue;

        /**
         * get one job, let thread execute
         */
        task_lock->lock(task_lock);
        ptask_list->top(ptask_list, (void **)&thread->task);
        task_lock->unlock(task_lock);

        pthread_list_lock->lock(pthread_list_lock);
        pbusy_thread_list->insert_last(pbusy_thread_list, thread);
        pthread_list_lock->unlock(pthread_list_lock);
        thread->wait_job->post(thread->wait_job);
    }
}

/**
 * @brief thread manager handler 
 */
static void thread_manager_handler(private_pool_t *this)
{   
    int found_idle_cnt = 0;
    int thread_cnt     = 0;
    int free_time      = DFT_IDLE_THREAD_FREE_TIME * TIME_PER_SEC;
    int idle_stay_time = 0;
    thread_pkg_t *thread        = NULL;
    tclock_t cur_time           = {0};
    struct   timeval wait_time  = {0};

    while (!this->thread_manager_stop) {   
        wait_time.tv_sec  = 0;
        wait_time.tv_usec = 100000;
#ifndef _WIN32
        select(0, NULL, NULL, NULL, &wait_time);
#else
        USLEEP(1);
#endif
        if (this->thread_manager_stop) break;

        /**
         * create new thread to thread pool
         */
        if (ptask_list->get_count(ptask_list) > 1 && pool_cur_size < pool_max_size) {
            thread_pkg_init(this);
            continue;
        }

        /**
         * move busy_thread to idle_thread
         */
        found_idle_cnt = 0;

        pthread_list_lock->lock(pthread_list_lock);
        pbusy_thread_list->reset_current(pbusy_thread_list);
        thread_cnt = pbusy_thread_list->get_count(pbusy_thread_list);
        pthread_list_lock->unlock(pthread_list_lock);

        while (thread_cnt-- > 0) {
            thread = NULL;
            pthread_list_lock->lock(pthread_list_lock);
            pbusy_thread_list->top(pbusy_thread_list, (void **)&thread);
            pthread_list_lock->unlock(pthread_list_lock);
            if (!thread) break;

            if (thread->state == THREAD_IDLE) {
                pthread_list_lock->lock(pthread_list_lock);
                pidle_thread_list->insert_last(pidle_thread_list, thread);
                pthread_list_lock->unlock(pthread_list_lock);
                found_idle_cnt++;
            } else {
                pthread_list_lock->lock(pthread_list_lock);
                pbusy_thread_list->insert_last(pbusy_thread_list, thread);
                pthread_list_lock->unlock(pthread_list_lock);
            }
            thread = NULL;
        }

        pthread_list_lock->lock(pthread_list_lock);
        while (found_idle_cnt-- > 0) {
            has_idle_pthread->post(has_idle_pthread);
        }
        pthread_list_lock->unlock(pthread_list_lock);

        /**
         * free idle thread
         */
        if (this->cur_size <= this->min_size) continue;
        pidle_thread_list->reset_current(pidle_thread_list);

        pthread_list_lock->lock(pthread_list_lock);
        thread_cnt = pidle_thread_list->get_count(pidle_thread_list);
        pthread_list_lock->unlock(pthread_list_lock);
        while (thread_cnt-- > 0) {
			thread = NULL;
            pthread_list_lock->lock(pthread_list_lock);
            pidle_thread_list->top(pidle_thread_list, (void **)&thread);
            pthread_list_lock->unlock(pthread_list_lock);
            if (!thread) break;

            GETCURRTIME(cur_time);
            idle_stay_time = (unsigned int)GETCOSTTIME(cur_time, (thread->idle_time)) * TIME_PER_SEC;
            if (idle_stay_time < 0 || idle_stay_time < free_time) {
                pthread_list_lock->lock(pthread_list_lock);
                pidle_thread_list->insert_last(pidle_thread_list, thread);
                pthread_list_lock->unlock(pthread_list_lock);
                continue;
            }

            /**
             * free idle thread
             */
            thread_pkg_deinit(this, thread);
            thread = NULL;

            if (this->thread_manager_stop) {
                break;
            }
        }
    }
}

/**
 * @brief error handler 
 */
static void error_handler(int sig)
{
    void *pool_ptr = NULL;
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            if (pool_list) {
                while (pool_list->remove_first(pool_list, &pool_ptr) != NOT_FOUND) {
#ifndef _WIN32
                    _destroy_(pool_ptr);
#else
                    destroy_(pool_ptr);
#endif
                }
                pool_list->destroy(pool_list);
            }
            exit(1);
            break;
    }
}

/**
 * @brief init thread pool 
 */
static int init_pool(private_pool_t *this)
{
    int i = 0;

    /**
     * act signal handler
     */
    signal(SIGINT, error_handler);
    signal(SIGTERM, error_handler);

    /**
     * create thread in pool
     */
    for (i = 0; i < pool_min_size; i++) {
        if (thread_pkg_init(this) < 0) break;
    }
    if (i < pool_min_size) return -1;

    /**
     * 1. create manager thread
     * 2. create wait_job
     * 3. create thread lock
     */
    task_manager = thread_create((void *)task_manager_handler, this);
    if (!task_manager) return -1;
    if (this->enable_thread_manager) {
        thread_manager = thread_create((void *)thread_manager_handler, this);
        if (!thread_manager) return -1;
    }

    this->created = 1;

    return 0;
}

METHOD(pool_t, addjob_, int, private_pool_t *this, void (*job) (void *), void *arg)
{
    thread_task_t *task = NULL;

    /**
     * if pool is not created completed, wait
     */
    while (!this->created) USLEEP(1);
    if (!ptask_list) return -1;
        
    /**
     * create task
     */
    task = create_thread_task(job, arg);
    if (!task) return -1;

    /**
     * add task to pool task list
     */
    task_lock->lock(task_lock);
    ptask_list->insert_last(ptask_list, task);
    pool_has_work->post(pool_has_work);
    task_lock->unlock(task_lock);

    return 0;
}

pool_t *pool_create(int min_size, int max_size)
{
    private_pool_t *this;

#ifndef _WIN32
    INIT(this,
        .public = {
            .addjob  = _addjob_,
            .destroy = _destroy_,
        },
        .created               = 0,
        .enable_thread_manager = 1,
        .task_manager_stop     = 0,
        .thread_manager_stop   = 0,
        .min_size              = min_size,
        .cur_size              = 0,
        .max_size              = max_size,
        .busy_thread_list      = linked_list_create(),
        .idle_thread_list      = linked_list_create(),
        .task_list             = linked_list_create(),
        .task_list_manager     = NULL,
        .thread_list_manager   = NULL,
        .thread_list_lock      = mutex_create(),
        .task_list_lock        = mutex_create(),
        .has_work              = bsem_create(0),
        .has_idle_thread       = bsem_create(0),
    );
#else
    INIT(this, private_pool_t, 
        {
            addjob_,
            destroy_,
        },
        0,
        1,
        0,
        0,
        min_size,
        0,
        max_size,
        linked_list_create(),
        linked_list_create(),
        linked_list_create(),
        NULL,
        NULL,
        mutex_create(),
        mutex_create(),
        bsem_create(100),
        bsem_create(max_size),
    );
#endif

    /**
     * start thread pool
     */
    if (init_pool(this) != 0) {
#ifndef _WIN32
        _destroy_(this);
#else
        destroy_(this);
#endif
        return NULL;
    }

    /**
     * add this thread pool to pool list
     */
    if (!pool_list) pool_list = linked_list_create();
    pool_list->insert_last(pool_list, this);
    return &this->public;
}
