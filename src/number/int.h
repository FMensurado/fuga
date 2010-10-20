/*
** # number/int: Unbounded-size integers.
**
** Header file: number/int.h
** Source file: number/int.c
** Prefix: number_int
**
** This module implements big integers, AKA unbounded-size integers. 
*/

#ifndef NUMBER_INT_H
#define NUMBER_INT_H

/*
** `number_int_t` represents an unbounded-size integer.
*/
typedef struct _number_int_t *number_int_t;

/*
** Oneway dependencies: gc/gc
** Mutual dependencies: string/string, number/ratio
*/
#include "../gc/gc.h"
#include "../string/string.h"
#include "ratio.h"

/*
** ## Constructors
**
*/
number_int_t number_int_new    (gc_t, long long x);
number_int_t number_int_cparse (gc_t, const char* str, unsigned int base);
number_int_t number_int_parse  (gc_t, string_t str, unsigned int base);

/*
** ## Basic Arithmetic
**
*/
number_int_t number_int_add  (gc_t, number_int_t left, number_int_t right);
number_int_t number_int_sub  (gc_t, number_int_t left, number_int_t right);
number_int_t number_int_mul  (gc_t, number_int_t left, number_int_t right);
number_int_t number_int_fdiv (gc_t, number_int_t left, number_int_t right);
number_int_t number_int_mod  (gc_t, number_int_t left, number_int_t right);

/*
** ## Rational Division
**
**
#include "ratio.h"
number_ratio_t number_int_div(gc_t, number_int_t left, number_int_t right);
number_ratio_t number_int_reciprocal  (gc_t, number_int_t value);
number_ratio_t number_int_ratio       (gc_t, number_int_t value);
*/

/*
** ## Formatting
*/

char*    number_int_cstr (gc_t, number_int_t);
string_t number_int_str  (gc_t, number_int_t);

#endif

