#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "linked_list.h"
#ifndef _WIN32
#include <unistd.h>
#include <utils/utils.h>
#else
#include "utils.h"
#endif


typedef struct element_t element_t;

/**
 * This element holds a pointer to the value it represents.
 */
struct element_t {

    /**
     * Value of a list item.
     */
    void *value;

    /**
     * Previous list element.
     *
     * NULL if first element in list.
     */
    element_t *previous;

    /**
     * Next list element.
     *
     * NULL if last element in list.
     */
    element_t *next;
};

/**
 * Creates an empty linked list object.
 */
element_t *element_create(void *value)
{
    element_t *this;

#ifndef _WIN32
    INIT(this,
        .value = value,
        .previous = NULL, 
        .next = NULL,
    );
#else
    INIT(this, element_t, 
        value,
        NULL, 
        NULL,
    );
#endif
    return this;
}


typedef struct private_linked_list_t private_linked_list_t;

/**
 * Private data of a linked_list_t object.
 *
 */
struct private_linked_list_t {
    /**
     * Public part of linked list.
     */
    linked_list_t public;

    /**
     * Number of items in the list.
     */
    int count;

    /**
     * First element in list.
     * NULL if no elements in list.
     */
    element_t *first;

    /**
     * Current element in list.
     * NULL if no elements in list.
     */
    element_t *current;

    /**
     * Last element in list.
     * NULL if no elements in list.
     */
    element_t *last;

    /**
     * @brief flag of enumerator
     */
    bool finished;
};


typedef struct private_enumerator_t private_enumerator_t;
/**
 * linked lists enumerator implementation
 */
struct private_enumerator_t {

    /**
     * implements enumerator interface
     */
    enumerator_t enumerator;

    /**
     * associated linked list
     */
    private_linked_list_t *list;

    /**
     * current item
     */
    element_t *current;

    /**
     * enumerator has enumerated all items
     */
    bool finished;
};      

METHOD(linked_list_t, get_count, int, private_linked_list_t *this)
{
    return this->count;
}

METHOD(linked_list_t, insert_first, void, private_linked_list_t *this, void *item)
{
    element_t *element;

    element = element_create(item);
    if (this->count == 0) {
        /* first entry in list */
        this->first = element;
        this->last = element;
    } else {
        element->next = this->first;
        this->first->previous = element;
        this->first = element;
    }

    this->count++;
}

/**
 * unlink an element form the list, returns following element
 */
static element_t* remove_element(private_linked_list_t *this,
        element_t *element)
{
    element_t *next, *previous;

    next     = element->next;
    previous = element->previous;
    free(element);

    if (next) next->previous = previous;
    else this->last = previous;

    if (previous) previous->next = next;
    else this->first = next;

    if (--this->count == 0) {
        this->first = NULL;
        this->last = NULL;
    }

    return next;
}

METHOD(linked_list_t, get_first, status_t, private_linked_list_t *this, void **item)
{
    if (this->count == 0) return NOT_FOUND;
    if (this->first) *item = this->first->value;
    return SUCCESS;
}

METHOD(linked_list_t, reset_current, status_t, private_linked_list_t *this)
{
    if (this->count == 0) return NOT_FOUND;
    this->current  = this->first;
    this->finished = FALSE;
    return SUCCESS;
}

METHOD(linked_list_t, get_next, status_t,
        private_linked_list_t *this, void **item)
{
    if (this->count == 0) return NOT_FOUND;
    if (this->finished) return NOT_FOUND;
    if (!this->current) this->current = this->first;
    *item = this->current->value;
    this->current = this->current->next;
    if (!this->current) this->finished = TRUE;
    return SUCCESS;
}

METHOD(linked_list_t, remove_first, status_t,
        private_linked_list_t *this, void **item)
{
    if (get_first(this, item) == SUCCESS) {
        remove_element(this, this->first);
        return SUCCESS;
    }
    return NOT_FOUND;
}

