// agenda.h -- v1.0.1 -- Public Domain -- https://www.github.com/3le4ms1/agenda.h

// A GENeric Dynamic Array implementation in C

// This is a header only library, stb style, that provides an implementation of
// generic dynamic array in C.  This is achieved by heavily relying on type
// punning to `void*', and hiding the metadata before the first element of the
// array itself.

// This project takes inspiration from:
// - https://github.com/tsoding/nob.h
// - https://github.com/gallexi/darray

// Quick example:
#if 0
#include <stdio.h>
#define AGENDA_IMPLEMENTATION
#define AGENDA_HARDEN_WITH_MAGIC
#include <agenda.h>

int main(void) {
    int *items = agenda_init(int);

    for (int i = 0; i < 100; i++) {
        int x = i * 2;
        agenda_push_back(&items, &x);
    }

    for (int i = 0; i < agenda_items_count(items); i++) {
        printf("%d\n", agenda[i]);
    }

    agenda_deinit(&items);
    return 0;
}
#endif // 0

#ifndef AGENDA_H_
#define AGENDA_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Defines the number of bytes reserved by `agenda_init()' when creating a new
// dynamic array
#ifndef AGENDA_INITIAL_CAPACITY_IN_BYTES
#define AGENDA_INITIAL_CAPACITY_IN_BYTES 256
#endif // AGENDA_INITIAL_CAPACITY_IN_BYTES

// Defines the scale factor used when increasing the capacity of the dynamic
// array on realloc.  Usually this value is 1.5f or 2
#ifndef AGENDA_SCALE_FACTOR
#define AGENDA_SCALE_FACTOR 2
#endif // AGENDA_SCALE_FACTOR

// This macro goes before function declaration and implementation.  Useful do
// `#define AGENDADEF static inline'.  This will force the compilter to optimize
// out the unused functions if the source code is a single file.
#ifndef AGENDADEF
#define AGENDADEF
#endif // AGENDADEF

// These macros can be used to define custom alloc, realloc and free functions
// when needed
#ifndef AGENDA_ALLOC
#define AGENDA_ALLOC alloc
#endif // AGENDA_ALLOC

#ifndef AGENDA_FREE
#define AGENDA_FREE free
#endif // AGENDA_FREE

#ifndef AGENDA_REALLOC
#define AGENDA_REALLOC realloc
#endif // AGENDA_REALLOC

#ifndef AGENDA_MEMCPY
#define AGENDA_MEMCPY memcpy
#endif // AGENDA_MEMCPY

// When `AGENDA_HARDEN_WITH_MAGIC' is enables, it hardens the implementation of
// the dynamic arrays and all of the functions that interact with them, adding a
// magic number field that will be checked on each operation.  This is useful to
// prevent operation on unsafe memory
#ifdef AGENDA_HARDEN_WITH_MAGIC
#    ifndef AGENDA__HARDEN_WITH_MAGIC_GUARD_
#        define AGENDA__HARDEN_WITH_MAGIC_GUARD_
#        define AGENDA_MAGIC_NUMBER 0x4144
#    endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_
#endif // AGENDA_HARDEN_WITH_MAGIC

// The creation of the dynamic array, via the `agenda_init()` function, returns the
// pointer `items` (as show below), and this lets the user access the items the
// allocated array just as they were in a static array, by the subscript operator
// `[]`.  The necessary metadata is stored inside the `Agenda_Header`, hidden
// behind the first element of the array, like this:

// +---------------+---------+---------+---------+
// | Agenda_Header |   [0]   |   [1]   |   ...   |
// +---------------+---------+---------+---------+
//                 ^
//                 items

typedef struct {
    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    uint16_t magic;
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    // All the members of the header measure the memory in bytes
                      // Example:
                      //  . = 1 byte
    size_t item_size; //  ..            2 bytes item_size
    size_t size;      // |....        | 2 items inserted of item_size 2
    size_t capacity;  // |............|
                      //  ^ ^ ^ ^ ^ ^ = 6 item slots available before realloc
} Agenda_Header;

typedef void* Agenda;
#define agenda(T) T*

AGENDADEF Agenda agenda_init_(size_t item_size);
AGENDADEF bool agenda_deinit_(Agenda *items);
AGENDADEF bool agenda_push_back_(Agenda *items, void *item);
AGENDADEF bool agenda_push_back_array_(Agenda *items, void *arr, size_t arr_len);
AGENDADEF bool agenda_pop_back_(Agenda *items);
AGENDADEF bool agenda_shrink_to_fit_(Agenda *items);
AGENDADEF Agenda_Header *agenda_get_header_(Agenda *items);
AGENDADEF size_t agenda_items_count_(Agenda *items);
AGENDADEF bool agenda_reset_(Agenda *items);

