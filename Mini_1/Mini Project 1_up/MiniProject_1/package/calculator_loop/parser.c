#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.c"

int sbcount = 0;
Symbol table[TBLSIZE];

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
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    newv++;
    return 0;
}

/*Extend for get address*/
int getmem(char *str){
    int i=0;

    for (i = 0; i < sbcount; i++){
        if (strcmp(str, table[i].name) == 0)
            return i*4;
    }
}

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
    newv--;
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
            evaluateTree(retp);
            //printf("%d\n", evaluateTree(retp));
            //printf("Prefix traversal: ");
            //printPrefix(retp);
            //printf("\n");
            //printf("Assembly check\n");
            freeTree(retp);
            regis = -1;
            newv=0;
            advance();
        }else{
            error(SYNTAXERR);
        }
    }
}

// assign_expr := ID ASSIGN assign_expr | or_expr 
// change to := 
BTNode *assign_expr(void){
    //printf("In assign\n");
    BTNode *retp = NULL, *left = NULL;
    
    left = or_expr();
    if(left->data == ID){
        //left = or_expr;
        //left = makeNode(ID, getLexeme());
        //advance();
        if(match(ASSIGN)){
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
        } 
        else {
            retp = left;
            //error(SYNTAXERR);
        }
    } else {
        retp = left;
    } 
//    else{
//        error(NOTNUMID);
//    }
    return retp;
}

// or_expr          := xor_expr or_expr_tail 
// or_expr_tail     := OR xor_expr or_expr_tail | NiL
BTNode *or_expr(void){
    //printf("In or\n");
    BTNode *retp = NULL, *left = NULL;
    
    retp = left = xor_expr();
    while (match(OR))
    {
        retp = makeNode(OR, getLexeme());
        advance();
        retp->left = left;
        retp->right = xor_expr();
        left = retp;
    }
    return retp;
} 

//xor_expr         := and_expr xor_expr_tail 
//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL 
BTNode *xor_expr(void){
    //printf("In xor\n");
    BTNode *retp=NULL, *left=NULL;

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
}

//and_expr         := addsub_expr and_expr_tail 
//and_expr_tail    := AND addsub_expr and_expr_tail | NiL 
BTNode *and_expr(void){
    //printf("In and\n");
    BTNode *retp=NULL, *left=NULL;

    retp = left = addsub_expr();
    while(match(AND)){
        retp = makeNode(AND, getLexeme());
        advance();
        retp->left = left;
        retp->right = addsub_expr();
        left = retp;
    }
    return retp;
}

// addsub_expr      := muldiv_expr addsub_expr_tail 
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL 
BTNode *addsub_expr(void){
    //printf("In addsub\n");
    BTNode *retp = NULL, *left = NULL;

    retp = left = muldiv_expr();
    while (match(ADDSUB))
    {
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = muldiv_expr();
        left = retp;
    }
    return retp;
}

// muldiv_expr      := unary_expr muldiv_expr_tail 
// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL 
BTNode *muldiv_expr(void){
    //printf("In muldiv\n");
    BTNode *retp = NULL, *left = NULL;

    retp = left = unary_expr();
    while (match(MULDIV))
    {
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = unary_expr();
        left = retp;
    }
    return retp;
}

// unary_expr  := ADDSUB unary_expr | factor
BTNode *unary_expr(void){
    //printf("In unary\n");
    BTNode *retp = NULL, *left = NULL;
    
    left = Efactor();
    if(match(ADDSUB)){
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = unary_expr();
    } else{
        retp = left;
    }
    
    return retp;
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *Efactor(void){
    //printf("In fac\n");
    BTNode *retp = NULL, *left = NULL;

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
