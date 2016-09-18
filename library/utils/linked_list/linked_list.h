#ifndef __LINKED_LIST_H_
#define __LINKED_LIST_H_

#include "enumerator.h"
#ifndef _WIN32
#include <utils/utils.h>
#else
#include "utils.h"
#endif

typedef struct linked_list_t linked_list_t;

/**
 * Method to match elements in a linked list (used in find_* functions)
 *
 * @param item			current list item
 * @param ...			user supplied data (only pointers, at most 5)
 * @return
 *						- TRUE, if the item matched
 *						- FALSE, otherwise
 */
typedef bool (*linked_list_match_t)(void *item, ...);

/**
 * Method to be invoked on elements in a linked list (used in invoke_* functions)
 *
 * @param item			current list item
 * @param ...			user supplied data (only pointers, at most 5)
 */
typedef void (*linked_list_invoke_t)(void *item, ...);

/**
 * Class implementing a double linked list.
 *
 * General purpose linked list. This list is not synchronized.
 */
struct linked_list_t {

    /**
     * Gets the count of items in the list.
     *
     * @return			number of items in list
     */
    int (*get_count) (linked_list_t *this);


    /**
     * Inserts a new item at the beginning of the list.
     *
     * @param item		item value to insert in list
     */
    void (*insert_first) (linked_list_t *this, void *item);

    /**
     * Removes the first item in the list and returns its value.
     *
     * @param item		returned value of first item, or NULL
     * @return			SUCCESS, or NOT_FOUND if list is empty
     */
    status_t (*remove_first) (linked_list_t *this, void **item);

    /**
     * Remove items from the list matching the given item.
     *
     * If a compare function is given, it is called for each item, with the
     * first parameter being the current list item and the second parameter
     * being the supplied item. Return TRUE from the compare function to remove
     * the item, return FALSE to keep it in the list.
     *
     * If compare is NULL, comparison is done by pointers.
     *
     * @param item		item to remove/pass to comparator
     * @param compare	compare function, or NULL
     * @return			number of removed items
     */
    int (*remove)(linked_list_t *this, void *item, bool (*compare)(void*,void*));

    /**
     * Returns the value of the first list item without removing it.
     *
     * @param item		returned value of first item
     * @return			SUCCESS, NOT_FOUND if list is empty
     */
    status_t (*get_first) (linked_list_t *this, void **item);

    /**
     * Returns the value of the current list item without removing it.
     *
     * @param item		returned value of first item
     * @return			SUCCESS, NOT_FOUND if list is empty
     */
    status_t (*get_next) (linked_list_t *this, void **item);

    /**
     * @brief reset current pointer
     */
    status_t (*reset_current) (linked_list_t *this);

    /**
     * Inserts a new item at the end of the list.
     *
     * @param item		value to insert into list
     */
    void (*insert_last) (linked_list_t *this, void *item);

    /**
     * Removes the last item in the list and returns its value.
     *
     * @param item		returned value of last item, or NULL
     * @return			SUCCESS, NOT_FOUND if list is empty
     */
    status_t (*remove_last) (linked_list_t *this, void **item);

    /**
     * Returns the value of the last list item without removing it.
     *
     * @param item		returned value of last item
     * @return			SUCCESS, NOT_FOUND if list is empty
     */
    status_t (*get_last) (linked_list_t *this, void **item);

