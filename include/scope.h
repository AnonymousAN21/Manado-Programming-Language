#ifndef SCOPE_H
#define SCOPE_H

#include "types.h"

/* ===================== SCOPE: CARI & BUAT VARIABEL ===================== */
Variable* scope_find(const char *name, Variable **scope, int scope_count);
Variable* scope_create(const char *name, Variable **scope, int *scope_count);

#endif /* SCOPE_H */