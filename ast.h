/*   Abstract syntax tree code


 Header file   
 Jared Kaiser Spring 2023

 Defines Types for the AST

*/

#include<stdio.h>
#include<malloc.h>

#include"symtable.h"

#ifndef AST_H
#define AST_H
int mydebug;

/* define the enumerated types for the AST.  THis is used to tell us what 
sort of production rule we came across */

enum ASTtype {
   A_FUNCTIONDEC, //1
   A_VARDEC,  //2
   A_COMPOUND, //3
   A_WRITE, //4
   A_NUM, //5
   A_EXPR, //6
   A_MOD, //7
   A_DIV, //8
   A_MULT, //9
   A_GE, //10
   A_GT, //11
   A_LT, //12
   A_LE,  //13
   A_EQ,  //14
   A_NE,  //15
   A_ASSIGN,  //16
   A_PARAM,  //17
   A_IF,  //18
   A_READ, //19
   A_WHILE,  //20
   A_RETURN,  //21
   A_ELSE, //22 
   A_VAR, //23
   A_CALL, //24
   A_ARGS, //25
   A_EXPRSTMT //26
};

// Math Operators

enum AST_OPERATORS {
   A_PLUS,
   A_MINUS,
   A_TIMES,
   A_UMINUS
};

enum AST_MY_DATA_TYPE {
   A_INTTYPE,
   A_VOIDTYPE

};

/* define a type AST node which will hold pointers to AST structs that will
   allow us to represent the parsed code 
*/

typedef struct ASTnodetype
{
     enum ASTtype type;
     enum AST_OPERATORS operator;
     char * name;
     char * label;
     int value;
     enum AST_MY_DATA_TYPE my_data_type;
     struct SymbTab * symbol;
     struct ASTnodetype *s1,*s2, *next ; /* used for holding IF and WHILE components -- not very descriptive */
} ASTnode;


/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
ASTnode *ASTCreateNode(enum ASTtype mytype);

void PT(int howmany);


/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p);

// parameter checking
int Check_Params(ASTnode *actuals, ASTnode *formals);

#endif // of AST_H