METHOD(linked_list_t, insert_last, void,
        private_linked_list_t *this, void *item)
{
    element_t *element;

    element = element_create(item);
    if (this->count == 0) {
        /* first entry in list */
        this->first = element;
        this->last = element;
    } else {
        element->previous = this->last;
        this->last->next = element;
        this->last = element;
    }
    this->count++;
}

METHOD(linked_list_t, get_last, status_t, private_linked_list_t *this, void **item)
{
    if (this->count == 0) return NOT_FOUND;
    *item = this->last->value;
    return SUCCESS;
}

METHOD(linked_list_t, remove_last, status_t,
        private_linked_list_t *this, void **item)
{
    if (get_last(this, item) == SUCCESS) {
        remove_element(this, this->last);
        return SUCCESS;
    }
    return NOT_FOUND;
}

METHOD(linked_list_t, remove_, int,
        private_linked_list_t *this, void *item, bool (*compare)(void*,void*))
{
    element_t *current = this->first;
    int removed = 0;

    while (current) {
        if ((compare && compare(current->value, item)) ||
                (!compare && current->value == item)) {
            removed++;
            current = remove_element(this, current);
        } else {
            current = current->next;
        }
    }
    return removed;
}

/*
   METHOD(linked_list_t, find_first, status_t,
   private_linked_list_t *this, linked_list_match_t match,
   void **item, void *d1, void *d2, void *d3, void *d4, void *d5)
   {
   element_t *current = this->first;

   while (current)
   {
   if ((match && match(current->value, d1, d2, d3, d4, d5)) ||
   (!match && item && current->value == *item))
   {
   if (item != NULL)
   {
 *item = current->value;
 }
 return SUCCESS;
 }
 current = current->next;
 }
 return NOT_FOUND;
 }
 */

METHOD(linked_list_t, find_first, status_t,
        private_linked_list_t *this, 
        void **item, void *key, int (*cmp) (void *, void *))
{
    element_t *current = this->first;
    if (!cmp || !key || !item) return NOT_FOUND;

    while (current) {
        if (cmp && !cmp(current->value, key)) {
            if (item != NULL) *item = current->value;
            return SUCCESS;
        }
        current = current->next;
    }
    return NOT_FOUND;
}

#ifndef _WIN32
METHOD(linked_list_t, invoke_offset, void,
        private_linked_list_t *this, size_t offset,
        void *d1, void *d2, void *d3, void *d4, void *d5)
{
    element_t *current = this->first;
    linked_list_invoke_t *method;

    while (current) {
        method = current->value + offset;
        (*method)(current->value, d1, d2, d3, d4, d5);
        current = current->next;
    }
}

METHOD(linked_list_t, invoke_function, void,
        private_linked_list_t *this, linked_list_invoke_t fn,
        void *d1, void *d2, void *d3, void *d4, void *d5)
{
    element_t *current = this->first;

    while (current) {
        fn(current->value, d1, d2, d3, d4, d5);
        current = current->next;
    }
}

METHOD(linked_list_t, clone_offset, linked_list_t*,
        private_linked_list_t *this, size_t offset)
{
    element_t *current = this->first;
    linked_list_t *clone;

    clone = linked_list_create();
    while (current) {
        void* (**method)(void*) = current->value + offset;
        clone->insert_last(clone, (*method)(current->value));
        current = current->next;
    }

    return clone;
}
#endif

METHOD(linked_list_t, clear_, void, private_linked_list_t *this)
{
    void *element = NULL;
    int cnt = this->count;

    while (cnt-- > 0) {
#ifndef _WIN32
        _remove_first(this, &element);
#else 
        remove_first(this, &element);
#endif
        if (element) free(element);
        element = NULL;
    }
}

