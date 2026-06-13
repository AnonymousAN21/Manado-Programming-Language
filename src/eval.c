#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/eval.h"
#include "../include/globals.h"
#include "../include/value.h"
#include "../include/scope.h"
#include "../include/functab.h"
#include "../include/exec.h"

/* ===================== SPLIT ARGUMEN ===================== */
int split_args(Token *tokens, int count,
                      Token  *arg_toks[MAX_FUNC_PARAMS],
                      int     arg_cnts[MAX_FUNC_PARAMS]) {
    int argc = 0, start = 0, pd = 0, bd = 0;
    for (int j = 0; j < count; j++) {
        if (tokens[j].type == TK_LPAREN)   pd++;
        else if (tokens[j].type == TK_RPAREN)  pd--;
        else if (tokens[j].type == TK_LBRACKET) bd++;
        else if (tokens[j].type == TK_RBRACKET) bd--;

        int is_sep  = (tokens[j].type == TK_COMMA && pd == 0 && bd == 0);
        int is_last = (j == count - 1);

        if (is_sep || is_last) {
            int len = j - start + (is_last && !is_sep ? 1 : 0);
            if (len > 0 && argc < MAX_FUNC_PARAMS) {
                arg_toks[argc] = &tokens[start];
                arg_cnts[argc] = len;
                argc++;
            }
            start = j + 1;
        }
    }
    return argc;
}

