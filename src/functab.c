#include <string.h>
#include "../include/functab.h"
#include "../include/globals.h"

/* ===================== FIND FUNCTION ===================== */
Function* find_func(const char *name) {
    for (int i = 0; i < g_func_count; i++)
        if (strcmp(g_funcs[i].name, name) == 0) return &g_funcs[i];
    return NULL;
}

/* ===================== PARSE FUNGSI ===================== */
void parse_functions(void) {
    for (int i = 0; i < g_line_count; i++) {
        CodeLine *cl = &g_lines[i];
        if (cl->count < 2 || cl->tokens[0].type != TK_FUNGSI) continue;

        Function *fn = &g_funcs[g_func_count++];
        memset(fn, 0, sizeof(Function));
        strcpy(fn->name, cl->tokens[1].value);
        fn->start_line = i + 1;

        /* Parse parameter */
        int pi = 2;
        if (pi < cl->count && cl->tokens[pi].type == TK_LPAREN) {
            pi++;
            while (pi < cl->count && cl->tokens[pi].type != TK_RPAREN) {
                TokenType t = cl->tokens[pi].type;
                if (t == TK_COMMA || t == TK_ANGKA || t == TK_DESIMAL ||
                    t == TK_KARAKTER || t == TK_KOSONG) { pi++; continue; }
                if (t == TK_IDENTIFIER && fn->param_count < MAX_FUNC_PARAMS) {
                    strcpy(fn->params[fn->param_count++], cl->tokens[pi].value);
                }
                pi++;
            }
        }

        /* Cari selesai */
        int depth = 0; fn->end_line = g_line_count - 1;
        for (int j = i+1; j < g_line_count; j++) {
            if (g_lines[j].count == 0) continue;
            TokenType t = g_lines[j].tokens[0].type;
            if (t == TK_FUNGSI || t == TK_JIKA || t == TK_SELAMA || t == TK_UNTUK) depth++;
            else if (t == TK_SELESAI) { if (depth==0) { fn->end_line = j-1; break; } depth--; }
        }
    }
}