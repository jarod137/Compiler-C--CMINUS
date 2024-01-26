/*   Abstract syntax tree code

    This code is used to define an AST node, 
    routine for printing out the AST
    defining an enumerated type so we can figure out what we need to
    do with this.  The ENUM is basically going to be every non-terminal
    and terminal in our language.

    Added funtion to type check function calls

    Jared Kaiser Spring 2023

*/

#include<stdio.h>
#include<malloc.h>
#include "ast.h" 


/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
//  PRE:  Ast Node Type
//  POST:   PTR To heap memory and ASTnode set and all other pointers set to NULL
ASTnode *ASTCreateNode(enum ASTtype mytype)

{
    ASTnode *p;
    if (mydebug) fprintf(stderr,"Creating AST Node \n");
    p=(ASTnode *)malloc(sizeof(ASTnode));
    p->type=mytype;
    p->s1=NULL;
    p->s2=NULL;
    p->next=NULL;
    p->value=0;
    return(p);
}

/*  Helper function to print tabbing */
//PRE:  Number of spaces desired
//POST:  Number of spaces printed on standard output

void PT(int howmany)
{
	 for(int i=0; i<howmany; i++) {
    printf(" ");
   }
}

//  PRE:  A declaration type
//  POST:  A character string that is the name of the type
//          Typically used in formatted printing
char * ASTtypeToString(enum AST_MY_DATA_TYPE mytype)
{
    switch (mytype) {

        case A_INTTYPE:
              return "INT";
        case A_VOIDTYPE:
              return "VOID";
        default :
              return "unknown data type";

    }
}



/*  Print out the abstract syntax tree */
// PRE:   PRT to an ASTtree
// POST:  indented output using AST order printing with indentation

void ASTprint(int level,ASTnode *p)
{
   int i;
   if (p == NULL ) return;
  
  switch (p->type) {
    case A_VARDEC : 
        PT(level); 
        printf("Variable %s %s",  
            ASTtypeToString(p->my_data_type), 
            p->name);
        if (p->value)
            {
                printf("[%d]",p->value);
            }
        printf(" level %d offset %d",p->symbol->level ,p->symbol->offset);
        printf("\n");
        ASTprint(level+1, p->s1);
        ASTprint(level, p->next);
        break;

    case A_FUNCTIONDEC :  
        PT(level);  
        printf("Function %s %s level %d offset %d",
            ASTtypeToString(p->my_data_type),
            p->name,
            p->symbol->level,
            p->symbol->offset);
        printf("\n");
        ASTprint(level + 1 , p->s1); // parameters
        ASTprint(level + 1, p->s2); // compound
        ASTprint(level , p->next);
        break;

    case A_PARAM :
        PT(level); printf("Parameter %s %s level %d offset %d", 
            ASTtypeToString(p->my_data_type), 
            p->name,
            p->symbol->level,
            p->symbol->offset);
        if (p->value == -1)
            printf("[]");
        printf("\n");
        ASTprint(level , p->next);
        break;

    case A_COMPOUND : 
        PT(level);  printf("Compound Statement\n");
        ASTprint(level + 1 , p->s1); // local decs
        ASTprint(level + 1, p->s2); // statement list
        ASTprint(level , p->next);
        break;

    case A_EXPRSTMT : 
        PT(level); printf("Expression STATEMENT\n");
        ASTprint(level+1, p->s1);
        ASTprint(level , p->next);
        break;

    case A_IF : 
        PT(level); printf("IF STATEMENT\n");
        PT(level+1); printf("IF Expression\n");
        ASTprint(level + 1, p->s1);
        if (p->s2->s2 == NULL)
            {
            PT(level); printf("IF body\n");
            ASTprint(level + 1, p->s2->s1);
            }
        if (p->s2->s2 != NULL)
            {
            PT(level); printf("IF body\n");
            ASTprint(level + 1, p->s2->s1);
            PT(level); printf("ELSE body\n");
            ASTprint(level+1, p->s2->s2);
            }
        ASTprint(level , p->next);
        break;

    case A_WHILE : 
        PT(level); printf("WHILE STATEMENT\n");
        PT(level+1); printf("WHILE Expression\n");
        ASTprint(level + 1, p->s1);
        PT(level+1); printf("WHILE Body\n");
        ASTprint(level + 1, p->s2);
        ASTprint(level , p->next);
        break;

    case A_ASSIGN : 
        PT(level); printf("ASSIGNMENT STATEMENT \n");
        ASTprint(level + 1, p->s1);
        PT(level); printf("is assigned  \n");
        ASTprint(level + 1, p->s2);
        ASTprint(level , p->next);
        break;

    case A_RETURN :
        PT(level); printf("RETURN STATEMENT\n");
        ASTprint(level+1, p->s1);
        ASTprint(level , p->next);
        break;

    case A_READ : 
        PT(level); printf("READ STATEMENT\n");
        if (p->s1 != NULL)
            ASTprint(level+1, p->s1);
        break;

    case A_WRITE : 
        PT(level);  
        if (p->name != NULL) 
        {
            printf("Write String  %s\n", p->name);
        }
        else // it is an expression
        {
            printf("Write Expression\n");
            ASTprint(level + 1, p->s1);
        }
        ASTprint(level , p->next);
        break;

    case A_VAR : 
        PT(level); printf("VARIABLE %s level %d offset %d\n", p->name,
                        p->symbol->level,
                        p->symbol->offset );
        if (p->s1 != NULL){
            PT(level+1); printf("[\n");
            ASTprint(level+2, p->s1);
            PT(level+1); printf("]\n");
        }
        break;

    case A_EXPR : 
        PT(level); printf("EXPRESSION operator "); 
        switch(p->operator) {
            case A_PLUS : printf("+\n"); break;
            case A_MINUS : printf("-\n"); break;
            case A_DIV : printf("/\n"); break;
            case A_MULT : printf("*\n"); break;
            case A_MOD : printf("%\n"); break;
            case A_EQ : printf("==\n"); break;
            case A_NE : printf("!=\n"); break;
            case A_LE : printf(">=\n"); break;
            case A_LT : printf(">\n"); break;
            case A_GE : printf("<=\n"); break;
            case A_GT : printf("<\n"); break;
            case A_UMINUS : printf("Unary Minus\n"); break;
            default : printf("unknown operator in A_EXPR in ASTprint\n"); break;
        }    // of switch
        ASTprint(level+1, p->s1);
        ASTprint(level+1, p->s2);
        break;

    case A_NUM : 
        PT(level); printf("NUMBER value %d \n", p->value); 
        break;

    case A_CALL : 
        PT(level); printf("CALL Statement function %s\n", p->name);
        PT(level); printf("(\n");
        ASTprint(level+1, p->s1);
        PT(level); printf(")\n");
        break;

    case A_ARGS : 
        PT(level); printf("CALL Argument\n");
        ASTprint(level+1, p->s1);
        ASTprint(level, p->next);
        break;

default: printf("unknown AST Node type %d in ASTprint\n", p->type);
} // of switch

} // of ASTprint

// PRE: PTRS to actual and formals
// POST: 0 if they are not the same type or length
//       1 if they are
int Check_Params(ASTnode *actuals, ASTnode *formals)
{
                                                       // DOES NOT AWAY
    if(actuals == NULL && formals == NULL)
        return 1;
    if(actuals == NULL || formals == NULL)
        return 0;
    if(actuals->my_data_type != formals->my_data_type)
        return 0;
    Check_Params(actuals->next, formals->next);
}
