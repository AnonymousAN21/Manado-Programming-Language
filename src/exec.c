#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/exec.h"
#include "../include/globals.h"
#include "../include/value.h"
#include "../include/scope.h"
#include "../include/functab.h"
#include "../include/eval.h"

/* ===================== EXECUTE RUNTIME LOOP ===================== */

/*
 * Panggil fungsi dengan by-reference binding.
 * arg_toks/arg_cnts: token argumen.
 * caller_scope: scope pemanggil untuk resolve argumen.
 */
Value call_func_stmt(Function *fn,
                             Token    *arg_toks[],  int arg_cnts[], int argc,
                             Variable **caller_scope, int caller_scope_count) {
    Variable *fn_scope[MAX_VARS];
    int fn_scope_count = 0;

    /* Simpan nama asli untuk di-restore setelah fungsi selesai */
    char      saved_names[MAX_FUNC_PARAMS][MAX_TOKEN_LEN];
    Variable *bound_vars[MAX_FUNC_PARAMS];
    int       is_ref[MAX_FUNC_PARAMS]; /* 1 = by-ref (nama harus direstore), 0 = slot sementara */
    int       temp_var_start = g_var_count;

    memset(bound_vars, 0, sizeof(bound_vars));
    memset(is_ref,     0, sizeof(is_ref));

    for (int j = 0; j < fn->param_count && j < argc; j++) {
        Variable *target = NULL;

        /* Argumen = identifier tunggal → by-reference ke slot asli */
        if (arg_cnts[j] == 1 && arg_toks[j][0].type == TK_IDENTIFIER) {
            target = scope_find(arg_toks[j][0].value, caller_scope, caller_scope_count);
            if (target) {
                strcpy(saved_names[j], target->name);
                strcpy(target->name, fn->params[j]);
                is_ref[j] = 1;
            }
        }

        /* Argumen = ekspresi/literal → slot sementara (by-value) */
        if (target == NULL && g_var_count < MAX_VARS) {
            target = &g_vars[g_var_count++];
            memset(target, 0, sizeof(Variable));
            strcpy(target->name, fn->params[j]);
            target->val = eval_expr(arg_toks[j], arg_cnts[j], caller_scope, caller_scope_count);
            is_ref[j] = 0;
        }

        if (target) {
            bound_vars[j] = target;
            fn_scope[fn_scope_count++] = target;
        }
    }

    g_should_return = 0;
    g_call_depth++;
    Value ret = execute_lines(fn->start_line, fn->end_line, fn_scope, fn_scope_count);
    g_call_depth--;
    g_should_return = 0;

    /* Restore nama DAN sync nilai balik ke variabel caller */
    for (int j = 0; j < fn->param_count && j < argc; j++) {
        if (is_ref[j] && bound_vars[j]) {
            Value updated_val = bound_vars[j]->val;
            strcpy(bound_vars[j]->name, saved_names[j]);
            Variable *orig = scope_find(saved_names[j], caller_scope, caller_scope_count);
            if (orig) orig->val = updated_val;
        }
    }
    /* Bebaskan slot sementara HANYA jika tidak ada pemanggilan rekursif aktif */
    g_var_count = temp_var_start;

    return ret;
}

