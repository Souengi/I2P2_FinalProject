#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN,
    END,
    ENDFILE,
    INT,
    ID,
    ADDSUB,
    MULDIV,
    AND,
    OR,
    XOR,
    ASSIGN,
    LPAREN,
    RPAREN,
    INCDEC
} TokenSet;

// Test if a token matches the current token
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

extern int isFirst;

#endif  // __LEX__
#ifndef __PARSER__
#define __PARSER__

#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0
#define MAXREG 7

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum)                                                       \
    {                                                                         \
        if (PRINTERR)                                                         \
            fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
        err(errorNum);                                                        \
    }
// Error types
typedef enum {
    UNDEFINED,
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    NOTLVAL,
    DIVZERO,
    SYNTAXERR,
    NOTVAR,
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

extern int regc;
extern int possym;
extern int sbcount;
// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *factor(void);

extern BTNode *unary_expr(void);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *assign_expr();
extern void statement(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif  // __PARSER__
#ifndef __CODEGEN__
#define __CODEGEN__


// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];
TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t')
        ;

    if (isdigit(c)) {
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
    }
    else if (c == '+' || c == '-') {
        lexeme[0] = c;
        c = fgetc(stdin);
        if (c == lexeme[0]) {
            lexeme[1] = '\0';
            return INCDEC;
        }
        ungetc(c, stdin);
        lexeme[1] = '\0';
        return ADDSUB;
    }
    else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_') {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || isdigit(c) || c == '_') && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    }
    else if (c == EOF) {
        return ENDFILE;
    }
    else if (c == '&') {
        strcpy(lexeme, "&");
        return AND;
    }
    else if (c == '|') {
        strcpy(lexeme, "|");
        return OR;
    }
    else if (c == '^') {
        strcpy(lexeme, "^");
        return XOR;
    }
    else {
        return UNKNOWN;
    }
}

void advance(void)
{
    curToken = getToken();
    if (isFirst && curToken != UNKNOWN) {
        isFirst = 0;
    }
}

int match(TokenSet token)
{
    if (curToken == UNKNOWN) advance();
    return token == curToken;
}

char *getLexeme(void) { return lexeme; }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int regc = 0;
int possym = 0;
int sbcount = 0;
int isFirst;

Symbol table[TBLSIZE];
BTNode *_left;

void initTable(void)
{
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0) {
            possym = i * 4;
            return table[i].val;
        }

    if (sbcount >= TBLSIZE) error(RUNOUT);
    error(NOTFOUND);
    return 0;
}

int setval(char *str, int val)
{
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE) error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode *)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root)
{
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void)
{
    BTNode *retp = NULL;
    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    }
    else if (match(INCDEC)) {
        // retp = makeNode(INCDEC, getLexeme());
        // advance();
        // if (match(ID)) {
        //     retp->right = makeNode(ID, getLexeme());
        //     advance();
        // }
        // else {
        //     error(NOTVAR);
        // }
        BTNode *OP = makeNode(ADDSUB, getLexeme());
        retp = makeNode(ASSIGN, "=");
        advance();
        if (match(ID)) {
            retp->left = makeNode(ID, getLexeme());
            retp->right = OP;
            retp->right->left = makeNode(ID, getLexeme());
            retp->right->right = makeNode(INT, "1");
            advance();
        }
        else {
            error(NOTVAR);
        }
    }
    else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    }

    else {
        error(NOTNUMID);
    }

    return retp;
}

// unary_expr := ADDSUB unary_expr | factor
BTNode *unary_expr(void)
{
    BTNode *retp = NULL;
    if (match(ADDSUB)) {
        // printf("X");
        int c = 0;
        if (c) {
            retp = makeNode(ADDSUB, getLexeme());
            advance();
        }
        else {
            int minusCnt = 0;
            while (match(ADDSUB)) {
                if (*getLexeme() == '-') minusCnt++;
                advance();
            }
            if (minusCnt % 2 == 1)
                retp = makeNode(ADDSUB, "-");
            else
                retp = makeNode(ADDSUB, "+");
        }

        retp->left = makeNode(INT, "0");
        retp->right = unary_expr();
        return retp;
    }
    else {
        return factor();
    }
}

// muldiv_expr := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void)
{
    if (match(MULDIV)) return muldiv_expr_tail(_left);

    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else {
        return left;
    }
}

// addsub_expr := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void)
{
    // STH with var
    if (!isFirst) {
        if (match(ADDSUB)) return addsub_expr_tail(_left);
    }
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else {
        return left;
    }
}

