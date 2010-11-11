#ifndef GC_LIST_H
#define GC_LIST_H

#include <stdbool.h>

/**
*** # GC Lists
*** ### FugaGCList
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
***
*** - Fields:
***     - `FugaGCList* next`: the next item in the list, or the first if this
***     is the dummy.
***     - `FugaGCList* prev`: the previous item in the list, or the last if
***     this is the dummy.
**/
typedef struct FugaGCList FugaGCList;
struct FugaGCList {
    FugaGCList* next;
    FugaGCList* prev;
};

/**
*** ## Constructor
*** ### FugaGCList_init
***
*** Initializes (or reinitialize) a list. This involves making the list a
*** list of one (itself). Or you could view it as an empty list. I.e. the
*** dummy points only to itself.
***
*** - Parameters:
***     - `FugaGCList* list`: The list to initialize. It is modified.
*** - Return value: void
*** - See also: `FugaGCList`
**/
void FugaGCList_init (FugaGCList* list);

/**
*** ## Adding Items
*** ### FugaGCList_pushBack
***
*** Place an item at the end of the list.
***
*** - Parameters:
***     - `FugaGCList* list`: pointer to the dummy of the list.
***     - `FugaGCList* item`: the item to be placed at the end.
*** - See also: `FugaGCList_pushFront`
**/
void FugaGCList_pushBack(FugaGCList* list, FugaGCList* item);

/**
*** ### FugaGCList_pushFront
***
*** Place an item at the start of the list.
***
*** - Parameters:
***     - `FugaGCList* list`: pointer to the dummy of the list.
***     - `FugaGCList* item`: the item to be placed at the start.
*** - See also: `FugaGCList_pushBack`
**/
void FugaGCList_pushFront(FugaGCList* list, FugaGCList* item);

/**
*** ## Removing Items
*** ### FugaGCList_unlink
***
*** Remove an item from its surrounding list. The item's links are unchanged,
*** so know that its `next` and `prev` fields are going to be wrong.
***
*** - Parameters:
***     - `FugaGCList* item`: The item to unlink.
*** - Returns: void
*** - See also: `FugaGCList_popFront`, `FugaGCList_popBack`
**/
void FugaGCList_unlink (FugaGCList* item);

/**
*** ### FugaGCList_popFront
***
*** Remove the first item from the list, returning it. Note that the returned
*** item's `next` and `prev` remain unchanged, so they are both wrong 
*** (usually).
***
*** - Parameters:
***     - `FugaGCList* list`: The list from which to remove the first item.
*** - Returns: a pointer to the removed item with `next` and `prev` unchanged.
*** - See Also: `FugaGCList_popBack`, `FugaGCList_unlink`
**/
FugaGCList* FugaGCList_popFront(FugaGCList* list);

/**
*** ### FugaGCList_popBack
***
*** Remove the last item from the list, returning it. Note that the returned
*** item's `next` and `prev` remain unchanged, so they are both wrong 
*** (usually).
***
*** - Parameters:
***     - `FugaGCList* list`: The list from which to remove the lastt item.
*** - Returns: a pointer to the removed item with `next` and `prev` unchanged.
*** - See Also: `FugaGCList_popFront`, `FugaGCList_unlink`
**/
FugaGCList* FugaGCList_popBack(FugaGCList* list);

/**
*** ## Appending
*** ### FugaGCList_appendFront
***
*** Concatenate the `src` list with the `dest` list (in that order), putting
*** the result in `dest`, and leaving the `src` list empty.
***
*** - Parameters:
***     - `FugaGCList* dest`: The destination list, which ends up with `src`
***     concatenated at the beginning.
***     - `FugaGCList* src`: The source list, which ends up empty.
*** - Returns: void
*** - See Also: `FugaGCList_appendBack`
**/
void FugaGCList_appendFront(FugaGCList* dest, FugaGCList* src);

/**
*** ### FugaGCList_appendBack
***
*** Concatenate the `dest` list with the `src` list (in that order), putting
*** the result in `dest`, and leaving the `src` list empty.
***
*** - Parameters:
***     - `FugaGCList* dest`: The destination list, which ends up with `src`
***     concatenated at the end.
***     - `FugaGCList* src`: The source list, which ends up empty.
*** - Returns: void
*** - See Also: `FugaGCList_appendFront`
**/
void FugaGCList_appendBac(FugaGCList* dest, FugaGCList* src);

/**
*** ## Properties
*** ### FugaGCList_empty
***
*** Determine whether the list is empty.
***
*** - Parameters:
***     - `FugaGCList* list`: the list.
*** - Returns: true if the list is empty, false otherwise.
**/
bool FugaGCList_empty(FugaGCList* list);

/**
*** ### FugaGCList_contains
*** 
*** Determine whether an item is a member of the list.
*** 
*** - Parameters:
***     - `FugaGCList* list`: the list to search in.
***     - `void* data`: the item to search for.
*** - Returns: true if the item is in the list, false otherwise.
**/
bool FugaGCList_contains(FugaGCList* list, void* data);

#endif

