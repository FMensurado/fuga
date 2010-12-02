#include "fuga.h"
#include "test.h"
#include "gc.h"

#define INITIAL_CAPACITY 4

Fuga* Fuga_setf(Fuga* self, Fuga* name, Fuga* value) {
    ALWAYS(self);
    ALWAYS(value);
    
    // make sure self->slots is big enough
    if (!self->slots) {
        self->slots = malloc(
            sizeof(FugaSlots) + INITIAL_CAPACITY * sizeof(FugaSlot)
        );
        self->slots->length = 0;
        self->slots->capacity = INITIAL_CAPACITY;
    } else if (self->slots->length == self->slots->capacity) {
        self->slots = realloc(
            self->slots,
            sizeof(FugaSlots) + 2 * self->slots->capacity * sizeof(FugaSlot)
        );
        self->slots->capacity *= 2;
    }
    
    // store value in the appropriate place
    FugaSlot* slot = self->slots->slot + self->slots->length;
    
    // store the name+value correctly
    if ((!name) || (name->type == FUGA_TYPE_NULL)) {
        slot->name = NULL;
    } else if (name->type == FUGA_TYPE_SYMBOL) {
        slot->name = name;
    } else if (name->type == FUGA_TYPE_STRING) {
        slot->name = Fuga_mkSymbol(self, name->data);
    } else {
        return Fuga_raise(Fuga_mkTypeError(self,
            "`name` argument for `setSlot` can only be null, an Int,"
            " a Symbol, or a String."
        ));
    }
    self->slots->length++;
    slot->value = value;
    return NULL;
}