Value execute_lines(int from, int to, Variable **scope, int scope_count) {
    Value ret = make_int(0);
    int i = from;

    while (i <= to) {
        if (g_should_return) { ret = g_return_val; break; }
        if (g_should_break) break;

        CodeLine *cl = &g_lines[i];
        if (cl->count == 0) { i++; continue; }
        Token *tk = cl->tokens;
        int    tc = cl->count;

        /* ---- Skip definisi fungsi ---- */
        if (tk[0].type == TK_FUNGSI) {
            int depth = 0;
            for (int j = i+1; j <= to; j++) {
                if (g_lines[j].count == 0) continue;
                TokenType t = g_lines[j].tokens[0].type;
                if (t == TK_FUNGSI || t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
                else if (t == TK_SELESAI) { if (depth == 0) { i = j+1; goto next_iter; } depth--; }
            }
            i++; continue;
        }

        /* ---- DEKLARASI VARIABEL: angka x = ... ---- */
        if (tc >= 2 && (tk[0].type == TK_ANGKA || tk[0].type == TK_DESIMAL || tk[0].type == TK_KARAKTER || tk[0].type == TK_KOSONG)) {
            Variable *v = scope_create(tk[1].value, scope, &scope_count);
            if (v) {
                int ap = -1;
                for (int j = 2; j < tc; j++) if (tk[j].type == TK_ASSIGN) { ap = j; break; }
                if (ap >= 0 && ap+1 < tc)
                    v->val = eval_expr(tk+ap+1, tc-ap-1, scope, scope_count);
            }
            i++; continue;
        }

        /* ---- ASSIGNMENT arr[idx] = expr ---- */
        if (tc >= 5 && tk[0].type == TK_IDENTIFIER && tk[1].type == TK_LBRACKET) {
            int rb = -1;
            for (int j = 2; j < tc; j++) if (tk[j].type == TK_RBRACKET) { rb = j; break; }
            if (rb > 0 && rb+1 < tc && tk[rb+1].type == TK_ASSIGN) {
                Variable *v = scope_find(tk[0].value, scope, scope_count);
                if (v && v->val.type == VAL_ARRAY) {
                    int idx = val_as_int(eval_expr(tk+2, rb-2, scope, scope_count));
                    Value rhs = eval_expr(tk+rb+2, tc-rb-2, scope, scope_count);
                    if (idx >= 0 && idx < MAX_ARRAY_SIZE) {
                        v->val.arr[idx] = val_as_int(rhs);
                        if (idx >= v->val.arr_size) v->val.arr_size = idx+1;
                    }
                }
                i++; continue;
            }
        }

        /* ---- ASSIGNMENT: x = expr ---- */
        if (tc >= 3 && tk[0].type == TK_IDENTIFIER && tk[1].type == TK_ASSIGN) {
            Variable *v = scope_find(tk[0].value, scope, scope_count);
            if (!v) v = scope_create(tk[0].value, scope, &scope_count);
            if (v) v->val = eval_expr(tk+2, tc-2, scope, scope_count);
            i++; continue;
        }

        /* ---- INCREMENT: x tambah_satu ---- */
        if (tc == 2 && tk[0].type == TK_IDENTIFIER && tk[1].type == TK_INC) {
            Variable *v = scope_find(tk[0].value, scope, scope_count);
            if (v) {
                if (v->val.type == VAL_FLOAT) v->val.fval += 1.0;
                else { v->val.ival++; v->val.fval = (double)v->val.ival; }
            }
            i++; continue;
        }

        /* ---- DECREMENT: x kurang_satu ---- */
        if (tc == 2 && tk[0].type == TK_IDENTIFIER && tk[1].type == TK_DEC) {
            Variable *v = scope_find(tk[0].value, scope, scope_count);
            if (v) {
                if (v->val.type == VAL_FLOAT) v->val.fval -= 1.0;
                else { v->val.ival--; v->val.fval = (double)v->val.ival; }
            }
            i++; continue;
        }

        /* ---- TAMPILKAN ---- */
        if (tk[0].type == TK_TAMPILKAN) {
            int as = (tc > 1 && tk[1].type == TK_LPAREN) ? 2 : 1;
            int ae = (tc > 0 && tk[tc-1].type == TK_RPAREN) ? tc-1 : tc;
            Token *at[MAX_FUNC_PARAMS]; int ac[MAX_FUNC_PARAMS];
            int argc = split_args(tk+as, ae-as, at, ac);
            for (int j = 0; j < argc; j++)
                print_value(eval_expr(at[j], ac[j], scope, scope_count));
            printf("\n");
            i++; continue;
        }

        /* ---- MASUKKAN x ---- */
        if (tc >= 2 && tk[0].type == TK_MASUKKAN && tk[1].type == TK_IDENTIFIER) {
            Variable *v = scope_find(tk[1].value, scope, scope_count);
            if (!v) v = scope_create(tk[1].value, scope, &scope_count);
            if (v) {
                char buf[MAX_TOKEN_LEN];
                if (fgets(buf, sizeof(buf), stdin)) {
                    trim_newline(buf); char *ep;
                    long iv = strtol(buf, &ep, 10);
                    if (*ep == '\0') { v->val = make_int(iv); }
                    else { double dv = strtod(buf, &ep); if (*ep == '\0') v->val = make_float(dv); else v->val = make_string(buf); }
                }
            }
            i++; continue;
        }

        /* ---- KEMBALIKAN ---- */
        if (tk[0].type == TK_KEMBALIKAN) {
            ret = (tc > 1) ? eval_expr(tk+1, tc-1, scope, scope_count) : make_int(0);
            g_return_val = ret; g_should_return = 1;
            return ret;
        }

 /* ---- KALO (IF) ---- */
        if (tk[0].type == TK_JIKA) {
            int cond_end = tc;
            for (int j = 1; j < tc; j++) if (tk[j].type == TK_MAKA) { cond_end = j; break; }
            int cond = val_as_int(eval_expr(tk+1, cond_end-1, scope, scope_count)) != 0;
            int else_line = -1, selesai_line = -1;
            find_if_boundaries(i, to, &else_line, &selesai_line);
            if (selesai_line == -1) selesai_line = to + 1;

            if (else_line >= 0 && g_lines[else_line].tokens[0].type == TK_JIKA_TIDAK_JIKA) {
                /* else-if: jadikan if baru sementara */
                if (cond) {
                    execute_lines(i+1, else_line-1, scope, scope_count);
                    if (g_should_return) { ret = g_return_val; break; } /* FIX: Catch return */
                    i = selesai_line + 1; goto next_iter;
                } else {
                    /* Ganti sementara jadi TK_JIKA */
                    g_lines[else_line].tokens[0].type = TK_JIKA;
                    execute_lines(else_line, selesai_line, scope, scope_count);
                    g_lines[else_line].tokens[0].type = TK_JIKA_TIDAK_JIKA;
                    if (g_should_return) { ret = g_return_val; break; } /* FIX: Catch return */
                    i = selesai_line + 1; goto next_iter;
                }
            }

            if (cond)
                execute_lines(i+1, (else_line >= 0) ? else_line-1 : selesai_line-1, scope, scope_count);
            else if (else_line >= 0)
                execute_lines(else_line+1, selesai_line-1, scope, scope_count);

            /* FIX: Cek apakah ada 'kembalikan' dari dalam blok IF/ELSE sebelum melompat */
            if (g_should_return) { ret = g_return_val; break; }

            i = selesai_line + 1; goto next_iter;
        }

        /* ---- KALO TIDAK / KALO TIDAK KALO standalone (dilompati) ---- */
        if (tk[0].type == TK_JIKA_TIDAK || tk[0].type == TK_JIKA_TIDAK_JIKA) {
            int depth = 0;
            for (int j = i+1; j <= to; j++) {
                if (g_lines[j].count == 0) continue;
                TokenType t = g_lines[j].tokens[0].type;
                if (t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
                else if (t == TK_SELESAI) { if (depth==0) { i = j+1; goto next_iter; } depth--; }
            }
            i++; continue;
        }

        if (tk[0].type == TK_SELAMA) {
            int beking_pos = tc;
            for (int j = 1; j < tc; j++) if (tk[j].type == TK_LAKUKAN) { beking_pos = j; break; }
            int body_start = i+1, body_end = to, selesai_pos = to, depth = 0;
            for (int j = i+1; j <= to; j++) {
                if (g_lines[j].count == 0) continue;
                TokenType t = g_lines[j].tokens[0].type;
                if (t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
                else if (t == TK_SELESAI) { if (depth==0) { body_end=j-1; selesai_pos=j; break; } depth--; }
            }
            int max_iter = 10000000;
            while (max_iter-- > 0 && !g_should_return) {
                if (!val_as_int(eval_expr(tk+1, beking_pos-1, scope, scope_count))) break;
                g_should_break = 0;

                int temp_mem = g_var_count; /* SIMPAN MEMORI SEBELUM BLOK */
                execute_lines(body_start, body_end, scope, scope_count);
                g_var_count = temp_mem;     /* BERSIHKAN VARIABEL LOKAL SETELAH ITERASI (LEAK FIX) */

                if (g_should_break) { g_should_break = 0; break; }
            }
            i = selesai_pos + 1; goto next_iter;
        }

        /* ---- DARI (FOR) ---- */
        if (tk[0].type == TK_UNTUK) {
            int lak_pos = tc;
            for (int j = 1; j < tc; j++) if (tk[j].type == TK_LAKUKAN) { lak_pos = j; break; }
            int body_start = i+1, body_end = to, selesai_pos = to, depth = 0;
            for (int j = i+1; j <= to; j++) {
                if (g_lines[j].count == 0) continue;
                TokenType t = g_lines[j].tokens[0].type;
                if (t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
                else if (t == TK_SELESAI) { if (depth==0) { body_end=j-1; selesai_pos=j; break; } depth--; }
            }
            int sampe_pos = -1;
            for (int j = 1; j < lak_pos; j++)
                if (strcmp(tk[j].value, "sampe") == 0) { sampe_pos = j; break; }

            if (sampe_pos > 0) {
                Variable *v = scope_create(tk[1].value, scope, &scope_count);
                long start = val_as_int(eval_expr(tk+3, sampe_pos-3, scope, scope_count));
                long end   = val_as_int(eval_expr(tk+sampe_pos+1, lak_pos-sampe_pos-1, scope, scope_count));
                if (v) v->val = make_int(start);
                int max_iter = 10000000;
                while (max_iter-- > 0 && !g_should_return) {
                    if (val_as_int(v->val) > end) break;

                    int temp_mem = g_var_count; /* SIMPAN MEMORI SEBELUM BLOK */
                    execute_lines(body_start, body_end, scope, scope_count);
                    g_var_count = temp_mem;     /* BERSIHKAN VARIABEL LOKAL (LEAK FIX) */

                    if (g_should_break) { g_should_break = 0; break; }
                    v->val = make_int(val_as_int(v->val) + 1);
                }
            }
            i = selesai_pos + 1; goto next_iter;
        }

        /* ---- PANGGIL / pemanggilan fungsi statement ---- */
        if (tk[0].type == TK_PANGGIL || tk[0].type == TK_IDENTIFIER) {
            int fi = (tk[0].type == TK_PANGGIL) ? 1 : 0;
            if (fi < tc && fi+1 < tc &&
                tk[fi+1].type == TK_LPAREN && tk[tc-1].type == TK_RPAREN) {
                Function *fn = find_func(tk[fi].value);
                if (fn) {
                    Token *at[MAX_FUNC_PARAMS]; int ac[MAX_FUNC_PARAMS];
                    int argc = split_args(tk+fi+2, tc-fi-3, at, ac);
                    call_func_stmt(fn, at, ac, argc, scope, scope_count);
                    i++; continue;
                }
            }
        }

        /* ---- SELESAI / KELAR ---- */
        if (tk[0].type == TK_SELESAI) { i++; continue; }

        i++;
        next_iter:;
    }

    return ret;
}