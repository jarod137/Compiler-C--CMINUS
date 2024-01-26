%{

/*
            CMINUS BNF YACC implemenntation
            taken from the PDF provided by CS 370 class instructor
            left recursion is made on recurssive definitions
            except for expressions

            Added symbol table and type checking functionality

            Jared Kaiser
            Spring 2023
*/

	/* begin specs */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "emit.h"
#include "ast.h" 
#include "symtable.h"

ASTnode *PROGRAM;

int yylex();
extern int lineno; // From LEX
extern int mydebug; // From LEX

int LEVEL = 0; // global variable to keep track of the level
int OFFSET = 0; // global variable for accumulating needed runtime space
int GOFFSET =0; // global varibale for accumulation global variable offset
int maxoffset =0; // the largest offset needed for the current function


void yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  printf ("YACC PARSE ERROR: %s on line %d\n", s, lineno);
}

 
%} 
/*  defines the start symbol, what values come back from LEX and how the operators are associated  */

%union {
		int value;
		char * string;
          ASTnode *node;
          enum AST_MY_DATA_TYPE d_type;
          enum AST_OPERATORS operator;
}

%start Program

%token  <value> T_NUM
%token  <string> T_ID T_STRING
%token T_INT
%token T_VOID
%token T_READ T_WRITE T_WHILE T_RETURN T_IF T_ELSE 
%token T_GT T_GE T_LT T_LE T_EQ T_NE
%token T_PLUS T_MINUS T_MULT T_MOD

%type <node> Declaration_List Declaration Var_Declaration Var_List
%type <node> Fun_Declaration Params Compound_Stmt Local_Declarations Statement_List
%type <node> Statement Write_Stmt Expresion Simple_Expression Additive_Expression Term Expresion_Stmt
%type <node> Factor Assignment_Stmt Param_List Param Selection_Stmt Iteration_Stmt Return_Stmt Var Read_Stmt Args Call Arg_List

%type <d_type> Type_Specifier
%type <operator> Addop Multop Relop

%%	/* end specs, begin rules */

Program	:	Declaration_List { PROGRAM = $1; }
          ;

Declaration_List  :  Declaration { $$ = $1; }
                  |  Declaration Declaration_List
                  {
                    $$ = $1;
                    $$ -> next = $2;
                  }
                  ;
        
Declaration  :  Var_Declaration  { $$ = $1; }
             |  Fun_Declaration  { $$ = $1; }
             ;

