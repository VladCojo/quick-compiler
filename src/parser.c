#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

int iTk;  // the iterator in tokens
Token *consumed;  // the last consumed token

// same as err, but also prints the line of the current token
_Noreturn void tkerr(const char *fmt, ...) {
    fprintf(stderr, "error in line %d: ", tokens[iTk].line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];
        return true;
    }
    return false;
}

// Function definitions
bool consume(int code);
bool program();
void parse();
bool defVar();
bool baseType();
bool defFunc();
bool block();
bool funcParams();
bool funcParam();
bool instr();
bool expr();
bool exprLogic();
bool exprAssign();
bool exprComp();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool factor();

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    for (;;) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}
        else break;
    }
    if (consume(FINISH)) {
        return true;
    } else tkerr("syntax error");
    return false;
}

void parse() {
    iTk = 0;
    program();
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        return true;
                    } else tkerr("Missing ';' after variable declaration");
                }
            } else tkerr("Missing ':' in variable declaration");
        } else tkerr("Missing identifier in variable declaration");
    }
    return false;
}

// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
    if (consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR)) {
        return true;
    }
    return false;
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                funcParams();
                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            while (defVar()) {}
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                } else tkerr("Missing 'end' after function definition");
                            }
                        }
                    } else tkerr("Missing ':' in function declaration");
                } else tkerr("Missing ')' after function parameters");
            } else tkerr("Missing '(' after function identifier");
        } else tkerr("Missing function identifier");
    }
    return false;
}

// block ::= instr+
bool block() {
    if (instr()) {
        while (instr()) {}
        return true;
    }
    return false;
}

// funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams() {
    if (funcParam()) {
        while (consume(COMMA)) {
            if (!funcParam()) tkerr("Invalid function parameter after ','");
        }
        return true;
    }
    return false;
}

// funcParam ::= ID COLON baseType
bool funcParam() {
    if (consume(ID)) {
        if (consume(COLON)) {
            if (baseType()) {
                return true;
            } else tkerr("Invalid base type in function parameter");
        } else tkerr("Missing ':' in function parameter");
    }
    return false;
}

// instr ::= expr? SEMICOLON | IF LPAR expr RPAR block ( ELSE block )? END | RETURN expr SEMICOLON | WHILE LPAR expr RPAR block END
bool instr() {
    int start = iTk;
    if (expr()) {
        if (consume(SEMICOLON)) {
            return true;
        }
        iTk = start;
    }
    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(ELSE)) {
                            if (!block()) tkerr("Expected block after 'else'");
                        }
                        if (consume(END)) {
                            return true;
                        } else tkerr("Missing 'end' after 'if' statement");
                    }
                }
            }
        }
    } else if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("Missing ';' after return statement");
        } else tkerr("Missing expression in return statement");
    } else if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        } else tkerr("Missing 'end' after 'while' loop");
                    }
                }
            }
        }
    }
    return false;
}

// expr ::= exprLogic
bool expr() {
    return exprLogic();
}

// exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
    if (exprAssign()) {
        while (consume(AND) || consume(OR)) {
            if (!exprAssign()) tkerr("Invalid expression after 'and/or'");
        }
        return true;
    }
    return false;
}

// exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
    int start = iTk;
    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (exprComp()) {
                return true;
            } else tkerr("Invalid expression after '='");
        }
        iTk = start;
    }
    return exprComp();
}

// exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
    if (exprAdd()) {
        if (consume(LESS) || consume(EQUAL)) {
            if (!exprAdd()) tkerr("Invalid expression after '<' or '='");
        }
        return true;
    }
    return false;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
    if (exprMul()) {
        while (consume(ADD) || consume(SUB)) {
            if (!exprMul()) tkerr("Invalid expression after '+' or '-'");
        }
        return true;
    }
    return false;
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
    if (exprPrefix()) {
        while (consume(MUL) || consume(DIV)) {
            if (!exprPrefix()) tkerr("Invalid expression after '*' or '/'");
        }
        return true;
    }
    return false;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    if (consume(SUB) || consume(NOT)) {
        return factor();
    }
    return factor();
}

// factor ::= INT | REAL | STR | LPAR expr RPAR | ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
bool factor() {
    if (consume(INT) || consume(REAL) || consume(STRING)) {
        return true;
    }
    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            } else tkerr("Missing ')' after expression");
        }
    }
    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) tkerr("Invalid expression after ',' in function call");
                }
            }
            if (consume(RPAR)) {
                return true;
            } else tkerr("Missing ')' after function arguments");
        }
        return true;
    }
    return false;
}