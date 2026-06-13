#ifndef LEXER_H
#define LEXER_H

#include "types.h"

TokenType keyword_type(const char *w);
int tokenize_line(const char *line, Token *tokens);

#endif /* LEXER_H */