/* ===================== IF/ELSE BOUNDARY ===================== */
int find_if_boundaries(int if_line, int to, int *else_line, int *selesai_line) {
    int depth = 0;
    *else_line = -1; *selesai_line = -1;
    for (int i = if_line + 1; i <= to; i++) {
        if (g_lines[i].count == 0) continue;
        TokenType t = g_lines[i].tokens[0].type;
        if (t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
        else if (t == TK_JIKA_TIDAK || t == TK_JIKA_TIDAK_JIKA) {
            if (depth == 0 && *else_line == -1) *else_line = i;
        } else if (t == TK_SELESAI) {
            if (depth == 0) { *selesai_line = i; return 1; }
            depth--;
        }
    }
    return 0;
}

/* ===================== EVALUATOR EKSPRESI ===================== */
Value eval_expr(Token *tokens, int count, Variable **scope, int scope_count) {
    if (count == 0) return make_int(0);

    /* masukkan("prompt") sebagai ekspresi */
    if (count >= 3 &&
        tokens[0].type == TK_MASUKKAN &&
        tokens[1].type == TK_LPAREN &&
        tokens[count-1].type == TK_RPAREN) {
        Value prompt = eval_expr(tokens+2, count-3, scope, scope_count);
        print_value(prompt); fflush(stdout);
        char buf[MAX_TOKEN_LEN];
        if (fgets(buf, sizeof(buf), stdin)) {
            trim_newline(buf); char *ep;
            long iv = strtol(buf, &ep, 10);
            if (*ep == '\0') return make_int(iv);
            double dv = strtod(buf, &ep);
            if (*ep == '\0') return make_float(dv);
            return make_string(buf);
        }
        return make_int(0);
    }

    /* semaso("prompt") atau semaso() */
    if (count >= 2 &&
        tokens[0].type == TK_IDENTIFIER &&
        strcmp(tokens[0].value, "semaso") == 0 &&
        tokens[1].type == TK_LPAREN &&
        tokens[count-1].type == TK_RPAREN) {
        /* Cetak prompt kalau ada */
        if (count > 3) {
            Value prompt = eval_expr(tokens+2, count-3, scope, scope_count);
            print_value(prompt);
            fflush(stdout);
        }
        char buf[MAX_TOKEN_LEN];
        if (fgets(buf, sizeof(buf), stdin)) {
            trim_newline(buf);
            char *ep;
            long iv = strtol(buf, &ep, 10);
            if (*ep == '\0') return make_int(iv);
            double dv = strtod(buf, &ep);
            if (*ep == '\0') return make_float(dv);
            return make_string(buf);
        }
        return make_int(0);
    }

    /* Array literal [1, 2, 3] */
    if (count >= 2 && tokens[0].type == TK_LBRACKET && tokens[count-1].type == TK_RBRACKET) {
        Value arr; memset(&arr, 0, sizeof(Value)); arr.type = VAL_ARRAY;
        Token *at[MAX_FUNC_PARAMS]; int ac[MAX_FUNC_PARAMS];
        int argc = split_args(tokens+1, count-2, at, ac);
        for (int i = 0; i < argc && i < MAX_ARRAY_SIZE; i++)
            arr.arr[arr.arr_size++] = val_as_int(eval_expr(at[i], ac[i], scope, scope_count));
        return arr;
    }

   /* Baca arr[idx] */
    if (count >= 4 &&
        tokens[0].type == TK_IDENTIFIER &&
        tokens[1].type == TK_LBRACKET &&
        tokens[count-1].type == TK_RBRACKET) {

        /* FIX: Verifikasi bahwa kurung sikunya memang satu kesatuan! */
        int b_depth = 0;
        int matched = 0;
        for (int i = 1; i < count; i++) {
            if (tokens[i].type == TK_LBRACKET) b_depth++;
            else if (tokens[i].type == TK_RBRACKET) {
                b_depth--;
                if (b_depth == 0 && i == count - 1) matched = 1;
                if (b_depth == 0 && i < count - 1) break; /* Ketemu ']' terlalu cepat */
            }
        }

        if (matched) {
            Variable *v = scope_find(tokens[0].value, scope, scope_count);
            if (v && v->val.type == VAL_ARRAY) {
                Value idx = eval_expr(tokens+2, count-3, scope, scope_count);
                int i = val_as_int(idx);
                if (i >= 0 && i < v->val.arr_size) return make_int(v->val.arr[i]);
            }
            return make_int(0);
        }
    }
    /* Pemanggilan fungsi sebagai ekspresi: panggil nama(args) */
    if (count >= 4 &&
        tokens[0].type == TK_PANGGIL &&
        tokens[1].type == TK_IDENTIFIER &&
        tokens[2].type == TK_LPAREN &&
        tokens[count-1].type == TK_RPAREN) {
        Function *fn = find_func(tokens[1].value);
        if (fn) {
            Token *at[MAX_FUNC_PARAMS]; int ac[MAX_FUNC_PARAMS];
            int argc = split_args(tokens+3, count-4, at, ac);
            /* reuse the existing by-ref call logic */
            Variable *fn_scope[MAX_VARS];
            int fn_scope_count = 0;
            char      saved_names[MAX_FUNC_PARAMS][MAX_TOKEN_LEN];
            Variable *bound_vars[MAX_FUNC_PARAMS];
            int       is_ref[MAX_FUNC_PARAMS];
            int       temp_var_start = g_var_count;
            memset(bound_vars, 0, sizeof(bound_vars));
            memset(is_ref,     0, sizeof(is_ref));
            for (int j = 0; j < fn->param_count && j < argc; j++) {
                Variable *target = NULL;
                if (ac[j] == 1 && at[j][0].type == TK_IDENTIFIER) {
                    target = scope_find(at[j][0].value, scope, scope_count);
                    if (target) {
                        strcpy(saved_names[j], target->name);
                        strcpy(target->name, fn->params[j]);
                        is_ref[j] = 1;
                    }
                }
                if (target == NULL && g_var_count < MAX_VARS) {
                    target = &g_vars[g_var_count++];
                    memset(target, 0, sizeof(Variable));
                    strcpy(target->name, fn->params[j]);
                    target->val = eval_expr(at[j], ac[j], scope, scope_count);
                    is_ref[j] = 0;
                }
                if (target) { bound_vars[j] = target; fn_scope[fn_scope_count++] = target; }
            }
            g_should_return = 0; g_call_depth++;
            Value res = execute_lines(fn->start_line, fn->end_line, fn_scope, fn_scope_count);
            g_call_depth--; g_should_return = 0;
            for (int j = 0; j < fn->param_count && j < argc; j++)
                if (is_ref[j] && bound_vars[j]) strcpy(bound_vars[j]->name, saved_names[j]);
            g_var_count = temp_var_start;
            return res;
        }
    }
    /* Token tunggal */
    if (count == 1) {
        if (tokens[0].type == TK_NUMBER)    return make_int(atol(tokens[0].value));
        if (tokens[0].type == TK_FLOAT_NUM) return make_float(atof(tokens[0].value));
        if (tokens[0].type == TK_STRING)    return make_string(tokens[0].value);
        if (tokens[0].type == TK_CHAR_LIT) {
            Value v = make_string(tokens[0].value);
            return v;
        }
        if (tokens[0].type == TK_IDENTIFIER) {
            Variable *v = scope_find(tokens[0].value, scope, scope_count);
            if (v) return v->val;
            return make_int(0);
        }
        return make_int(0);
    }

    /* Ekspresi dalam kurung */
    if (tokens[0].type == TK_LPAREN && tokens[count-1].type == TK_RPAREN) {
        /* Verifikasi memang satu pasang kurung */
        int depth = 0; int matched = 0;
        for (int i = 0; i < count; i++) {
            if (tokens[i].type == TK_LPAREN) depth++;
            else if (tokens[i].type == TK_RPAREN) { depth--; if (depth == 0 && i == count-1) matched = 1; }
        }
        if (matched) return eval_expr(tokens+1, count-2, scope, scope_count);
    }

    /* Minus unary */
    if (count >= 2 && tokens[0].type == TK_MINUS) {
        Value v = eval_expr(tokens+1, count-1, scope, scope_count);
        return (v.type == VAL_FLOAT) ? make_float(-v.fval) : make_int(-val_as_int(v));
    }

    /*
     * Cari operator dengan precedence paling rendah (kiri ke kanan / right-associative
     * untuk yang sama precedence-nya → pakai <=).
     * Precedence: ato(1) < deng(2) < perbandingan(3) < tambah/kurang(4) < kali/bagi/modulo(5)
     */
    int pd = 0, bd = 0, op_pos = -1, op_prec = 999;
    TokenType op_type = TK_UNKNOWN;

    for (int i = 0; i < count; i++) {
        if (tokens[i].type == TK_LPAREN)   { pd++; continue; }
        if (tokens[i].type == TK_RPAREN)   { pd--; continue; }
        if (tokens[i].type == TK_LBRACKET) { bd++; continue; }
        if (tokens[i].type == TK_RBRACKET) { bd--; continue; }
        if (pd > 0 || bd > 0) continue;

        int prec = 999;
        switch (tokens[i].type) {
            case TK_ATAU:             prec = 1; break;
            case TK_DAN:              prec = 2; break;
            case TK_SAMA_DENGAN:
            case TK_TIDAK_SAMA_DENGAN:
            case TK_LEBIH_BESAR:
            case TK_LEBIH_KECIL:
            case TK_LEBIH_BESAR_SAMA:
            case TK_LEBIH_KECIL_SAMA: prec = 3; break;
            case TK_PLUS:
            case TK_MINUS:            prec = 4; break;
            case TK_KALI:
            case TK_BAGI:
            case TK_MODULO:           prec = 5; break;
            default: break;
        }
        if (prec <= op_prec) { op_prec = prec; op_pos = i; op_type = tokens[i].type; }
    }

    if (op_pos > 0 && op_pos < count-1) {
        Value L = eval_expr(tokens,        op_pos,           scope, scope_count);
        Value R = eval_expr(tokens+op_pos+1, count-op_pos-1, scope, scope_count);
        int   use_float = (L.type == VAL_FLOAT || R.type == VAL_FLOAT);

        switch (op_type) {
            case TK_PLUS:
                if (L.type == VAL_STRING || R.type == VAL_STRING) {
                    char ls[MAX_TOKEN_LEN], rs[MAX_TOKEN_LEN], buf[MAX_TOKEN_LEN*2];
                    if (L.type == VAL_STRING) strcpy(ls, L.sval); else snprintf(ls, sizeof(ls), "%ld", L.ival);
                    if (R.type == VAL_STRING) strcpy(rs, R.sval); else snprintf(rs, sizeof(rs), "%ld", R.ival);
                    snprintf(buf, sizeof(buf), "%s%s", ls, rs);
                    return make_string(buf);
                }
                return use_float ? make_float(val_as_float(L)+val_as_float(R)) : make_int(val_as_int(L)+val_as_int(R));
            case TK_MINUS:
                return use_float ? make_float(val_as_float(L)-val_as_float(R)) : make_int(val_as_int(L)-val_as_int(R));
            case TK_KALI:
                return use_float ? make_float(val_as_float(L)*val_as_float(R)) : make_int(val_as_int(L)*val_as_int(R));
            case TK_BAGI: {
                double d = val_as_float(R);
                if (d == 0) { fprintf(stderr,"[Error] Bagi dengan nol!\n"); return make_int(0); }
                return use_float ? make_float(val_as_float(L)/d) : make_int(val_as_int(L)/val_as_int(R));
            }
            case TK_MODULO: {
                long r = val_as_int(R);
                if (r == 0) { fprintf(stderr,"[Error] Modulo dengan nol!\n"); return make_int(0); }
                return make_int(val_as_int(L) % r);
            }
            case TK_SAMA_DENGAN:
                if (L.type==VAL_STRING && R.type==VAL_STRING) return make_int(strcmp(L.sval,R.sval)==0);
                return use_float ? make_int(val_as_float(L)==val_as_float(R)) : make_int(val_as_int(L)==val_as_int(R));
            case TK_TIDAK_SAMA_DENGAN:
                if (L.type==VAL_STRING && R.type==VAL_STRING) return make_int(strcmp(L.sval,R.sval)!=0);
                return use_float ? make_int(val_as_float(L)!=val_as_float(R)) : make_int(val_as_int(L)!=val_as_int(R));
            case TK_LEBIH_BESAR:
                return use_float ? make_int(val_as_float(L)>val_as_float(R)) : make_int(val_as_int(L)>val_as_int(R));
            case TK_LEBIH_KECIL:
                return use_float ? make_int(val_as_float(L)<val_as_float(R)) : make_int(val_as_int(L)<val_as_int(R));
            case TK_LEBIH_BESAR_SAMA:
                return use_float ? make_int(val_as_float(L)>=val_as_float(R)) : make_int(val_as_int(L)>=val_as_int(R));
            case TK_LEBIH_KECIL_SAMA:
                return use_float ? make_int(val_as_float(L)<=val_as_float(R)) : make_int(val_as_int(L)<=val_as_int(R));
            case TK_DAN: return make_int(val_as_int(L) && val_as_int(R));
            case TK_ATAU: return make_int(val_as_int(L) || val_as_int(R));
            default: break;
        }
    }

    return make_int(0);
}