#ifndef __LEX__
#define __LEX__

#define MAXLEN 256
/*edit*/
/*
int regis = -1;
int nid = 0;
int newv = 0;
*/
// Token types
typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,
    ADDSUB, MULDIV,
    ASSIGN, 
    LPAREN, RPAREN,
    AND,OR,XOR,
    INCDEC
} TokenSet;

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

extern int First;
/*edit*/

#endif // __LEX__
#ifndef __PARSER__
#define __PARSER__

//#include "lex.h"
#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);

//Extend Get address of variable
extern int getmem(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

/*New extend grammar*/
extern void Estatement(void);
extern BTNode *assign_expr(void);
extern BTNode *or_expr (void);
extern BTNode *or_expr_tail(BTNode *left_brunch);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left_brunch);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left_brunch);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left_brunch);
extern BTNode *and_expr(void);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left_brunch);
extern BTNode *unary_expr(void);
extern BTNode *Efactor(void);
/*Extend grammar end*/

extern BTNode *factor(void);
extern BTNode *term(void);
extern BTNode *expr(void);
extern void statement(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__
#ifndef __CODEGEN__
#define __CODEGEN__

//#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//#include "lex.h"

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
            lexeme[1] = '\0';
            //lexeme[2] = '\0';
            return INCDEC;
        }
        ungetc(c, stdin);
        lexeme[1] = '\0';
        return ADDSUB;
        
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
    } else if (isalpha(c) || c == '_') {
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
        strcpy(lexeme, "&");
        return AND;
    }else if ( c == '|'){
        strcpy(lexeme, "|");
        return OR;
    }else if ( c == '^'){
        strcpy(lexeme, "^");
        return XOR;
    }
    /*END extension*/
    else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
    if(First && curToken!= UNKNOWN) First = 0;
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "parser.h"

int rec_cha = 0;
int pos_sym = 0;
int First;
int sbcount = 0;
Symbol table[TBLSIZE];
BTNode *left;

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    /*
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    newv++;
    */
   error(NOTFOUND);
    return 0;
}

/*Extend for get address*/
/*
int getmem(char *str){
    int i=0;

    for (i = 0; i < sbcount; i++){
        if (strcmp(str, table[i].name) == 0)
            return i*4;
    }
}*/

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    //newv--;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