METHOD(linked_list_t, destroy, void,
        private_linked_list_t *this)
{
    void *value;

    /* Remove all list items before destroying list */
#ifndef _WIN32
    while (_remove_first(this, &value) == SUCCESS) {
#else
    while (remove_first(this, &value) == SUCCESS) {
#endif
        /* values are not destroyed so memory leaks are possible
         * if list is not empty when deleting */
    }
    free(this);
}

#ifndef _WIN32
METHOD(linked_list_t, destroy_offset, void,
        private_linked_list_t *this, size_t offset)
{
    element_t *current = this->first, *next;

    while (current) {
        void (**method)(void*) = current->value + offset;
        (*method)(current->value);
        next = current->next;
        free(current);
        current = next;
    }
    free(this);
}
#endif

METHOD(linked_list_t, destroy_function, void,
        private_linked_list_t *this, void (*fn)(void*))
{
    element_t *current = this->first, *next;

    while (current) {
        fn(current->value);
        next = current->next;
        free(current);
        current = next;
    }
    free(this);
}

METHOD(enumerator_t, enumerate, bool,
        private_enumerator_t *this, void **item)
{
    if (this->finished) return FALSE;
    if (!this->current) this->current = this->list->first;
    else this->current = this->current->next;

    if (!this->current) {
        this->finished = TRUE;
        return FALSE;
    }
    
    if (item) *item = this->current->value;
    return TRUE;
}       

METHOD(linked_list_t, create_enumerator, enumerator_t*, private_linked_list_t *this)
{
    private_enumerator_t *enumerator;

#ifndef _WIN32
    INIT(enumerator,
        .enumerator = {
            .enumerate  = (void*)_enumerate,
            .destroy    = (void*)free,
        },
        .list = this,
    );
#else
    INIT(enumerator, private_enumerator_t, 
        {
            (void*)enumerate,
            (void*)free,
        },
        this,
    );
#endif

    return &enumerator->enumerator;
}

METHOD(linked_list_t, enumerate_, bool, private_linked_list_t *this, void **item)
{
    if (this->finished) return FALSE;
    if (!this->current) this->current = this->first;
    else this->current = this->current->next;
    if (!this->current) {
        this->finished = TRUE;
        return FALSE;
    }
    if (item) *item = this->current->value;
    return TRUE;
}

METHOD(linked_list_t, reset_enumerator, void, private_linked_list_t *this, private_enumerator_t *enumerator)
{
    //enumerator->current = NULL;
    //enumerator->finished = FALSE;
    this->current = NULL;
    this->finished = FALSE;
}

METHOD(linked_list_t, reverse_, void, private_linked_list_t *this)
{
    element_t *pb = NULL, *pa = NULL, *pn = NULL;

    if (this->count < 2) return;
    pb = this->first;
    pa = this->first->next;

    pb->next = NULL;
    while (pa != NULL) {
        pb->previous = pa;
        pn = pa->next;
        pa->next = pb;

        pb = pa;
        pa = pn;
    }

    pb->previous = NULL;
    this->first = pb;
}

METHOD(linked_list_t, top_, status_t, private_linked_list_t *this, void **item)
{
    if (!this->first || this->count < 1 ) {
        item = NULL;
        return NOT_FOUND;
    }

    if (item) {
        *item = this->first->value;
    }
    this->first = this->first->next;
    if (this->first) {
        this->first->previous = NULL;
    }
    this->count--;

    return SUCCESS;
}

METHOD(linked_list_t, print_, void, private_linked_list_t *this, void (*print_cb) (void *))
{
    element_t *ps = this->first;

    while (ps != NULL) {
        print_cb(ps->value);
        ps = ps->next;
    }
}
    
METHOD(linked_list_t, bubble_, bool, private_linked_list_t *this, int (*cmp) (void *, void *))
{
    element_t *pc = NULL, *pb = NULL, *pn = NULL, *pe = NULL;
    if (!cmp) return false;
    if (this->count < 2) return true;

    pc = this->first;
    while (pc != this->first->next) {
        pb = NULL;
        pc = this->first;
        pn = this->first->next;
        while (pn != pe) {
            if (cmp(pc->value, pn->value) > 0) {
                if (pb) pb->next = pn;
                pc->next = pn->next;
                pn->next = pc;

                if (this->first == pc) {
                    this->first  = pn;
                    pc->previous = pn;
                    pn->previous = NULL;
                }
                pb = pn;
                if (pc) pn = pc->next;
            } else {
                pb = pc;
                pc = pn;
                pn = pn->next;
            }
        }
        pe = pc;
    }
    return true;
}

static void merge_sort(element_t **elh, int (*cmp) (void *, void *))
{
    element_t *pl = NULL, *pr  = NULL, *ple = NULL;
    element_t *ps = NULL, *pq  = NULL;
    element_t *ph = NULL, *prr = NULL;

    if (!elh || !(*elh)->next) return;

    ple = NULL;
    ps  = *elh;
    pq  = *elh;

    /**
     * find middle element of list
     */
    while (pq != NULL && pq->next != NULL) {
        ple = ps;
        ps  = ps->next;
        pq  = pq->next->next;
    }

    pl = *elh;
    pr = ps;
    ple->next = NULL;

    /**
     * merge sort
     */
    merge_sort(&pl, cmp);
    merge_sort(&pr, cmp);

    /**
     * merge
     */
    if (cmp(pl->value, pr->value) <= 0) {
        ph = prr = pl;
        pl = pl->next;
    } else {
        ph = prr = pr;
        pr = pr->next;
    }

    while (pl != NULL && pr != NULL) {
        if (cmp(pl->value, pr->value) <= 0) {
            prr->next = pl;
            pl = pl->next;
        } else {
            prr->next = pr;
            pr = pr->next;
        }
        prr = prr->next;
    }
    
    if (!pl) prr->next = pr;
    else if (!pr) prr->next = pl;

    *elh = ph;
}

METHOD(linked_list_t, merge_, bool, private_linked_list_t *this, int (*cmp) (void *, void *))
{
    element_t *pb = NULL, *pa = NULL;

    if (!cmp) return false;
    if (this->count < 2) return true;

    merge_sort(&this->first, cmp);

    this->first->previous = NULL;
    pb = this->first;
    if (this->first) pa = this->first->next;
    while (pa != NULL) {
        pa->previous = pb;
        pa = pa->next;
        pb = pb->next;
    }

    return true;
}

/*
 * Described in header.
 */
linked_list_t *linked_list_create()
{
    private_linked_list_t *this;

#ifndef _WIN32
    INIT(this,
        .public = {
            .get_count     = _get_count,
            .get_first     = _get_first,
            .get_next      = _get_next,
            .get_last      = _get_last,
            .reset_current = _reset_current,

            .find_first      = (void*)_find_first,
            .insert_first    = _insert_first,
            .insert_last     = _insert_last,
            .remove_first    = _remove_first,
            .remove_last     = _remove_last,
            .remove          = _remove_,
            .invoke_offset   = (void*)_invoke_offset,
            .invoke_function = (void*)_invoke_function,

            .clone_offset     = _clone_offset,
            .clear            = _clear_,
            .destroy          = _destroy,
            .destroy_offset   = _destroy_offset,
            .destroy_function = _destroy_function,

            .reverse           = _reverse_,
            .top               = _top_,
            .print             = _print_,
            .enumerate         = _enumerate_,
            .create_enumerator = _create_enumerator,
            .reset_enumerator  = (void*)_reset_enumerator,

            .bubble = _bubble_,
            .merge  = _merge_,
        },
        .first   = NULL,
        .current = NULL,
        .last    = NULL,
    );
#else
    INIT(this, private_linked_list_t, 
        {
            get_count,
            insert_first,
            remove_first,
            remove_,
            get_first,
            get_next,
            reset_current,
            insert_last,
            remove_last,
            get_last,

            (void*)find_first,
            // (void*)invoke_offset,
            // (void*)invoke_function,

            // clone_offset,
            clear_,
            destroy,
            // destroy_offset,
            destroy_function,

            create_enumerator,
            enumerate_,
            (void*)reset_enumerator,

            reverse_,
            top_,
            print_,

            bubble_,
            merge_,

        },
        0,
        NULL,
        NULL,
        NULL,
        0,
    );
#endif

    return &this->public;
}

/*
 * See header.
 */
linked_list_t *linked_list_create_with_items(void *item, ...)
{
    linked_list_t *list;
    va_list args;

    list = linked_list_create();

    va_start(args, item);
    while (item)
    {
        list->insert_last(list, item);
        item = va_arg(args, void*);
    }
    va_end(args);

    return list;
}

