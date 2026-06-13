#ifndef EXEC_H
#define EXEC_H

#include "types.h"

/* ===================== EXECUTE RUNTIME LOOP ===================== */
Value execute_lines(int from, int to, Variable **scope, int scope_count);

/*
 * Panggil fungsi dengan by-reference binding.
 * arg_toks/arg_cnts: token argumen.
 * caller_scope: scope pemanggil untuk resolve argumen.
 */
Value call_func_stmt(Function *fn,
                      Token    *arg_toks[],  int arg_cnts[], int argc,
                      Variable **caller_scope, int caller_scope_count);

#endif /* EXEC_H */