/*Extend parser for project*/
// statement(E) := END | assign_expr END
void Estatement(void){
   // printf("In state\n");
    rec_cha = 0;
    BTNode *retp = NULL;

    if(match(ENDFILE)){
        printf("MOV r0, [0]\n");
        printf("MOV r1, [4]\n");
        printf("MOV r2, [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }else if(match(END)){
        advance(); /*wait to see what it have to do*/
    } else{
        retp = assign_expr();
        if(match(END)){
            /*wait to add assembly*/
            if(!PRINTERR){
                evaluateTree(retp);
                freeTree(retp);
            }else{
                printf("%d\n", evaluateTree(retp));
                printf("Prefix traversal: ");
                printPrefix(retp);
                printf("\n");
                printf("Assembly check\n");
            }
            
            /*
            regis = -1;
            newv=0;
            advance();
            */
        }else{
            error(SYNTAXERR);
        }
    }
}

// assign_expr := ID ASSIGN assign_expr | or_expr 
// change to := 
BTNode *assign_expr(void){
    //printf("In assign\n");
    First = 1;
    left = NULL;
    BTNode *retp = NULL;
    
    //left = or_expr();
    if(match(ID)){
        //left = or_expr;
        left = makeNode(ID, getLexeme());
        advance();
        if(match(END)) return left;
        if(match(RPAREN)) return left;
        if(match(ASSIGN)){
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
            return retp;
        } 
        else {
            return left;
            //error(SYNTAXERR);
        }
    } else {
        //retp = left;
        return or_expr();
    } 
//    else{
//        error(NOTNUMID);
//    }
    ///return retp;
}

// or_expr          := xor_expr or_expr_tail 
// or_expr_tail     := OR xor_expr or_expr_tail | NiL
BTNode *or_expr(void){
    //printf("In or\n");
    
    
    //retp = left = xor_expr();
    if (match(OR)) return or_expr_tail(left);
    BTNode *retp = xor_expr();
    return or_expr_tail(retp);
    /*
        retp = makeNode(OR, getLexeme());
        advance();
        retp->left = left;
        retp->right = xor_expr();
        left = retp;
    }*/
    //return retp;
} 
BTNode *or_expr_tail(BTNode* left){
    BTNode *rept = NULL;
    if(match(OR)){
        rept = makeNode(OR, getLexeme());
        advance;
        rept->left = left;
        rept->right = xor_expr();
        return or_expr_tail(rept);
    }else{
        return left;
    }
}

//xor_expr         := and_expr xor_expr_tail 
//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL 
BTNode *xor_expr(void){
    //printf("In xor\n");
    if(match(XOR))
        return xor_expr_tail(left);
    BTNode *retp=and_expr();
    return xor_expr_tail(retp);
    /*
    retp = left = and_expr();
    while (match(XOR))
    {
        retp = makeNode(XOR, getLexeme());
        advance();
        retp->left = left;
        retp->right = and_expr();
        left = retp;
    }
    return retp;
    */
}
BTNode *xor_expr_tail(BTNode* left){
    BTNode *retp=NULL;

    //retp = left = and_expr();
    if(match(XOR))
    {
        retp = makeNode(XOR, getLexeme());
        advance();
        retp->left = left;
        retp->right = and_expr();
        return xor_expr_tail(retp);
        //left = retp;
    }else{
        return left;
    }
}

//and_expr         := addsub_expr and_expr_tail 
//and_expr_tail    := AND addsub_expr and_expr_tail | NiL 
BTNode *and_expr(void){
    //printf("In and\n");
    if(match(AND))
        return and_expr_tail(left);
    BTNode *retp = addsub_expr();
    return and_expr_tail(retp);
}
BTNode *and_expr_tail(BTNode *left){
    BTNode *retp=NULL;

    //retp = left = addsub_expr();
    if(match(AND)){
        retp = makeNode(AND, getLexeme());
        advance();
        retp->left = left;
        retp->right = addsub_expr();
        return and_expr_tail(retp);
        //left = retp;
    }else{
        return left;
    }
}

// addsub_expr      := muldiv_expr addsub_expr_tail 
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL 
BTNode *addsub_expr(void){
    //printf("In addsub\n");
    if(!First){
        if(match(ADDSUB)) return addsub_expr_tail(left);
    }BTNode *rept = muldiv_expr();
    return  addsub_expr_tail(rept);
}
BTNode *addsub_expr_tail(BTNode* left){
    BTNode *retp = NULL;

    //retp = left = muldiv_expr();
    if (match(ADDSUB))
    {
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = muldiv_expr();
        return addsub_expr_tail(retp);
        //left = retp;
    }else{
        return left;
    }
}
// muldiv_expr      := unary_expr muldiv_expr_tail 
// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL 
BTNode *muldiv_expr(void){
    //printf("In muldiv\n");
    if(match(MULDIV))
        return muldiv_expr_tail(left);
    BTNode *retp = unary_expr();
    return muldiv_expr_tail(retp);
}
BTNode *muldiv_expr_tail(BTNode *left){
    BTNode *retp = NULL;

    //retp = left = unary_expr();
    if (match(MULDIV))
    {
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = unary_expr();
        return muldiv_expr_tail(retp);
        //left = retp;
    }else{
        return left;
    }
    //return retp;
}

// unary_expr  := ADDSUB unary_expr | factor
BTNode *unary_expr(void){
    //printf("In unary\n");
    BTNode *retp = NULL;
    
    //left = Efactor();
    if(match(ADDSUB)){
        int n = 0;
        if(n){
            retp = makeNode(ADDSUB, getLexeme());
            advance();
        }
        else{
            int mi_count = 0;
            while(match(ADDSUB)){
                if(*getLexeme() == '-')
                    mi_count++;
                advance();
            }if(mi_count %2 == 1){
                retp = makeNode(ADDSUB, "-");
            }else{
                retp = makeNode(ADDSUB, "+");
            }
        }
        //retp = makeNode(ADDSUB, getLexeme());
        //advance();
        retp->left = makeNode(INT, "0");
        retp->right = unary_expr();
    } else{
        return factor();
    }
    
    //return retp;
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *Efactor(void){
    //printf("In fac\n");
    BTNode *retp = NULL;

    if(match(INT)){
        retp = makeNode(INT, getLexeme());
        advance();
    }else if(match(ID)){
        retp = makeNode(ID, getLexeme());
        advance();
    } else if(match(INCDEC)){
        retp = makeNode(INCDEC, getLexeme());
        advance();
        if(match(LPAREN)){
            advance();
            if(match(ID)){
                retp->right = makeNode(ID, getLexeme());
                advance();
            }else{
                error(SYNTAXERR);
            }
            if(match(RPAREN)){
                advance();
            } else{
                error(MISPAREN);
            }
        } else if(match(ID)){
            retp->right = makeNode(ID, getLexeme());
            advance();
        } else{
            error(SYNTAXERR);
        }
    } else if(match(LPAREN)){
        advance();
        retp = assign_expr();
        if(match(RPAREN)){
            advance();
        } else {
            error(MISPAREN);
        }
    } else {
        error(NOTNUMID);
    }
    return retp;
}

/*Extention End*/

// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  | 
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;

    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        left = makeNode(ID, getLexeme());
        advance();
        if (!match(ASSIGN)) {
            retp = left;
        } else {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
    } else if (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        retp->left = makeNode(INT, "0");
        advance();
        if (match(INT)) {
            retp->right = makeNode(INT, getLexeme());
            advance();
        } else if (match(ID)) {
            retp->right = makeNode(ID, getLexeme());
            advance();
        } else if (match(LPAREN)) {
            advance();
            retp->right = expr();
            if (match(RPAREN))
                advance();
            else
                error(MISPAREN);
        } else {
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}

// term      := factor term_tail
// term_tail := MULDIV factor term_tail | NiL
BTNode *term(void) {
    BTNode *retp = NULL, *left = NULL;

    retp = left = factor();
    while (match(MULDIV)) {
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = factor();
        left = retp;
    }
    return retp;
}

// expr      := term expr_tail
// expr_tail := ADDSUB term expr_tail | NiL
BTNode *expr(void) {
    BTNode *retp = NULL, *left = NULL;

    retp = left = term();
    while (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = term();
        left = retp;
    }
    return retp;
}


// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;

    if (match(ENDFILE)) {
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = expr();
        if (match(END)) {
            printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    /*
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }*/
    printf("EXIT 1\n");
    exit(0);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "codeGen.h"
int idnum = 0;

int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                idnum++;
                if(rec_cha > 7){
                    printf("MOV [%d] r%d\n", sbcount << 2, rec_cha % 8);
                    sbcount++;
                }
                printf("MOV [%d] r%d\n", rec_cha % 8, pos_sym);
                rec_cha++;
                break;
            case INT:
                retval = atoi(root->lexeme);
                if(rec_cha > 7){
                    printf("MOV [%d] r%d\n", sbcount << 2, rec_cha % 8);
                    sbcount++;
                }
                printf("MOV [%d] r%d\n", rec_cha % 8, pos_sym);
                rec_cha++;
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                printf("MOV [%d], r%d\n", pos_sym, (rec_cha-1)%8);
                idnum++;
                break;
            
            case MULDIV:
                if (strcmp(root->lexeme, "*") == 0) {
                    lv = evaluateTree(root->left);
                    rv = evaluateTree(root->right);
                    retval = lv * rv;
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                    printf("MUL r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                } else if (strcmp(root->lexeme, "/") == 0) {
                    lv = evaluateTree(root->left);
                    idnum = 0;
                    rv = evaluateTree(root->right);
                    if (rv == 0){
                        if(idnum == 0){
                            error(DIVZERO);
                        }
                        /*else{
                            rv = 1;
                            lv = 0;
                        }*/
                    }else{
                        retval = lv / rv;
                        if(rec_cha > 7){
                            sbcount--;
                            printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                        }
                        printf("DIV r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                        
                    }rec_cha--;
                }
                break;
            /*Extend for Project*/
            case ADDSUB:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                }
                rec_cha--;
                break;
            case INCDEC:
                if(root->right->data != ID){
                    error(SYNTAXERR);
                }
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "++") == 0) {
                    retval = ++rv;
                    printf("ADD [%d], 1\n", sbcount << 2);
                } else if (strcmp(root->lexeme, "--") == 0) {
                    retval = --rv;
                    printf("SUB [%d], 1\n", sbcount << 2);
                }
                break;
            case OR:
            case XOR:
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                    printf("OR r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                } else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                    printf("XOR r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                } else if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                    printf("AND r%d, r%d\n", (rec_cha-2) % 8, (rec_cha-1) % 8);
                    if(rec_cha > 7){
                        sbcount--;
                        printf("MOV r%d [%d]\n", (rec_cha-1) % 8, sbcount << 2);
                    }
                }
                rec_cha--;
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  | 
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main() {
    initTable();
    //printf(">> ");
    while (1) {
        //statement();
        Estatement();
    }
    return 0;
}
