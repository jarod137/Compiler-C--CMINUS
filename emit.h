// EMIT.h
// Spring 2023
// Jared Kaiser
//
// Header Function
//
// interface file for other etities to know about available functions
//
// provides connesctivity to MIPS generating subroutines

#ifndef EMIT_H
#define EMIT_H
#include "ast.h"

#define WSIZE 4
#define LOG_WSIZE 2

void EMIT_STRINGS(ASTnode*p, FILE*fp);
void EMIT_GLOBALS(ASTnode*p, FILE*fp);
void emit(FILE *fp, char * label, char * command, char * comment);
void emit_expr(ASTnode *p, FILE *fp);
void emit_write(ASTnode *p, FILE *fp);
void emit_read(ASTnode *p, FILE *fp);
void emit_var(ASTnode *p, FILE *fp) ;
void emit_function(ASTnode *p, FILE *fp); 
void EMIT_AST(ASTnode * p, FILE *fp);
void EMIT(ASTnode* p, FILE* fp);

#endif
