#include "test.h"
#include "obj.h"
#include "int.h"
#include "symbol.h"

void _FugaObj_mark(void* _self)
{
    FugaObj* self = _self;
    Fuga_mark(self);
    FugaGC_mark(self, self->proto);
    FugaGC_mark(self, self->slots);
}

Fuga* _FugaObj_new(FugaRoot *root)
{
    Fuga* self = FugaGC_alloc(root, FUGA_SIZE);
    FugaGC_onMark(self, _FugaObj_mark);
    self->header.root = root;
    self->header.type = &FugaType_Obj;
    self->header.id = ++(root->lastID);
    return self;
}

/**
 * Clone an object, for great justice.
 */
Fuga* Fuga_clone(Fuga* proto)
{
    ALWAYS(proto);
    FugaObj* self = (FugaObj*)_FugaObj_new(proto->header.root);
    self->proto = proto;
    return (Fuga*)self;
}

Fuga* FugaObj_proto(Fuga* _self)
{
    FugaObj* self = (FugaObj*)_self;
    return self->proto;
}

Fuga* FugaObj_has(Fuga* _self, Fuga* name)
{
    FugaObj* self = (FugaObj*)_self;
    if (!self->slots)
        return FUGA->False;
    if (name->header.type == &FugaType_Int)
        return FUGA_BOOL(FugaSlots_hasByIndex(
            self->slots,
            ((FugaInt*)name)->value
        ));
    else // (name->header.type == &FugaType_Symbol)
        return FUGA_BOOL(FugaSlots_hasBySymbol(
            self->slots,
            name
        ));
}

Fuga* FugaObj_doc(Fuga* _self, Fuga* name)
{
    FugaObj* self = (FugaObj*)_self;
    if (!self->slots)
        return NULL;
    FugaSlot *slot;
    if (name->header.type == &FugaType_Int)
        slot = FugaSlots_getByIndex(
            self->slots,
            ((FugaInt*)name)->value
        );
    else // (name->header.type == FugaType_Symbol)
        slot = FugaSlots_getBySymbol(
            self->slots,
            name
        );
    if (slot)
        return slot->doc;
    else
        return NULL;
}

Fuga* FugaObj_get(Fuga* _self, Fuga* name)
{
    FugaObj* self = (FugaObj*)_self;
    if (!self->slots)
        return NULL;
    FugaSlot *slot;
    if (name->header.type == &FugaType_Int)
        slot = FugaSlots_getByIndex(
            self->slots,
            ((FugaInt*)name)->value
        );
    else // (name->header.type == FugaType_Symbol)
        slot = FugaSlots_getBySymbol(
            self->slots,
            name
        );
    if (slot)
        return slot->value;
    else
        return NULL;
}

Fuga* FugaObj_set(Fuga* _self, Fuga* name, Fuga* value)
{
    FugaObj* self = (FugaObj*)_self;
    if (!self->slots)
        self->slots = FugaSlots_new(self);
    
    FugaSlot slot = {.name = NULL, .value = value, .doc = NULL};

    if (!name) {
        result = FugaSlots_append(self->slots, slot);
    } else if (name->header.type == &FugaType_Int) {
        // FIXME: check sign + size of name (the index)
        FugaIndex index = ((FugaInt*)name)->value;
        FugaSlots_setByIndex(self->slots, index, slot);
    } else if (name->header.type == FugaType_Symbol)
        slot->name = name;   
        FugaSlots_setBySymbo(self->slots, name, slot);
    } else {
        FUGA_RAISE
    }
    return FUGA->nil;
}

FugaType FugaType_Obj = {
    .proto = FugaObj_proto,
    .has = FugaObj_has,
    .doc = FugaObj_doc,
    .get = FugaObj_get,
    .set = FugaObj_set,
    .call = NULL // FIXME: implement FugaObj_call
};

