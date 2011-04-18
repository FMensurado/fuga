#include "msg.h"
#include "test.h"

const FugaType FugaMsg_type = {
    .name = "Msg"
};

void FugaMsg_init(void* self)
{
    Fuga_setS(FUGA->Msg, "str",   FUGA_METHOD_STR(FugaMsg_str));
    Fuga_setS(FUGA->Msg, "match", FUGA_METHOD_1(FugaMsg_match_));
}

bool FugaMsg_is_(FugaMsg* self, const char* value) {
    self = Fuga_need(self);
    return Fuga_isMsg(self) &&
           FugaSymbol_is_(self->name, value);
}

FugaMsg* FugaMsg_new_(
    void* self,
    const char* name
) {
    ALWAYS(self); ALWAYS(name);
    return FugaMsg_fromSymbol(FUGA_SYMBOL(name));
}

void FugaMsg_mark(
    void* _self
) {
    FugaMsg* self = _self;
    Fuga_mark_(self, self->name);
}

FugaMsg* FugaMsg_fromSymbol(
    FugaSymbol* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isSymbol(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg toSymbol: expected primitive symbol"
        );

    FugaMsg* result = Fuga_clone_(FUGA->Msg, sizeof(FugaMsg));
    Fuga_type_(result, &FugaMsg_type);
    Fuga_onMark_(result, FugaMsg_mark);
    result->name = self;
    return result;
}

FugaSymbol* FugaMsg_toSymbol(
    FugaMsg* self
) {
    FugaSymbol* name = FugaMsg_name(self);
    FUGA_NEED(name);
    if (!Fuga_isSymbol(name))
        FUGA_RAISE(FUGA->TypeError,
            "Msg toSymbol: expected a symbol msg, not an int msg"
        );
    return name;
}

void* FugaMsg_name(
    FugaMsg* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg name: expected primitive msg"
        );
    return self->name;
}

void* FugaMsg_args(
    FugaMsg* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg args: expected primitive msg"
        );
    return Fuga_slots(self);
}

void* FugaMsg_eval_in_(FugaMsg* self, void* recv, void* scope)
{
    ALWAYS(self);    ALWAYS(recv);    ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg eval: expected primitive msg"
        );

    void* name = FugaMsg_name(self);
    void* args = Fuga_lazy_(FugaMsg_args(self), scope);
    return Fuga_send(recv, name, args);
}

void* FugaMsg_str(void* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    void * name = FugaMsg_name(self);
    FugaString* namestr = Fuga_str(name);
    FUGA_NEED(namestr);
    if (Fuga_isSymbol(name))
        namestr = FugaString_from_(namestr, 1);

    if (Fuga_hasLength_(self, 0)) {
        return namestr;
    } else {
        void* argsstr = Fuga_strSlots(self);
        return FugaString_cat_(namestr, argsstr);
    }
}

void* FugaMsg_match_(FugaMsg* self, void* other) 
{
    FUGA_NEED(self);
    FUGA_NEED(other);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError, "Msg match: self must be a msg");
    if (!Fuga_hasLength_(self, 0)) 
        FUGA_RAISE(FUGA->SyntaxError, "Msg match: msg can't have args");

    void* result = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_set(result, self->name, other));
    return result;
}

