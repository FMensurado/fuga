#ifndef FUGA_H
#define FUGA_H

typedef unsigned long fuga_type_t;
typedef struct _fuga_t* fuga_t;

struct _fuga_t {
    fuga_t Object;
    fuga_t proto;
    char data[];
};

/**
 * Initialize the Fuga environment, specifying which parts of the
 * environment to allow.
 *
 * @param flags -- initialization flags. Choose from
 *      FUGA_INIT_NONE -- initialize nothing (use with caution).
 *      FUGA_INIT_ALL  -- initialize everything (the default).
 *      FUGA_INIT_CORE -- initialize the core modules (the bare minimum).
 *      FUGA_INIT_STR  -- initialize the String module (part of core).
 *      FUGA_INIT_NUM  -- initialize the Number module (part of core).
 *      FUGA_INIT_IO   -- initialize the IO module (not part of core).
 *  Use bitwise or (|) to combine multiple flags.
 * @return Object -- the object from which all other objects are derived.
 */
fuga_t fuga_init(unsigned long flags);

#define FUGA_INIT_NONE ((unsigned long)0x00000000)
#define FUGA_INIT_ALL  (~FUGA_INIT_NONE)
#define FUGA_INIT_CORE (FUGA_INT_STR | FUGA_INT_NUM)
#define FUGA_INIT_STR  ((unsigned long)0x00000001)
#define FUGA_INIT_NUM  ((unsigned long)0x00000002)
#define FUGA_INIT_IO   ((unsigned long)0x00000004)

/**
 * Clean up after we're done with the Fuga environment.
 *
 * @params fuga_t self -- any object from the fuga environment to be 
 *     cleared.
 */
void fuga_free(fuga_t self);


/**
 * The fine foo bar baz of the earth. it'll eat y'all up. mm. Fun!!! Ok no
 * I must be very tired.
 *
 * ...
 */
fuga_t fuga_clone(fuga_t proto);

/**
 *
 */
fuga_t fuga_alloc(fuga_t proto, fuga_type_t type, fuga_int);

#define FUGA_TYPE_NONE   0x0000 // Don't use this when allocating!
#define FUGA_TYPE_OBJECT 0x0001 // For Object only.

#define FUGA_TYPE_CORE   0x0100 // Core types are between 0x100 and 0x1FF 
#define FUGA_TYPE_INT    0x0101 // The type for primitive ints.
#define FUGA_TYPE_REAL   0x0102 // The type for primitive reals.
#define FUGA_TYPE_STR    0x0103 // The type for primitive strings.
#define FUGA_TYPE_METHOD 0x0104 // The type for primitive methods.

#define FUGA_TYPE_USER   0x8000 // Extension types are greater than 0x8000

#ifndef BOOL
# define BOOL char
# ifndef TRUE
#  define TRUE  1
#  define FALSE 0
# endif
#endif

/**
 * Return the number of slots in an object.
 */
size_t fuga_size(fuga_t fuga); 

/**
 *
 */
BOOL fuga_has(fuga_t fuga, const char* name);
BOOL fuga_rawHas(fuga_t fuga, const char* name);

/**
 *
 */
fuga_t fuga_get(fuga_t fuga, const char* name);
fuga_t fuga_rawGet(fuga_t fuga, const char* name);

/**
 *
 */
void fuga_set(fuga_t fuga, const char* name, fuga_t value);

#endif // #ifndef FUGA_H

