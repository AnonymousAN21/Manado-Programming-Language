/*
 * UNSRAT Interpreter - Versi Komplit
 * Compile: gcc -o unsrat *.c
 * Jalankan: ./unsrat nama_file.unsrat
 *
 * OPERATOR:
 *   kali  (*), tambah (+), kurang (-), bagi (/)
 *   tambah_satu (++), kurang_satu (--)
 *   lebih_besar_sama_dengan (>=), lebih_kecil_sama_dengan (<=)
 *   lebih_besar (>), lebih_kecil (<)
 *   ato (||), deng (&&)
 *   sama_dengan (==), tidak_sama_dengan (!=)
 *
 * POINTER:
 *   Semua variabel dipass by reference otomatis (seperti Python list/dict).
 *   Tidak perlu & atau * — perubahan di dalam fungsi langsung terasa di luar.
 */

#include <stdio.h>
#include "../include/globals.h"
#include "../include/value.h"
#include "../include/functab.h"
#include "../include/exec.h"
#include "../include/loader.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Cara pake: ./manado <file.mdo>\n");
        return 1;
    }

    /* 1. Muat semua file dan dependencies-nya */
    load_file(argv[1]);
    parse_functions();

    /* 2. Setup environment untuk Global Variables */
    Variable *root_scope[MAX_VARS];
    int root_scope_count = 0;

    /* 3. Eksekusi scope GLOBAL dulu (variabel di luar fungsi) */
    g_should_return = 0;
    g_call_depth = 0;
    execute_lines(0, g_line_count - 1, root_scope, root_scope_count);

/* 4. Baru panggil fungsi utama() */
    Function *main_fn = find_func("utama");
    if (main_fn) {
        g_should_return = 0;

        /* FIX: Tandai bahwa kita masuk ke dalam fungsi agar variabel lokal tidak bocor ke global */
        g_call_depth++;
        execute_lines(main_fn->start_line, main_fn->end_line, root_scope, root_scope_count);
        g_call_depth--;
    }

    return (int)val_as_int(g_return_val);
}