#define agenda_init(T) agenda_init_(sizeof(T))

#define agenda_deinit(items) agenda_deinit_((Agenda *)items)
#define agenda_push_back(items, item) agenda_push_back_((Agenda *)items, (void *)item)
#define agenda_push_back_array(items, arr, arr_len) agenda_push_back_array_((Agenda *)items, (void *)arr, arr_len)
#define agenda_pop_back(items) agenda_pop_back_((Agenda *)items)
#define agenda_shrink_to_fit(items) agenda_shrink_to_fit_((Agenda *)items)
#define agenda_get_header(items) agenda_get_header_((Agenda *)items)
#define agenda_items_count(items) agenda_items_count_((Agenda *)items)
#define agenda_reset(items) agenda_reset_((Agenda *)items)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AGENDA_H_

#ifdef AGENDA_IMPLEMENTATION
#    ifndef AGENDA__IMPLEMENTATION_GUARD_
#    define AGENDA__IMPLEMENTATION_GUARD_

// Auxiliary functions and macros
#define agenda__get_items(da) ((Agenda_Header*)da + 1)

// Performs a reallocation whenever the required capacity for the next insertion
// exceeds the allocated capacity.  The next capacity is then calculated by
// multiplying by `AGENDA_SCALE_FACTOR' the existing capacity, several times
// when needed
AGENDADEF bool agenda__realloc_many(Agenda *items, size_t number_of_new_items) {
    Agenda_Header *da = agenda_get_header(items);
    size_t required_capacity_for_alloc = da->size + (da->item_size * number_of_new_items);
    if (required_capacity_for_alloc > da->capacity) {
        while (required_capacity_for_alloc > da->capacity) {
            da->capacity = (da->capacity > 0 ? da->capacity : 1) * AGENDA_SCALE_FACTOR;
        }
        Agenda_Header *new_da = AGENDA_REALLOC(da, sizeof(Agenda_Header) + da->capacity);
        if (new_da == NULL) { return false; }
        da = new_da;
        *items = agenda__get_items(da);
    }
    return true;
}

// Useful functions

// Given the &items, this function returns the Agenda_Header pointer.  This can
// be used to check for some particular value of the header fields.
// WARNING: Watch out for dangerous modifications, they can result in UB
AGENDADEF Agenda_Header *agenda_get_header_(Agenda *items) {
    if (items == NULL || *items == NULL) { return NULL; }

    Agenda_Header *da = ((Agenda_Header*)(*items)) - 1;

    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (da->magic != AGENDA_MAGIC_NUMBER) {
        da = NULL;
    }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    return da;
}

// Returns the number of items currently held in the dynamic array.  To get the
// index of the last element inserted you can just substract 1, like this:
//        int items_last_index = agenda_items_count(&items) - 1;
AGENDADEF size_t agenda_items_count_(Agenda *items) {
    if (items == NULL || *items == NULL) { return 0; }

    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (agenda_get_header(items)->magic != AGENDA_MAGIC_NUMBER) {
        return 0;
    }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_
    return ((agenda_get_header(items)->size)/(agenda_get_header(items)->item_size));
}

// This function resets the dynamic array, making it just as afters it was
// allocated, minus capacity already allocated.  It does not zero initialize the
// memory already allocated
AGENDADEF bool agenda_reset_(Agenda *items) {
    if (items != NULL || *items == NULL) { return false; }

    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (agenda_get_header(items)->magic != AGENDA_MAGIC_NUMBER) {
        return false;
    }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_
    agenda_get_header(items)->size = 0;
    return true;
}

// Creates a new dynamic array on the heap, and returns a pointer to the first
// element of the array, right after the `Agenda_Header'
AGENDADEF Agenda agenda_init_(size_t item_size) {
    if (item_size == 0) { return NULL; }
    Agenda_Header *da = malloc(sizeof(Agenda_Header) + AGENDA_INITIAL_CAPACITY_IN_BYTES);
    if (da == NULL) { return NULL; }

    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    da->magic = AGENDA_MAGIC_NUMBER;
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    da->item_size = item_size;
    da->size = 0;
    da->capacity = AGENDA_INITIAL_CAPACITY_IN_BYTES;
    return agenda__get_items(da);
}

