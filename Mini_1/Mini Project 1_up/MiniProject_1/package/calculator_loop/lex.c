#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    /*check*/ //printf("In getToken\n");
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        /*wait for Extend invalid variable define*/
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        /*Extend for INCDEC*/
        lexeme[0] = c;
        c = fgetc(stdin);
        if(c == lexeme[0]){
            lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        }else{
            ungetc(c, stdin);
            lexeme[1] = '\0';
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c) | c == '_') {
        /*Extend more variable type*/
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || isdigit(c) || c == '_') && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c,stdin);
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } 
    /*Extension for project*/
    else if (c == '&'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    }else if ( c == '|'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    }else if ( c == '^'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }
    /*END extension*/
    else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
        //printf("Token is %d\n",curToken);
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}