// and_expr := addsub_expr and_expr_tail | NiL
BTNode *and_expr(void)
{
    if (match(AND)) return and_expr_tail(_left);

    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

// and_expr_tail := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{
    BTNode *node = NULL;
    if (match(AND)) {
        node = makeNode(AND, getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    }
    else {
        return left;
    }
}

// xor_expr := and_expr xor_expr_tail
BTNode *xor_expr(void)
{
    if (match(XOR)) return xor_expr_tail(_left);

    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

// xor_expr_tail := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{
    BTNode *node = NULL;
    if (match(XOR)) {
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    }
    else {
        return left;
    }
}

// or_expr := xor_expr or_expr_tail
BTNode *or_expr(void)
{
    if (match(OR)) return or_expr_tail(_left);

    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

// or_expr_tail := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{
    BTNode *node = NULL;
    if (match(OR)) {
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else {
        return left;
    }
}

// assign_expr := ID ASSIGN assign_expr | or_expr

BTNode *assign_expr()
{
    isFirst = 1;
    _left = NULL;
    BTNode *node = NULL;
    if (match(ID)) {
        _left = makeNode(ID, getLexeme());
        advance();
        if (match(RPAREN)) return _left;
        if (match(END)) return _left;
        if (match(ASSIGN)) {
            node = makeNode(ASSIGN, getLexeme());
            advance();
            node->left = _left;
            node->right = assign_expr();
            return node;
        }
        else {
            return or_expr();
        }
    }
    else {
        return or_expr();
    }
}

// statement := ENDFILE | END | assign_expr END
void statement(void)
{
    BTNode *retp = NULL;
    regc = 0;
    if (match(ENDFILE)) {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }
    else if (match(END)) {
        advance();
    }
    else {
        retp = assign_expr();
        if (match(END)) {
            if (!PRINTERR) {
                evaluateTree(retp);
                freeTree(retp);
            }
            else {
                printf("%d\n", evaluateTree(retp));
                printf("Prefix traversal: ");
                printPrefix(retp);
                printf("\n");
                freeTree(retp);
                printf(">> ");
            }

            advance();
        }
        else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum)
{
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
            case NOTVAR:
                fprintf(stderr, "variable expected\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    printf("EXIT 1\n");
    exit(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int idcnt = 0;

int evaluateTree(BTNode *root)
{
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                idcnt++;
                retval = getval(root->lexeme);
                if (regc > MAXREG) {
                    printf("MOV [%d] r%d\n", sbcount << 2, regc % 8);
                    sbcount++;
                }
                printf("MOV r%d [%d]\n", regc % 8, possym);
                regc++;
                break;
            case INT:
                retval = atoi(root->lexeme);
                if (regc > MAXREG) {
                    printf("MOV [%d] r%d\n", sbcount << 2, regc % 8);
                    sbcount++;
                }
                printf("MOV r%d %d\n", regc % 8, retval);
                regc++;
                break;
            case ASSIGN:
                idcnt++;
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                getval(root->left->lexeme);
                // if (regc > MAXREG) {
                //     printf("MOV [%d] r%d\n", sbcount << 2, regc % 8);
                // }
                printf("MOV [%d] r%d\n", possym, (regc - 1) % 8);
                break;
            // case INCDEC:
            //     rv = evaluateTree(root->right);
            //     if (strcmp(root->lexeme, "+") == 0) {
            //         retval = setval(root->right->lexeme, rv + 1);

            //         printf("MOV r%d %d\n", regc%8, 1);
            //         printf("ADD r%d r%d\n", regc%8 - 1, regc%8);
            //     }
            //     if (strcmp(root->lexeme, "-") == 0) {
            //         retval = setval(root->right->lexeme, rv - 1);
            //         printf("MOV r%d %d\n", regc%8, 1);
            //         printf("SUB r%d r%d\n", regc%8 - 1, regc%8);
            //     }
            //     printf("MOV [%d] r%d\n", possym, regc%8 - 1);
            //     break;
            case MULDIV:
                if (strcmp(root->lexeme, "*") == 0) {
                    lv = evaluateTree(root->left);
                    rv = evaluateTree(root->right);
                    retval = lv * rv;
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                    printf("MUL r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                }
                else if (strcmp(root->lexeme, "/") == 0) {
                    lv = evaluateTree(root->left);
                    idcnt = 0;
                    rv = evaluateTree(root->right);
                    if (rv == 0) {
                        if (idcnt == 0) error(DIVZERO);
                    }
                    else
                        retval = lv / rv;
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                    printf("DIV r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                }
                regc--;
                break;
            case ADDSUB:
            case AND:
            case OR:
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                }
                else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                }
                else if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                    printf("AND r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                }
                else if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                    printf("OR r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                }
                else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                    printf("XOR r%d r%d\n", (regc - 2) % 8, (regc - 1) % 8);
                    if (regc > MAXREG) {
                        sbcount--;
                        printf("MOV r%d [%d]\n", (regc - 1) % 8, sbcount << 2);
                    }
                }
                regc--;
                break;
            default:
                retval = 0;
        }
    }

    return retval;
}

void printPrefix(BTNode *root)
{
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

/**
* TODO: Refactor these methods
statement        := END | assign_expr END
assign_expr      := ID ASSIGN assign_expr | or_expr
or_expr          := xor_expr or_expr_tail
or_expr_tail     := OR xor_expr or_expr_tail | NiL
xor_expr         := and_expr xor_expr_tail
xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
and_expr         := addsub_expr and_expr_tail
and_expr_tail    := AND addsub_expr and_expr_tail | NiL
addsub_expr      := muldiv_expr addsub_expr_tail
addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
muldiv_expr      := unary_expr muldiv_expr_tail
muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
unary_expr       := ADDSUB unary_expr | factor
factor           := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN

                                            */
int main()
{
    // freopen("out.txt", "w", stdout);
    initTable();
    if (PRINTERR) printf(">> ");
    while (1) {
        statement();
    }
    return 0;
}
