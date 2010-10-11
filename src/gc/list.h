#ifndef GC_LIST_H
#define GC_LIST_H

/*
** # GC Lists
**
** `gc_list_t` is a doubly linked list. You'll notice that there are
** no values other than the links forward and backward. This is because,
** for the most part, the `gc_list_t*` is assumed to be the pointer to
** the object itself.
**
** We actually use doubly linked lists with a dummy. This makes
** programming the list significantly less troublesome, but it brings the
** onus of keeping track of the dummy on the user. In this documentation,
** we always refer to the dummy of the list as a "list", and to any other
** piece of the list as an "item", despite both being (pointers to) the
** same structure.
*/
typedef struct gc_list_t gc_list_t;
struct gc_list_t {
    gc_list_t *next;
    gc_list_t *prev;
};


/*
** `gc_list_init` initializes (or reinitializes) a list. This involves
** making the list a list of one. I.e., the dummy points only to itself.
*/
void gc_list_init (gc_list_t* list);


/*
** `gc_list_pushBack` places an item at the back of the list. Likewise,
** `gc_list_pushFront` places an item at the front of the list.
*/
void gc_list_pushBack  (gc_list_t* list, gc_list_t* item);
void gc_list_pushFront (gc_list_t* list, gc_list_t* item);

/*
** `gc_list_unlink` removes an item from its surrounding list. The item's
** links are unchanged, so know that `item->prev` and `item->next` are
** consequently wrong.
*/
void gc_list_unlink (gc_list_t* item);

/*
** `gc_list_popFront` removes an item from the front of the list, and
** returns it. `gc_list_popBack` does the same, but to the back of the
** list instead.
**
** The returned item's links are not modified, so `item->prev` and
** `item->next` are both wrong.
*/
gc_list_t* gc_list_popFront (gc_list_t* list);
gc_list_t* gc_list_popBack  (gc_list_t* list);

/*
** `gc_list_appendFront` takes an existing list and appends it to the
** front of another list. `gc_list_appendBack` does the same, but to
** the back.
**
** The `dest` list is the one that grows. The `src` list is reinitialized
** in order to maintain sanity.
*/
void gc_list_appendFront (gc_list_t* dest, gc_list_t* src);
void gc_list_appendBack  (gc_list_t* dest, gc_list_t* src);

#endif
