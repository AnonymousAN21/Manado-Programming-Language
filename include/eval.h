#ifndef EVAL_H
#define EVAL_H

#include "types.h"

/* ===================== SPLIT ARGUMEN ===================== */
int split_args(Token *tokens, int count,
               Token  *arg_toks[MAX_FUNC_PARAMS],
               int     arg_cnts[MAX_FUNC_PARAMS]);

/* ===================== IF/ELSE BOUNDARY ===================== */
int find_if_boundaries(int if_line, int to, int *else_line, int *selesai_line);

/* ===================== EVALUATOR EKSPRESI ===================== */
Value eval_expr(Token *tokens, int count, Variable **scope, int scope_count);

#endif /* EVAL_H */