Var_Declaration  :  Type_Specifier Var_List ';'
                 { 
                      // populate the s1 connected list with the defined type via $1
                      ASTnode *p = $2;
                      while( p != NULL )
                       {
                         p->my_data_type = $1;

                         // check each variable in the list to see if it has been defined at our level
                         if (Search(p->name, LEVEL, 0)!=NULL)
                         {    // symbol already defined -- BARF
                              yyerror(p->name);
                              yyerror("Symbol already defined");
                              exit(1);
                         }
                         // It was not in the symbol table
                         if (p->value == 0)
                         {    // we have a scalar
                              p->symbol = Insert(p->name, p->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                              OFFSET++;
                         }
                         else
                         {    // we have an array
                              p->symbol = Insert(p->name, p->my_data_type, SYM_ARRAY, LEVEL, p->value, OFFSET);
                              OFFSET = OFFSET + p->value;
                         }

                         p = p->s1;
                         
                       }
                      $$ = $2;
                 }
                 ;

Var_List  :  T_ID    
          {
             $$=ASTCreateNode(A_VARDEC);
             $$->name = $1;
          }
          |  T_ID '[' T_NUM ']' 
          {
               $$=ASTCreateNode(A_VARDEC);
               $$->name=$1;
               $$->value=$3;
          }
          |  T_ID ',' Var_List  
          {
               $$=ASTCreateNode(A_VARDEC);
               $$->name = $1;
               $$->s1 = $3;
          }
          |  T_ID '[' T_NUM ']' ',' Var_List 
          {
               $$=ASTCreateNode(A_VARDEC);
               $$->name=$1;
               $$->value=$3;
               $$->s1 = $6;
          } 
          ;
           
Type_Specifier  :  T_INT { $$ = A_INTTYPE; }  
                |  T_VOID { $$ = A_VOIDTYPE; }
                ;

Fun_Declaration  :  Type_Specifier T_ID 
                 { // check to see if fution has been defined
                    if(Search($2,LEVEL,0) != NULL)
                    { // ID has already been used -- BARF
                         yyerror($2);
                         yyerror("function name already in use");
                         exit(1);
                    }
                    // not in symbol table, install it

     
                    Insert($2,$1,SYM_FUNCTION,LEVEL,0,maxoffset);

                    GOFFSET = OFFSET;
                    OFFSET = 2;
                    maxoffset = OFFSET;
                 }
                 '(' Params ')' 
                 { Search($2,LEVEL,0)->fparms = $5; }
                 Compound_Stmt  
                 {
                      $$ = ASTCreateNode(A_FUNCTIONDEC);
                      $$->name = $2;
                      $$->my_data_type = $1;
                      $$->s1 = $5;
                      $$->s2 = $8;
                      $$->symbol = Search($2,LEVEL,0);
                      $$->symbol->offset = maxoffset;
                      
                      OFFSET-=Delete(1);  /* remove all the symbols from what we put in from the function call*/
                      LEVEL=0;  /* reset the level */
                      OFFSET = GOFFSET; // resets the offset for global variables
                 }
                 ;

Params  : T_VOID  { $$ = NULL; } // STAYS NULL
        | Param_List { $$ = $1; }
        ;
      
Param_List : Param  { $$ = $1; }
           | Param ',' Param_List  
           {
                $$ = $1;
                $$->next = $3;
           }
           ;

Param : Type_Specifier T_ID  
     { 
          if(Search($2,LEVEL+1,0) != NULL) 
          {
               yyerror($2);
               yyerror("Parameter already used");
               exit(1);
          }

          $$ = ASTCreateNode(A_PARAM);
          $$->name = $2;
          $$->my_data_type = $1; 
          $$->symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL+1, 1, OFFSET);
          OFFSET++;
     }
     | Type_Specifier T_ID '[' ']'  
     { 
          if(Search($2,LEVEL+1,0) != NULL) 
          {
               yyerror($2);
               yyerror("Parameter already used");
               exit(1);
          }

          $$ = ASTCreateNode(A_PARAM);
          $$->name = $2;
          $$->my_data_type = $1; 
          $$->value = -1;
          $$->symbol = Insert($$->name, $$->my_data_type, SYM_ARRAY, LEVEL+1, 1, OFFSET);
          OFFSET++;
          
     }
     ;

Compound_Stmt  : '{' { LEVEL++; } Local_Declarations Statement_List '}' 
               { 
                    $$ = ASTCreateNode(A_COMPOUND); 
                    $$->s1 = $3;
                    $$->s2 = $4;
                    if(mydebug == 1) Display();
                 // we set the max offset
                    if (OFFSET > maxoffset) maxoffset = OFFSET;
                    OFFSET -= Delete(LEVEL);
                    LEVEL--;
               }
               ;

Local_Declarations : /* empty */ { $$ = NULL; } // STAYS NULL
                   | Var_Declaration Local_Declarations 
                   { 
                         $$ = $1;
                         $$->next = $2;
                   }
                   ;

Statement_List :  /* empty */  { $$ = NULL; } // STAYS NULL
               |  Statement Statement_List
               { 
                    $$ = $1;
                    $$->next = $2;
               }
               ;
            
Statement  : Expresion_Stmt { $$ = $1; }
           | Compound_Stmt { $$ = $1; }
           | Selection_Stmt { $$ = $1; }
           | Iteration_Stmt { $$ = $1; }
           | Assignment_Stmt { $$ = $1; } 
           | Return_Stmt { $$ = $1; }
           | Read_Stmt { $$ = $1; }
           | Write_Stmt { $$ = $1; }
           ;

Expresion_Stmt : Expresion ';' 
               { 
                    $$ = ASTCreateNode(A_EXPRSTMT);
                    $$->s1 = $1; 
               }
               | ';'  
               { 
                    $$ = ASTCreateNode(A_EXPRSTMT);
                    $$->s1 = NULL; 
               }
               ;

Selection_Stmt : T_IF '(' Expresion ')' Statement  
               {
                   $$=ASTCreateNode(A_IF);
                   $$->s1 = $3;
                   $$->s2 = ASTCreateNode(A_ELSE);
                   $$->s2->s1 = $5;
                   $$->s2->s2 =NULL;                      
               }
               | T_IF '(' Expresion ')' Statement  T_ELSE Statement 
               {
                   $$=ASTCreateNode(A_IF);
                   $$->s1 = $3;
                   $$->s2 = ASTCreateNode(A_ELSE);
                   $$->s2->s1 = $5;
                   $$->s2->s2 =$7;
               }
               ;

