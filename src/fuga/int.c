
fuga_t fuga_int_init(fuga_t self) {
    ALWAYS(self);
}

fuga_t fuga_int(fuga_t self, long long value) {
    ALWAYS(self);
    
    fuga_t Object = self->Object;
    fuga_t Int;
    
    if (Fuga_has_(Object, "Int")) {
        Int = fuga_get(Object, "Int");
    } else {
        Int = fuga_clone(Object);
        fuga_set(Int, "+", fuga_method2(Object, fuga_int_add));
        fuga_set(Object, "Int", Int);
    }

    result = fuga_alloc(Int, FUGA_TYPE_INT, sizeof(long long));
    
    FUGA_DATA(long long, result->data) = value;

    return result;
}

fuga_t fuga_int_add(fuga_t self, fuga_t other) {
    ALWAYS(self);
    ALWAYS(other);
    if (self->type != FUGA_TYPE_INT)
        return fuga_TypeError(self, "Int + requires a primitive Int.");
    if (other->type == FUGA_TYPE_INT)
        return fuga_int(self, FUGA_DATA(long long, self)
                            + FUGA_DATA(long long, other));
    if (other->type == FUGA_TYPE_REAL)
        return fuga_real(self, FUGA_DATA(long long, self)
                             + FUGA_DATA(double, other));
    return fuga_TypeError(self, "Can't add non-number to number.")
}

fuga_t fuga_int_sub(fuga_t self, fuga_t other) {
    ALWAYS(self);
    ALWAYS(other);
    if (self->type != FUGA_TYPE_INT)
        return fuga_TypeError(self, "Int - requires a primitive Int.");
    if (other->type == FUGA_TYPE_INT)
        return fuga_int(self, FUGA_DATA(long long, self)
                            - FUGA_DATA(long long, other));
    if (other->type == FUGA_TYPE_REAL)
        return fuga_real(self, FUGA_DATA(long long, self)
                             - FUGA_DATA(double, other));
    return fuga_TypeError(self, "Can't subtract non-number from number.")
}

fuga_t fuga_int_mul(fuga_t self, fuga_t other) {
    ALWAYS(self);
    ALWAYS(other);
    if (self->type != FUGA_TYPE_INT)
        return fuga_TypeError(self, "Int * requires a primitive Int.");
    if (other->type == FUGA_TYPE_INT)
        return fuga_int(self, FUGA_DATA(long long, self)
                            * FUGA_DATA(long long, other));
    if (other->type == FUGA_TYPE_REAL)
        return fuga_real(self, FUGA_DATA(long long, self)
                             * FUGA_DATA(double, other));
    return fuga_TypeError(self, "Can't multiply number with non-number.")
}

fuga_t fuga_int_div(fuga_t self, fuga_t other) {
    ALWAYS(self);
    ALWAYS(other);
    if (self->type != FUGA_TYPE_INT)
        return fuga_TypeError(self, "Int / requires a primitive Int.");
    if (other->type == FUGA_TYPE_INT)
        return fuga_real(self, (double)FUGA_DATA(long long, self)
                             / (double)FUGA_DATA(long long, other));
    if (other->type == FUGA_TYPE_REAL)
        return fuga_real(self, (double)FUGA_DATA(long long, self)
                                     / FUGA_DATA(double, other));
    return fuga_TypeError(self, "Can't divide number and non-number.")
}