// Deallocates the given dynamic array
AGENDADEF bool agenda_deinit_(Agenda *items) {
    if (*items != NULL) {
        Agenda_Header *da = agenda_get_header(items);

        #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
        if (da->magic != AGENDA_MAGIC_NUMBER) { return false; }
        #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

        AGENDA_FREE(da);
        *items = NULL;
        return true;
    }
    return false;
}

// Makes available a new position on the back of the dynamic array, and copies
// the value pointed by `item'
AGENDADEF bool agenda_push_back_(Agenda *items, void *item) {
    if (*items == NULL) return false;

    Agenda_Header *da;
    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    da = agenda_get_header(items);
    if (da->magic != AGENDA_MAGIC_NUMBER) { return false; }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    agenda__realloc_many(items, 1);
    da = agenda_get_header(items); // Required second read in case of realloc,
                                   // on linux segfaults
    AGENDA_MEMCPY((void*)&(((unsigned char *)(*items))[da->size]), item, da->item_size);
    da->size += da->item_size;
    return true;
}

// Useful when in need of pushing an array into the dynamic array.  The element
// pushed by the function are copied, not referenced
AGENDADEF bool agenda_push_back_array_(Agenda *items, void *arr, size_t arr_len) {
    if (*items == NULL || arr_len == 0) { return false; }

    Agenda_Header *da = agenda_get_header(items);
    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (da->magic != AGENDA_MAGIC_NUMBER) { return false; }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    agenda__realloc_many(items, arr_len);
    da = agenda_get_header(items);
    AGENDA_MEMCPY((void*)&(((unsigned char *)(*items))[da->size]), arr, da->item_size * arr_len);
    da->size += da->item_size * arr_len;
    return true;
}

AGENDADEF bool agenda_pop_back_(Agenda *items) {
    if (items == NULL || *items == NULL) { return false; }

    Agenda_Header *da = agenda_get_header(items);

    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (da->magic != AGENDA_MAGIC_NUMBER) { return false; }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    if (da->size >= da->item_size) {
        da->size -= da->item_size;
        return true;
    }
    return false;
}

// This function can be used to modify the allocated space of the dynamic array.
// It is useful in environment where the memory is limited.  The performed
// actions are, in order:
// - It allocates a new buffer with the size of only the element present in the
//   dynamic array
// - Copies all the elements inside the new buffer
// - Deallocates the old buffer
AGENDADEF bool agenda_shrink_to_fit_(Agenda *items) {
    if (items == NULL || *items == NULL) { return false; }

    Agenda_Header *da = agenda_get_header(items);
    #ifdef AGENDA__HARDEN_WITH_MAGIC_GUARD_
    if (da->magic != AGENDA_MAGIC_NUMBER) { return false; }
    #endif // AGENDA__HARDEN_WITH_MAGIC_GUARD_

    Agenda_Header *new_da = AGENDA_REALLOC(da, sizeof(Agenda_Header) + da->size);
    if (new_da == NULL) { return false; }
    new_da->capacity = new_da->size;
    *items = agenda__get_items(new_da);
    return true;
}

#    endif // AGENDA__IMPLEMENTATION_GUARD_
#endif // AGENDA_IMPLEMENTATION

#ifdef AGENDA_SHORT_PREFIX
#    ifndef AGENDA__SHORT_PREFIX_GUARD_
#    define AGENDA__SHORT_PREFIX_GUARD_
#        define da                 agenda
#        define da_init            agenda_init
#        define da_deinit          agenda_deinit
#        define da_get_header      agenda_get_header
#        define da_items_count     agenda_items_count
#        define da_push_back       agenda_push_back
#        define da_push_back_array agenda_push_back_array
#        define da_pop_back        agenda_pop_back
#        define da_reset           agenda_reset
#        define da_shrink_to_fit   agenda_shrink_to_fit
#    endif // AGENDA__SHORT_PREFIX_GUARD_
#endif // AGENDA_SHORT_NAMES

// Future plans:
// - agenda_reserve_back(items, number_of_items)
// - Agenda submodules:
//    - stack
//    - string builder
//        - #define agenda_sb_push_back_str(da, item)

// Revision history:
// 1.0.1 (2026-01-05) adding `AGENDA__HARDEN_WITH_MAGIC_GUARD_' for better flag handling
// 1.0.0 (2025-12-27) first release

// License:

// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// For more information, please refer to <https://unlicense.org>

// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
