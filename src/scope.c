#include <stdio.h>
#include <string.h>
#include "../include/scope.h"
#include "../include/globals.h"
#include "../include/value.h"

/* ===================== SCOPE: CARI & BUAT VARIABEL =====================
 *
 * scope adalah array of pointer ke Variable.
 * Pencarian LIFO (dari akhir ke awal) agar local shadow global.
 * Fungsi menerima pointer ke Variable yang sama → by-reference otomatis.
 */
Variable* scope_find(const char *name, Variable **scope, int scope_count) {
    /* 1. Cari dari local scope dulu (prioritas utama) */
    for (int i = scope_count - 1; i >= 0; i--) {
        if (strcmp(scope[i]->name, name) == 0) return scope[i];
    }
    /* 2. Kalau nda dapa, cari di global scope */
    for (int i = g_global_count - 1; i >= 0; i--) {
        if (strcmp(g_global_scope[i]->name, name) == 0) return g_global_scope[i];
    }
    return NULL;
}

/*
 * Buat variabel baru di pool global lalu tambahkan ke scope.
 * Ini memastikan semua variabel punya alamat tetap di g_vars.
 */
Variable* scope_create(const char *name, Variable **scope, int *scope_count) {
    Variable *existing = scope_find(name, scope, *scope_count);
    if (existing) return existing; /* Update yang sudah ada */

    if (g_var_count >= MAX_VARS) {
        fprintf(stderr, "[Error] Memori variabel full!\n");
        return NULL;
    }
    Variable *nv = &g_vars[g_var_count++];
    memset(nv, 0, sizeof(Variable));
    strcpy(nv->name, name);
    nv->val = make_int(0);

    /* Masukkan ke scope yang aktif */
    if (*scope_count < MAX_VARS) {
        scope[(*scope_count)++] = nv;
    }

    /* Kalau dipanggil di luar fungsi (depth 0), daftarkan sebagai GLOBAL */
    if (g_call_depth == 0 && g_global_count < MAX_VARS) {
        g_global_scope[g_global_count++] = nv;
    }
    return nv;
}