#pragma once
#ifndef FUGA_OBJ_H
#define FUGA_OBJ_H

#include "fuga.h"
#include "slots.h"

extern FugaType FugaType_Obj;

Fuga* _FugaObj_new(FugaRoot*);
Fuga*  FugaObj_clone(Fuga*);
Fuga*  FugaObj_has(Fuga*, Fuga*);
Fuga*  FugaObj_doc(Fuga*, Fuga*);
Fuga*  FugaObj_get(Fuga*, Fuga*);
Fuga*  FugaObj_set(Fuga*, Fuga*, Fuga*);

#endif

