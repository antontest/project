#ifndef __UTILS_H__
#define __UTILS_H__
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

/******************************************/
/*************  type define  **************/
/******************************************/
/**
* General purpose boolean type.
*/
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  define _Bool signed char
# endif /* HAVE__BOOL */
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif /* HAVE_STDBOOL_H */
#ifndef FALSE
# define FALSE false
#endif /* FALSE */
#ifndef TRUE
# define TRUE  true
#endif /* TRUE */

typedef char s8;
typedef unsigned char u8;
typedef short int s16;
typedef unsigned short int u16;
typedef int s32;
typedef unsigned int u32;
typedef enum status_t status_t;


/******************************************/
/*************  func define  **************/
/******************************************/

#ifndef _WIN32
#define SNPRINTF(buf, size, fmt, ...) \
    snprintf(buf, size, fmt, ##__VA_ARGS__)
#else
#define SNPRINTF(buf, size, fmt, ...) \
    _snprintf(buf, size, fmt, ##__VA_ARGS__)
#endif

#ifndef _WIN32 
#define ACCESS(path, mode) access(path, mode)
#else 
#define F_OK 0
#define X_OK 0
#define R_OK 4
#define W_OK 2
#define ACCESS(path, mode) _access(path, mode)
#endif

#ifndef _WIN32
#define SLEEP(t) sleep(t)
#define USLEEP(t) usleep((t) * 1000)
#else
#define SLEEP(t) Sleep((t) * 1000)
#define USLEEP(t) Sleep(t)
#endif /* _WIN32 */

/**
 * time type and functions
 */ 
#ifndef _WIN32
typedef struct timeval tclock_t;
#define TIME_PER_SEC 1000000
#define GETCURRTIME(tm) gettimeofday(&(tm), NULL)
#define GETCOSTTIME(end_time, start_time) (double)(((end_time.tv_sec - start_time.tv_sec) * TIME_PER_SEC + end_time.tv_usec - start_time.tv_usec) / ((double)TIME_PER_SEC))
#else 
typedef clock_t tclock_t;
#define TIME_PER_SEC 1000
#define GETCURRTIME(tm) (tm) = clock()
#define GETCOSTTIME(end_time, start_time) ((((double)(end_time) - (double)(start_time))) / ((double)TIME_PER_SEC))
#endif
#define CUL_MBPS(len, cnt, time) (double)((double)(cnt) * (double)(len) * 8 / (double)(time) / 1024 / 1024)

#define GET_CUR_TIME_STR(tm_str, size, tm_fmt) { \
        time_t lt; \
        struct tm *ptm = NULL; \
        lt = time(NULL); \
        ptm = localtime(&lt); \
        if (ptm) { \
            strftime(tm_str, sizeof(tm_str), tm_fmt, ptm); \
        } \
    }

/**
 * return and log
 */
#ifndef _WIN32
#define CHECK_RET(ret, fmt, ...) \
    if (ret) { \
        printf(fmt, __VA_ARGS__); \
        printf(" at line \033[1;35m[%d]\033[0m in file \033[1;35m[%s]\033[0m.\n", __LINE__, __FILE__); \
        return ret; \
    }
#else
#define CHECK_RET(ret, fmt, ...) \
    if (ret) { \
        printf(fmt, __VA_ARGS__); \
        printf(" at line [%d] in file [%s].\n", __LINE__, __FILE__); \
        return ret; \
    }
#endif

#ifndef _WIN32
#define GOTO_IF_ERR(ret, where, fmt, ...) \
    if (ret) { \
        printf(fmt, __VA_ARGS__); \
        printf(" at line \033[1;35m[%d]\033[0m in file \033[1;35m[%s]\033[0m.\n", __LINE__, __FILE__); \
        goto where; \
    }
#else
#define GOTO_IF_ERR(ret, where, fmt, ...) \
    if (ret) { \
        printf(fmt, __VA_ARGS__); \
        printf(" at line [%d] in file [%s].\n", __LINE__, __FILE__); \
        goto where; \
    }
#endif

/**
* Call destructor of an object, if object != NULL
*/
#define DESTROY_IF(obj) if (obj) (obj)->destroy(obj)
#define FREE_IF(obj) if (obj) { free(obj); obj = NULL;}

/**
* Call offset destructor of an object, if object != NULL
*/
#define DESTROY_OFFSET_IF(obj, offset) if (obj) obj->destroy_offset(obj, offset);

/**
* Call function destructor of an object, if object != NULL
*/
#define DESTROY_FUNCTION_IF(obj, fn) if (obj) obj->destroy_function(obj, fn);

/**
* Macro gives back larger of two values.
*/
#ifndef _WIN32
#define max(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; })


/**
* Macro gives back smaller of two values.
*/
#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; })
#endif

/**
* Debug macro to follow control flow
*/
#define POS printf("%s, line %d\n", __FILE__, __LINE__)

/**
* get variable address
*/
#define ADDR(var) ((void *)&var)

/**
* get address's address
*/
#define ADDR_ADDR(var) ((void **)&var)

/**
* Object allocation/initialization macro, using designated initializer.
*/
#ifndef _WIN32
#define INIT(this, ...) \
{ \
	(this) = malloc(sizeof(*(this))); \
	*(this) = (typeof(*(this))){ __VA_ARGS__ }; \
	}
#else
#define INIT(self, type, ...) \
{ \
	type tmp = { __VA_ARGS__ }; \
	(self) = malloc(sizeof(*(self))); \
	memcpy((self), &tmp, sizeof(tmp)); \
}
#endif

/**
* Method declaration/definition macro, providing private and public interface.
*
* Defines a method name with this as first parameter and a return value ret,
* and an alias for this method with a _ prefix, having the this argument
* safely casted to the public interface iface.
* _name is provided a function pointer, but will get optimized out by GCC.
*/
#ifndef _WIN32
#define METHOD(iface, name, ret, this, ...) \
	static ret name(union {iface *_public; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)
#else
#define METHOD(iface, name, ret, self, ...) \
	static ret name(self, ##__VA_ARGS__)
#endif

/**
* Same as METHOD(), but is defined for two public interfaces.
*/
#define METHOD2(iface1, iface2, name, ret, this, ...) \
	static ret name(union {iface1 *_public1; iface2 *_public2; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)


/**
* Macro to allocate a sized type.
*/
#define malloc_thing(thing) ((thing*)malloc(sizeof(thing)))

/**
* Get the number of elements in an array
*/
#define countof(array) (sizeof(array)/sizeof(array[0]))

/**
* Ignore result of functions tagged with warn_unused_result attributes
*/
#define ignore_result(call) { if(call){}; }

/**
* Assign a function as a class method
*/
#define ASSIGN(method, function) (method = (typeof(method))function)

/******************************************/
/****************  enum  ******************/
/******************************************/
/**
 * Return values of function calls.
 */
enum status_t {
	/**
	 * Call succeeded.
	 */
	SUCCESS,

	/**
	 * Call failed.
	 */
	FAILED,

	/**
	 * Out of resources.
	 */
	OUT_OF_RES,

	/**
	 * The suggested operation is already done
	 */
	ALREADY_DONE,

	/**
	 * Not supported.
	 */
	NOT_SUPPORTED,

	/**
	 * One of the arguments is invalid.
	 */
	INVALID_ARG,

	/**
	 * Something could not be found.
	 */
	NOT_FOUND,

	/**
	 * Error while parsing.
	 */
	PARSE_ERROR,

	/**
	 * Error while verifying.
	 */
	VERIFY_ERROR,

	/**
	 * Object in invalid state.
	 */
	INVALID_STATE,

	/**
	 * Another call to the method is required.
	 */
	NEED_MORE,
};

#endif

