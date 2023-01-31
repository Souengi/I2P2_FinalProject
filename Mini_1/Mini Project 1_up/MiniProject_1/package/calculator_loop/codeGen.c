#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
int regis = -1;
int nid = 0;
int newv = 0;
int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                nid++;
                regis++;
                printf("MOV r%d, [%d]\n", regis, getmem(root->lexeme));
                break;
            case INT:
                retval = atoi(root->lexeme);
                regis++;
                printf("MOV r%d, %d\n", regis, retval);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                printf("MOV [%d], r%d\n", getmem(root->left->lexeme), regis);
                break;
            case ADDSUB:
            case MULDIV:
                nid = 0;
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD r%d, r%d\n", regis-1, regis);
                    regis--;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d, r%d\n", regis-1, regis);
                    regis--;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    printf("MUL r%d, r%d\n", regis-1, regis);
                    regis--;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv == 0){
                        if(nid == 0){
                            error(DIVZERO);
                        }else{
                            rv = 1;
                            lv = 0;
                        }
                    }
                    retval = lv / rv;
                    printf("DIV r%d, r%d\n", regis-1, regis);
                    regis--;
                }
                break;
            /*Extend for Project*/
            case INCDEC:
                if(root->right->data != ID){
                    error(SYNTAXERR);
                }
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "++") == 0) {
                    retval = ++rv;
                    printf("ADD [%d], 1\n", getmem(root->right->lexeme));
                } else if (strcmp(root->lexeme, "--") == 0) {
                    retval = --rv;
                    printf("SUB [%d], 1\n", getmem(root->right->lexeme));
                }
                break;
            case OR:
            case XOR:
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                    printf("OR r%d, r%d\n", regis-1, regis);
                    regis--;
                } else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                    printf("XOR r%d, r%d\n", regis-1, regis);
                    regis--;
                } else if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                    printf("AND r%d, r%d\n", regis-1, regis);
                    regis--;
                }
                break;
            default:
                retval = 0;
        }
        if(newv>0){
            error(SYNTAXERR);
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