Iteration_Stmt : T_WHILE '(' Expresion ')' Statement  
               {
                    $$ = ASTCreateNode(A_WHILE);
                    $$->s1 = $3;
                    $$->s2 = $5; 
               }
               ;

Return_Stmt : T_RETURN ';'  
            {
               $$ = ASTCreateNode(A_RETURN);
            }
            | T_RETURN Expresion ';'
            {
               $$ = ASTCreateNode(A_RETURN);
               $$->s1= $2;
            }
            ;

Read_Stmt : T_READ  Var  ';'  
          {
               $$ = ASTCreateNode(A_READ);
               $$->s1 = $2;
               $$->next = $2;
          }
          ;  

Write_Stmt : T_WRITE Expresion ';' 
           { 
               $$ = ASTCreateNode(A_WRITE);
               $$->s1 = $2;
           }
           | T_WRITE T_STRING ';' 
           {
               $$ = ASTCreateNode(A_WRITE);
               $$->name = $2;
           }
           ;

Assignment_Stmt : Var '=' Simple_Expression ';' // include type checking
                {
                     if ($1->my_data_type != $3->my_data_type) {
                         yyerror("Type does not match");
                         exit(1);
                     }
                     $$ = ASTCreateNode(A_ASSIGN);
                     $$->s1 = $1;
                     $$->s2 = $3;
                     $$->name = CreateTemp();
                     $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                     OFFSET++;
                     
                     // Create temp here      
                    
                }

Var : T_ID  
    { 
          struct SymbTab *p;
          p = Search($1, LEVEL, 1);
          if (p == NULL)
          { // a refecence variable not in symbol table
               yyerror($1);
               yyerror("symbol table used but not defined");
               exit(1);
          }

          if (p->SubType != SYM_SCALAR)                                
          { // a reference to a non SCALAR variable
               yyerror($1);
               yyerror("symbol used must be a SCALAR");
               exit(1);
          }
          $$ = ASTCreateNode(A_VAR);
          $$->name = $1;
          $$->symbol = p;
          $$->my_data_type = p->Declared_Type;
    }
    | T_ID '[' Expresion ']' 
    {                                      
          struct SymbTab *p;
          p = Search($1, LEVEL, 1);
          if (p == NULL)
          { // a refecence variable not in symbol table
               yyerror($1);
               yyerror("symbol table used but not defined");
               exit(1);
          }

          if (p->SubType != SYM_ARRAY) 
          { // a reference to a non ARRAY variable
               yyerror($1);
               yyerror("symbol used must be a ARRAY");
               exit(1);
          }

          $$ = ASTCreateNode(A_VAR);
          $$->name = $1;
          $$->symbol = p;
          $$->my_data_type = p->Declared_Type;
          $$->s1 = $3;
    }
    ;
  
Expresion : Simple_Expression { $$ = $1; }
          ;

Simple_Expression : Additive_Expression { $$ = $1; }
                  | Additive_Expression Relop Additive_Expression   
                  { 

                    if($1->my_data_type != $3->my_data_type) 
                       {
                         yyerror("Type Mismatch ");
                         exit(1);
                       }

                    $$ = ASTCreateNode(A_EXPR);
                    $$->s1 = $1;
                    $$->s2 = $3;
                    $$->operator = $2;
                    $$->my_data_type = $1->my_data_type;
                    $$->name = CreateTemp(); // Creates a temp Expression
                    $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                    OFFSET++;
                  }
                  ;
                
Relop : T_GE  { $$ = A_GE; }
      | T_GT  { $$ = A_GT; }
      | T_LT  { $$ = A_LT; }
      | T_LE  { $$ = A_LE; }
      | T_EQ  { $$ = A_EQ; }
      | T_NE  { $$ = A_NE; }
      ;

Additive_Expression : Term 
                    { $$ = $1; } 
                    | Additive_Expression Addop Term 
                    { 

                       if($1->my_data_type != $3->my_data_type) 
                       {
                         yyerror("Type Mismatch ");
                         exit(1);
                       }

                       $$ = ASTCreateNode(A_EXPR);
                       $$->s1 = $1;
                       $$->s2 = $3; 
                       $$->operator = $2;
                       $$->my_data_type = $1->my_data_type;
                       $$->name = CreateTemp(); // Creates a temp Expression
                       $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                       OFFSET++;

                    }
                    ;
                  
