#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

/* ===================== LEXER ===================== */

/*
 * Kata kunci. Operator simbolik sudah dihapus — diganti kata.
 * tambah, kurang, kali, bagi → ditangani di tokenizer sebagai TK_IDENTIFIER
 * lalu dikenali sebagai operator di sini.
 */
static struct { const char *word; TokenType type; } g_keywords[] = {
    /* Struktur */
    {"fungsi",                    TK_FUNGSI},
    {"kelar",                     TK_SELESAI},
    {"kembalikan",                TK_KEMBALIKAN},
    {"kalo",                      TK_JIKA},
    {"maka",                      TK_MAKA},
    {"ketika",                    TK_SELAMA},
    {"beking",                    TK_LAKUKAN},
    {"dari",                      TK_UNTUK},
    /* I/O */
    {"semuncul",                 TK_TAMPILKAN},
    {"panggil",                   TK_PANGGIL},
    {"masukkan",                  TK_MASUKKAN},
    {"pake",                      TK_PAKE},
    /* Tipe */
    {"angka",                     TK_ANGKA},
    {"desimal",                   TK_DESIMAL},
    {"karakter",                  TK_KARAKTER},
    {"kosong",                    TK_KOSONG},
    /* Operator perbandingan (kata) */
    {"sama_dengan",               TK_SAMA_DENGAN},
    {"tidak_sama_dengan",         TK_TIDAK_SAMA_DENGAN},
    {"lebih_besar_sama_dengan",   TK_LEBIH_BESAR_SAMA},
    {"lebih_kecil_sama_dengan",   TK_LEBIH_KECIL_SAMA},
    {"lebih_besar",               TK_LEBIH_BESAR},
    {"lebih_kecil",               TK_LEBIH_KECIL},
    /* Operator logika (kata) */
    {"deng",                      TK_DAN},
    {"ato",                       TK_ATAU},
    /* Operator aritmatika (kata) */
    {"tambah",                    TK_PLUS},
    {"kurang",                    TK_MINUS},
    {"kali",                      TK_KALI},
    {"bagi",                      TK_BAGI},
    {"modulo",                    TK_MODULO},
    /* Increment / decrement */
    {"tambah_satu",               TK_INC},
    {"kurang_satu",               TK_DEC},
    /* else */
    {"nda",                     TK_JIKA_TIDAK},
    {NULL, TK_UNKNOWN}
};

TokenType keyword_type(const char *w) {
    for (int i = 0; g_keywords[i].word; i++)
        if (strcmp(g_keywords[i].word, w) == 0) return g_keywords[i].type;
    return TK_IDENTIFIER;
}

int tokenize_line(const char *line, Token *tokens) {
    int tcount = 0, i = 0, len = strlen(line);
    while (i < len && isspace((unsigned char)line[i])) i++;

    while (i < len && tcount < MAX_TOKENS - 1) {
        /* Komentar */
        if (line[i] == '/' && i+1 < len && line[i+1] == '/') break;

        /* String literal */
        if (line[i] == '"') {
            int j = 0; i++;
            tokens[tcount].type = TK_STRING;
            while (i < len && line[i] != '"') {
                if (line[i] == '\\' && i+1 < len) {
                    i++;
                    switch (line[i]) {
                        case 'n': tokens[tcount].value[j++] = '\n'; break;
                        case 't': tokens[tcount].value[j++] = '\t'; break;
                        default:  tokens[tcount].value[j++] = line[i]; break;
                    }
                } else {
                    tokens[tcount].value[j++] = line[i];
                }
                i++;
            }
            tokens[tcount].value[j] = '\0';
            if (i < len) i++;
            tcount++; continue;
        }

        /* Char literal */
        if (line[i] == '\'') {
            i++;
            tokens[tcount].type = TK_CHAR_LIT;
            int j = 0;
            while (i < len && line[i] != '\'') tokens[tcount].value[j++] = line[i++];
            tokens[tcount].value[j] = '\0';
            if (i < len) i++;
            tcount++; continue;
        }

        /* Number */
        if (isdigit((unsigned char)line[i])) {
            int j = 0, is_float = 0;
            while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.')) {
                if (line[i] == '.') is_float = 1;
                tokens[tcount].value[j++] = line[i++];
            }
            tokens[tcount].value[j] = '\0';
            tokens[tcount].type = is_float ? TK_FLOAT_NUM : TK_NUMBER;
            tcount++; continue;
        }

        /* Simbol */
        if (line[i] == '(') { tokens[tcount].type=TK_LPAREN;   tokens[tcount].value[0]='('; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }
        if (line[i] == ')') { tokens[tcount].type=TK_RPAREN;   tokens[tcount].value[0]=')'; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }
        if (line[i] == '[') { tokens[tcount].type=TK_LBRACKET; tokens[tcount].value[0]='['; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }
        if (line[i] == ']') { tokens[tcount].type=TK_RBRACKET; tokens[tcount].value[0]=']'; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }
        if (line[i] == ',') { tokens[tcount].type=TK_COMMA;    tokens[tcount].value[0]=','; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }
        if (line[i] == '=') { tokens[tcount].type=TK_ASSIGN;   tokens[tcount].value[0]='='; tokens[tcount].value[1]='\0'; tcount++; i++; continue; }

        /* Identifier / keyword */
        if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            int j = 0;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_'))
                tokens[tcount].value[j++] = line[i++];
            tokens[tcount].value[j] = '\0';

            /* Deteksi "kalo tidak, kalo" dan "kalo tidak" */
            if (strcmp(tokens[tcount].value, "kalo") == 0) {
                int k = i;
                while (k < len && isspace((unsigned char)line[k])) k++;
                if (strncmp(&line[k], "nda,", 4) == 0) {
                    int m = k + 4;
                    while (m < len && isspace((unsigned char)line[m])) m++;
                    if (strncmp(&line[m], "kalo", 4) == 0 &&
                        !isalnum((unsigned char)line[m+4]) && line[m+4] != '_') {
                        i = m + 4;
                        tokens[tcount].type = TK_JIKA_TIDAK_JIKA;
                        strcpy(tokens[tcount].value, "kalo_nda_kalo");
                        tcount++; continue;
                    }
                }
                if (strncmp(&line[k], "nda", 3) == 0 &&
                    !isalnum((unsigned char)line[k+3]) && line[k+3] != '_') {
                    i = k + 3;
                    tokens[tcount].type = TK_JIKA_TIDAK;
                    strcpy(tokens[tcount].value, "kalo_nda");
                    tcount++; continue;
                }
                tokens[tcount].type = TK_JIKA;
            } else {
                tokens[tcount].type = keyword_type(tokens[tcount].value);
            }
            tcount++; continue;
        }

        /* Skip whitespace */
        if (isspace((unsigned char)line[i])) { i++; continue; }

        /* Unknown */
        tokens[tcount].type = TK_UNKNOWN;
        tokens[tcount].value[0] = line[i];
        tokens[tcount].value[1] = '\0';
        tcount++; i++;
    }
    return tcount;
}