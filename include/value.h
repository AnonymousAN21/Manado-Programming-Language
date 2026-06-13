#ifndef VALUE_H
#define VALUE_H

#include "types.h"

/* ===================== NILAI HELPERS ===================== */
Value make_int(long v);
Value make_float(double v);
Value make_string(const char *s);
long  val_as_int(Value v);
double val_as_float(Value v);
void print_value(Value v);
void trim_newline(char *s);
int  count_indent(const char *line);

#endif /* VALUE_H */