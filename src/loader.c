#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/loader.h"
#include "../include/config.h"
#include "../include/globals.h"
#include "../include/value.h"
#include "../include/lexer.h"

/* ===================== MODULE LOADER ===================== */
static char loaded_files[32][MAX_TOKEN_LEN];
static int  loaded_file_count = 0;

void load_file(const char *filename) {
    /* Cegah infinite loop kalau file saling impor */
    for (int i = 0; i < loaded_file_count; i++) {
        if (strcmp(loaded_files[i], filename) == 0) return;
    }
    strcpy(loaded_files[loaded_file_count++], filename);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "[Error] Ndak bisa dapa buka file '%s'\n", filename);
        exit(1);
    }

    char line_buf[MAX_LINE_LEN];
    while (fgets(line_buf, sizeof(line_buf), fp) && g_line_count < MAX_LINES) {
        trim_newline(line_buf);
        CodeLine *cl = &g_lines[g_line_count];
        cl->indent = count_indent(line_buf);
        cl->count  = tokenize_line(line_buf, cl->tokens);

        /* Handle 'pake "file.mdo"' langsung pas di-load */
        if (cl->count >= 2 && cl->tokens[0].type == TK_PAKE && cl->tokens[1].type == TK_STRING) {
            load_file(cl->tokens[1].value);
            continue; /* Skip tambahin baris 'pake' ke memory eksekusi */
        }
        g_line_count++;
    }
    fclose(fp);
}