#ifndef TYPES_H
#define TYPES_H

#include "config.h"

/* ===================== TIPE TOKEN ===================== */
typedef enum {
    /* Kata kunci struktur */
    TK_FUNGSI, TK_SELESAI, TK_KEMBALIKAN,
    TK_JIKA, TK_MAKA, TK_JIKA_TIDAK, TK_JIKA_TIDAK_JIKA,
    TK_SELAMA, TK_LAKUKAN, TK_UNTUK,
    /* I/O */
    TK_TAMPILKAN, TK_PANGGIL, TK_MASUKKAN, TK_PAKE,
    /* Tipe */
    TK_ANGKA, TK_DESIMAL, TK_KARAKTER, TK_KOSONG,
    /* Operator perbandingan */
    TK_SAMA_DENGAN, TK_TIDAK_SAMA_DENGAN,
    TK_LEBIH_BESAR, TK_LEBIH_KECIL,
    TK_LEBIH_BESAR_SAMA, TK_LEBIH_KECIL_SAMA,
    /* Operator logika */
    TK_DAN, TK_ATAU,
    /* Operator aritmatika */
    TK_PLUS, TK_MINUS, TK_KALI, TK_BAGI, TK_MODULO,
    /* Operator unary increment/decrement */
    TK_INC, TK_DEC,
    /* Tanda */
    TK_ASSIGN, TK_LPAREN, TK_RPAREN, TK_COMMA,
    TK_LBRACKET, TK_RBRACKET,
    /* Literal & identifier */
    TK_NUMBER, TK_FLOAT_NUM, TK_STRING, TK_CHAR_LIT,
    TK_IDENTIFIER,
    TK_UNKNOWN
} TokenType;

/* ===================== STRUKTUR DATA ===================== */
typedef struct {
    TokenType type;
    char      value[MAX_TOKEN_LEN];
} Token;

typedef enum { VAL_INT, VAL_FLOAT, VAL_STRING, VAL_ARRAY } ValType;

typedef struct _Value {
    ValType type;
    long    ival;
    double  fval;
    char    sval[MAX_TOKEN_LEN];
    /* Array */
    long    arr[MAX_ARRAY_SIZE];
    int     arr_size;
} Value;

/*
 * Variable menyimpan nilai DAN slot memori (index di g_vars atau local_vars).
 * Fungsi menerima pointer ke Variable asli — perubahan langsung terasa di caller.
 */
typedef struct {
    char  name[MAX_TOKEN_LEN];
    Value val;
} Variable;

typedef struct {
    Token tokens[MAX_TOKENS];
    int   count;
    int   indent;
} CodeLine;

typedef struct {
    char name[MAX_TOKEN_LEN];
    /* Nama parameter saja; binding dilakukan by-reference saat panggil */
    char params[MAX_FUNC_PARAMS][MAX_TOKEN_LEN];
    int  param_count;
    int  start_line;
    int  end_line;
} Function;

#endif /* TYPES_H */