    /**
     * Find the first matching element in the list.
     *
     * The first object passed to the match function is the current list item,
     * followed by the user supplied data.
     * If the supplied function returns TRUE this function returns SUCCESS, and
     * the current object is returned in the third parameter, otherwise,
     * the next item is checked.
     *
     * If match is NULL, *item and the current object are compared.
     *
     * @warning Only use pointers as user supplied data.
     *
     * @param match			comparison function to call on each object, or NULL
     * @param item			the list item, if found
     * @param ...			user data to supply to match function (limited to 5 arguments)
     * @return				SUCCESS if found, NOT_FOUND otherwise
     */
    //status_t (*find_first) (linked_list_t *this, linked_list_match_t match,
    status_t (*find_first) (linked_list_t *this, void **item, void *key, int (*cmp) (void *, void *));

#ifndef _WIN32
    /**
     * Invoke a method on all of the contained objects.
     *
     * If a linked list contains objects with function pointers,
     * invoke() can call a method on each of the objects. The
     * method is specified by an offset of the function pointer,
     * which can be evalutated at compile time using the offsetof
     * macro, e.g.: list->invoke(list, offsetof(object_t, method));
     *
     * @warning Only use pointers as user supplied data.
     *
     * @param offset	offset of the method to invoke on objects
     * @param ...		user data to supply to called function (limited to 5 arguments)
     */
    void (*invoke_offset) (linked_list_t *this, size_t offset, ...);

    /**
     * Invoke a function on all of the contained objects.
     *
     * @warning Only use pointers as user supplied data.
     *
     * @param function	offset of the method to invoke on objects
     * @param ...		user data to supply to called function (limited to 5 arguments)
     */
    void (*invoke_function) (linked_list_t *this, linked_list_invoke_t function, ...);

    /**
     * Clones a list and its objects using the objects' clone method.
     *
     * @param offset	offset ot the objects clone function
     * @return			cloned list
     */
    linked_list_t *(*clone_offset) (linked_list_t *this, size_t offset);
#endif

    /**
     * @brief clear linked_list
     */
    void (*clear) (linked_list_t *this);

    /**
     * Destroys a linked_list object.
     */
    void (*destroy) (linked_list_t *this);

    /**
     * Destroys a list and its objects using the destructor.
     *
     * If a linked list and the contained objects should be destroyed, use
     * destroy_offset. The supplied offset specifies the destructor to
     * call on each object. The offset may be calculated using the offsetof
     * macro, e.g.: list->destroy_offset(list, offsetof(object_t, destroy));
     *
     * @param offset	offset of the objects destructor
     */
#ifndef _WIN32
    void (*destroy_offset) (linked_list_t *this, size_t offset);
#endif

    /**
     * Destroys a list and its contents using a a cleanup function.
     *
     * If a linked list and its contents should get destroyed using a specific
     * cleanup function, use destroy_function. This is useful when the
     * list contains malloc()-ed blocks which should get freed,
     * e.g.: list->destroy_function(list, free);
     *
     * @param function	function to call on each object
     */
    void (*destroy_function) (linked_list_t *this, void (*)(void*));

    /**
     * @brief  create_enumerator
     */
    enumerator_t *(*create_enumerator) (linked_list_t *this);

    /**
     * @brief  enumerator list
     */
    bool (*enumerate) (linked_list_t *this, void **item);

    /**
     * @brief  reset_enumerator
     */
    void (*reset_enumerator) (linked_list_t *this);

    /**
     * @brief reverse list 
     */
    void (*reverse) (linked_list_t *this);

    /**
     * @brief remove first itme from list, not free
     */
    status_t (*top) (linked_list_t *this, void **item);

    /**
     * @brief print linked_list data
     */
    void (*print) (linked_list_t *this, void (*print_cb) (void *));

    /**
     * @brief sort data with bubble sort
     */
    bool (*bubble) (linked_list_t *this, int (*cmp) (void *, void *));

    /**
     * @brief sort data with merge sort
     */
    bool (*merge) (linked_list_t *this, int (*cmp) (void *, void *));
};

/**
 * Creates an empty linked list object.
 *
 * @return		linked_list_t object.
 */
linked_list_t *linked_list_create(void);


/**
 * Creates a linked list from a NULL terminated vararg list of items.
 *
 * @param first			first item
 * @param ...			subsequent items, terminated by NULL
 * @return				linked_list_t object, containing passed items
 */
linked_list_t *linked_list_create_with_items(void *first, ...);

#endif /** LINKED_LIST_H_ @}*/

