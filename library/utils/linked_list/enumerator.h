#ifndef __ENUMERATOR_H__
#define __ENUMERATOR_H__

typedef struct enumerator_t enumerator_t;
struct enumerator_t {
    /**
     * Enumerate collection.
     *
     * The enumerate function takes a variable argument list containing
     * pointers where the enumerated valu es get written.
     * 
     * @param ...variable list of enumerated items, implementation dependent
     * @returnTRUE if pointers returned
     */
    int (*enumerate)(enumerator_t *this, ...);

    /**
     * Destroy a enumerator instance.
     */
    void (*destroy)(enumerator_t *this);                
};

/**
 *  Create an enumerator which enumerates over nothing
 *  
 *  @return an enumerator over no values
 */
enumerator_t* enumerator_create_empty();

#endif /* __ENUMERATOR_H__ */
