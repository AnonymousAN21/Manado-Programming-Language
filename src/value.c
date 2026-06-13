#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/value.h"

/* ===================== NILAI HELPERS ===================== */
Value make_int(long v) {
    Value r; memset(&r, 0, sizeof(Value));
    r.type = VAL_INT; r.ival = v; r.fval = (double)v;
    return r;
}
Value make_float(double v) {
    Value r; memset(&r, 0, sizeof(Value));
    r.type = VAL_FLOAT; r.fval = v; r.ival = (long)v;
    return r;
}
Value make_string(const char *s) {
    Value r; memset(&r, 0, sizeof(Value));
    r.type = VAL_STRING;
    strncpy(r.sval, s, MAX_TOKEN_LEN-1);
    r.sval[MAX_TOKEN_LEN-1] = '\0';
    return r;
}
long val_as_int(Value v) {
    if (v.type == VAL_INT)    return v.ival;
    if (v.type == VAL_FLOAT)  return (long)v.fval;
    if (v.type == VAL_STRING) return atol(v.sval);
    return 0;
}
double val_as_float(Value v) {
    if (v.type == VAL_FLOAT)  return v.fval;
    if (v.type == VAL_INT)    return (double)v.ival;
    if (v.type == VAL_STRING) return atof(v.sval);
    return 0.0;
}
void print_value(Value v) {
    if      (v.type == VAL_INT)    printf("%ld", v.ival);
    else if (v.type == VAL_FLOAT)  printf("%g",  v.fval);
    else if (v.type == VAL_STRING) printf("%s",  v.sval);
}
void trim_newline(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) s[--len] = '\0';
}
int count_indent(const char *line) {
    int n = 0;
    for (int i = 0; line[i]; i++) {
        if      (line[i] == ' ')  n++;
        else if (line[i] == '\t') n += 4;
        else break;
    }
    return n;
}