#ifndef GC_LIST_H
#define GC_LIST_H

#include <stdbool.h>

/**
*** # GC Lists
*** 
*** `FugaGCList` is a doubly linked list. You'll notice that there are
*** no values other than the links forward and backward. This is because,
*** for the most part, the `FugaGCList*` is assumed to be the pointer to
*** the object itself.
***
*** We actually use doubly linked lists with a dummy. This makes
*** programming the list significantly less troublesome, but it brings the
*** onus of keeping track of the dummy on the user. In this documentation,
*** we always refer to the dummy of the list as a "list", and to any other
*** piece of the list as an "item", despite both being (pointers to) the
*** same structure.
**/
typedef struct FugaGCList FugaGCList;
struct FugaGCList {
    FugaGCList *next;
    FugaGCList *prev;
};

/**
*** `FugaGCList_init` initializes (or reinitializes) a list. This involves
*** making the list a list of one. I.e., the dummy points only to itself.
**/
void FugaGCList_init (FugaGCList* list);

/**
*** `FugaGCList_pushBack` places an item at the back of the list. Likewise,
*** `FugaGCList_pushFront` places an item at the front of the list.
**/
void FugaGCList_pushBack  (FugaGCList* list, FugaGCList* item);
void FugaGCList_pushFront (FugaGCList* list, FugaGCList* item);

/**
*** `FugaGCList_unlink` removes an item from its surrounding list. The item's
*** links are unchanged, so know that `item->prev` and `item->next` are
*** consequently wrong.
**/
void FugaGCList_unlink (FugaGCList* item);

/**
*** `FugaGCList_popFront` removes an item from the front of the list, and
*** returns it. `FugaGCList_popBack` does the same, but to the back of the
*** list instead.
***
*** The returned item's links are not modified, so `item->prev` and
*** `item->next` are both wrong.
**/
FugaGCList* FugaGCList_popFront (FugaGCList* list);
FugaGCList* FugaGCList_popBack  (FugaGCList* list);

/**
*** `FugaGCList_appendFront` takes an existing list and appends it to the
*** front of another list. `FugaGCList_appendBack` does the same, but to
*** the back.
***
*** The `dest` list is the one that grows. The `src` list is reinitialized
*** in order to maintain sanity.
**/
void FugaGCList_appendFront (FugaGCList* dest, FugaGCList* src);
void FugaGCList_appendBack  (FugaGCList* dest, FugaGCList* src);


/**
*** ## Properties
***
*** `FugaGCList_empty` determines whether a list is empty.
*** returns TRUE (non-0) if it is, and FALSE(0) otherwise
***
*** `FugaGCList_contains` determines whether data is a member of the list.
*** returns TRUE (non-0) if it is, and FALSE (0) otherwise.
**/
bool FugaGCList_empty   (FugaGCList* list);
bool FugaGCList_contains(FugaGCList* list, void* data);

#endif

