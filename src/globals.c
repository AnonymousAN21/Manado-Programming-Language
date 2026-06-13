#include "../include/globals.h"

/* ===================== GLOBAL STATE ===================== */
CodeLine  g_lines[MAX_LINES];
int       g_line_count = 0;

/* Pool variabel global */
Variable  g_vars[MAX_VARS];
int       g_var_count = 0;

Function  g_funcs[MAX_FUNCS];
int       g_func_count = 0;

Value     g_return_val;
int       g_should_return = 0;
int       g_should_break  = 0;
int       g_call_depth    = 0;  /* cegah reclaim slot saat rekursi */

/* ---- TAMBAHKAN DI SINI ---- */
Variable* g_global_scope[MAX_VARS];
int       g_global_count = 0;
/* --------------------------- */