#ifndef GLOBALS_H
#define GLOBALS_H

#include "config.h"
#include "types.h"

/* ===================== GLOBAL STATE ===================== */
extern CodeLine  g_lines[MAX_LINES];
extern int       g_line_count;

/* Pool variabel global */
extern Variable  g_vars[MAX_VARS];
extern int       g_var_count;

extern Function  g_funcs[MAX_FUNCS];
extern int       g_func_count;

extern Value     g_return_val;
extern int       g_should_return;
extern int       g_should_break;
extern int       g_call_depth;  /* cegah reclaim slot saat rekursi */

/* ---- TAMBAHKAN DI SINI ---- */
extern Variable* g_global_scope[MAX_VARS];
extern int       g_global_count;
/* --------------------------- */

#endif /* GLOBALS_H */