Addop : T_PLUS  { $$ = A_PLUS; }
      | T_MINUS { $$ = A_MINUS; }
      ;

Term : Factor { $$ = $1; }
     | Term Multop Factor 
     { 

         if($1->my_data_type != $3->my_data_type) 
          {
            yyerror("Type Mismatch ");
            exit(1);
          }

         $$ = ASTCreateNode(A_EXPR); 
         $$->s1 = $1;
         $$->s2 = $3;
         $$->operator = $2;
         $$->my_data_type = $1->my_data_type;
         $$->name = CreateTemp(); // Creates a temp Expression
         $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
         OFFSET++;

     }
     ;

Multop : T_MULT { $$ = A_MULT; }
       | '/'  { $$ = A_DIV; }
       | T_MOD  { $$ = A_MOD; }
       ;

Factor : '(' Expresion ')' { $$ = $2; }                            
       | T_NUM 
       {
          $$ = ASTCreateNode(A_NUM);
          $$->value = $1;                              
          $$->my_data_type = A_INTTYPE;
       }
       | Var 
       { $$ = $1; }
       | Call 
       { $$ = $1; }
       | T_MINUS Factor 
       { 
         $$ = ASTCreateNode(A_EXPR);
         $$->operator = A_UMINUS;
         $$->s1 = $2;   
         $$->my_data_type = $2->my_data_type;
         $$->name = CreateTemp();  // Creates a temp Expression
         $$->symbol = Insert($$->name, $2->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
         OFFSET++;                          
       }
       ;

Call : T_ID '(' Args ')' {}
     {    
          struct SymbTab *p;
          p=Search($1,0,0);
          
          if(p==NULL) 
          {
               // function name not known
               yyerror($1);
               yyerror("function name not defined");
               exit(1);
          }
          // name is there but is it a FUNCTION?

          if (p->SubType != SYM_FUNCTION) {
               // function name is not defined as a funtion
               yyerror($1);
               yyerror("function name not defined as a funtion");
               exit(1);
          }

          // check to see thath the formal and actual parameters are same length and type
          if (Check_Params($3, p->fparms) == 0)
          {
               yyerror($1);
               yyerror("Actual and Formals do not match");
               exit(1);
          }


          $$ = ASTCreateNode(A_CALL);
          $$->name = $1;
          $$->s1 = $3;
          $$->symbol = p;
          $$->my_data_type = p->Declared_Type;
          
     }
     ;
    
Args : Arg_List 
     { $$ = $1; }
     | /* empty */ 
     { $$ = NULL; }
     ;

Arg_List : Expresion 
         {
          $$ = ASTCreateNode(A_ARGS); 
          $$->s1=$1; 
          $$->my_data_type = $1->my_data_type;
          $$->name = CreateTemp(); // Creates a temp to store individual arguments
          $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
          OFFSET++;

         }
         | Expresion ',' Arg_List
         {
          $$ = ASTCreateNode(A_ARGS); 
          $$->s1=$1;
          $$->next = $3;
          $$->my_data_type = $1->my_data_type;
          $$->name = CreateTemp();  // Creates a temp to store individual arguments
          $$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
          OFFSET++;
         }
         ;

%%	/* end of rules, start of program */

int main(int argc, char *argv[])
{
     FILE *fp;
     // read our input arguments
     int i;
     char s[100];

     // option -d turn on debug
     // option -o next argument is our output file name
     for (int i = 0; i<argc; i++) 
     {
          if (strcmp(argv[i], "-d") == 0 ) mydebug = 1;
          if (strcmp(argv[i], "-o") == 0 )
          {
               // we have an input file
               strcpy(s,argv[i+1]);
               strcat(s,".asm");
               printf(" File name is %s\n",s);
          }
          printf("%s\n", argv[i]);
     }

     // now open the file that is referenced by s

     fp = fopen(s, "w");
     if (fp == NULL) 
          {
               printf(" cannot open file %s\n", s);
               exit(1);
          }
	 yyparse();
     // printf("\nFinished Parsing\n\n\n");
      // we know thaat global variable PROGRAM has a PTR to the top of the tree
     if(mydebug == 1) Display();
     if(mydebug == 1) printf("\n\nAST PRINT \n\n");
     if(mydebug == 1) ASTprint(0,PROGRAM);

      EMIT(PROGRAM, fp);
}
