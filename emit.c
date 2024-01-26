// emit.c
// program that emits MIPS code from test CMINUS code
// Jared Kaiser
// Spring 2023
// Grading Level e

#include <string.h>
#include <stdlib.h>

#include "ast.h"
#include "emit.h"

// internal prototypes
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
void emit_assign(ASTnode *p, FILE *fp);
void emit_while(ASTnode *p, FILE *fp);
void emit_call(ASTnode *p, FILE *fp);
void emit_return(ASTnode *p, FILE *fp);
void emit_params(ASTnode *p, FILE *fp);
void emit_if(ASTnode *p, FILE *fp);
void emit_args(ASTnode *p, FILE *fp);
void emit_exprstmt(ASTnode *p, FILE *fp);
void emit_argStore(ASTnode *p, FILE *fp);

// Global variables
int LNUM = 0;
int countParam=0;
int countARG = 0;


// PRE:   Assume one up Label LNUM
// POST:  Returns string with the format _L%d and increments the global variable

char * CreateLabel()
{    // Creates Label to help with code
    char hold[100];
    char *s;
    sprintf(hold,"_L%d",LNUM++);
    s=strdup(hold);
    return (s);
}

// PRE: PTR to top of AST, and FILE ptr to print to
// POST: Adds a labell into the AST for use string in write statements

void EMIT_STRINGS(ASTnode*p, FILE*fp)
{
    if(p==NULL) return;
    if (fp==NULL) return;

    if( p->name != NULL && p->type == A_WRITE) 
    {
        p->label = CreateLabel(); // creates new label
        fprintf(fp, "%s: .asciiz\t %s\n",p->label, p->name);
    }

    EMIT_STRINGS(p->s1, fp); 
    EMIT_STRINGS(p->s2, fp); 
    EMIT_STRINGS(p->next, fp);
}

// PRE: PTR to top of AST, and FILE ptr to print to
// POST: prints MIPS based global variables into file

void EMIT_GLOBALS(ASTnode*p, FILE*fp)
{
    if (p==NULL) return;
    if (fp==NULL) return;

    if (p->type == A_VARDEC && p->symbol->level == 0) 
        fprintf(fp, "%s: .space %d # global variable\n",p->symbol->name,p->symbol->mysize*WSIZE);

    EMIT_GLOBALS(p->s1,fp); 
    EMIT_GLOBALS(p->next, fp);
}

// PRE: possible label, command, comment
// POST: formatted output to the file

void emit(FILE *fp, char * label, char * command, char * comment)
{
    if(strcmp("",comment) == 0)
        if(strcmp("",label)==0)
            fprintf(fp,"\t%s\t\t\n",command);
        else
            fprintf(fp,"%s:\t%s\t\t\n",label,command);
        else
            if(strcmp("",label)==0)
                fprintf(fp,"\t%s\t\t# %s\n",command,comment);
            else
                fprintf(fp,"%s:\t%s\t\t# %s\n",label,command,comment);
} // emit

// PRE: PTR to expression family
// POST: MIPS code that sets $a0 to the value of the expression

void emit_expr(ASTnode *p, FILE *fp)
{
    char s[100];
    // base cases
    switch(p->type) {
        case A_NUM: // $a0 is the memory location
                sprintf(s,"li $a0, %d",p->value);
                emit(fp,"",s,"expression is a constant");
                return;
                break;
        case A_VAR:
                emit_var(p,fp); // $a0 is the memory location
                emit(fp,"","lw $a0, ($a0)", "Expression is a VAR");
                return;
                break;
        case A_CALL:
                emit_call(p,fp); // use helper function
                return;
                break;
        case A_EXPR: break; // handled after switch

        // should never be here
        default: printf("emit_expr switch NEVER SHOULD BE HERE\n");
                 printf("FIX FIX FIX\n");
                 exit(1);
    }

        // we know that we have A_EXPR Here
        if(p->operator == A_UMINUS)
        { // handle UMINUS first because it does not have an s2 connection
            emit_expr(p->s1,fp);
            emit(fp,"","neg $a0, $a0","Switch the sign to implement unary minus");       
        }
        else 
        {
            // sets up all operator funcitons except UMINUS which has allready been handled
            emit_expr(p->s1, fp);
            sprintf(s,"sw $a0, %d($sp)",p->symbol->offset*WSIZE);
            emit(fp,"",s,"expression store LHS temporarily");
            emit_expr(p->s2, fp);
            emit(fp,"","move $a1, $a0", "right hand side needs to be a1");
            sprintf(s,"lw $a0, %d($sp)",p->symbol->offset*WSIZE);
            emit(fp,"",s,"expression restore LHS from memory");
            switch (p->operator) // handles code for math operators
            {
            case A_PLUS: // handles plus operator
                emit(fp,"","add $a0, $a0, $a1","EXPR +");
                return;
                break;
            case A_MINUS: // handles minus operator
                emit(fp,"","sub $a0, $a0, $a1","EXPR -");
                return;
                break;
            case A_MULT: // handles multpilication operator 
                emit(fp,"","mult $a0 $a1","EXPR *");
                emit(fp,"","mflo $a0","EXPR *");
                return;
                break;
            case A_DIV:  // handles division operator
                emit(fp,"","div $a0 $a1","/");
                emit(fp,"","mflo $a0","/");
                return;
                break;
            case A_MOD:  // handles handles modulus operator
                emit(fp,"","div $a0 $a1","EXPR \%");
                emit(fp,"","mfhi $a0","EXPR \%");
                return;
                break;
            case A_GT:  // handles greater than operator
                emit(fp,"","slt $a0, $a0, $a1","EXPR >");
                return;
                break;
            case A_LT:  // handles less than operator
                emit(fp,"","slt $a0, $a1, $a0","EXPR <");
                return;
                break;
            case A_LE:  // handles less than or equal to operator
                emit(fp,"","add $a0, $a0, 1","EXPR <= add one to do compare");
                emit(fp,"","slt $a0, $a1, $a0","EXPR <=");
                return;
                break;
            case A_GE: // handles greater than or equal to operator
                emit(fp,"","add $a1, $a1, 1","EXPR >= add one to do compare");
                emit(fp,"","slt $a0, $a0, $a1","EXPR >=");
                return;
                break;
            case A_EQ: // handles equals operator
                emit(fp,"","slt $t2 ,$a0, $a1", "EXPR ==");
	            emit(fp,"","slt $t3 ,$a1, $a0" ,"EXPR ==");
	            emit(fp,"","nor $a0 ,$t2, $t3", "EXPR ==");
	            emit(fp,"","andi $a0 , 1", "EXPR ==");
                return;
                break;
            case A_NE: // handles not equal to operator
                emit(fp,"","slt $t2 ,$a0, $a1","EXPR !=");
	            emit(fp,"","slt $t3 ,$a1, $a0","EXPR !=");
	            emit(fp,"","or $a0 ,$t2, $t3","EXPR !=");            
                return;
                break;
        
            // should never be here
            default: emit(fp,"","","operator not defined"); 
            break;
            }
 
        }
    EMIT_AST(p->next,fp);
} // emit expr

// PRE: PTR to A_WRITE
// POST: MIPS code to generate WRITE string or write a number dependinf on our argument

void emit_write(ASTnode *p, FILE *fp)
{
    char s[100];
    //if we are writing a string then
    if(p->name != NULL)
    {
        // need to lead the address of label into $a0 and call print string
        sprintf(s,"la $a0, %s", p->label);
        emit(fp,"",s, "The string address");
        emit(fp,"","li $v0, 4", "About to print a string");
        emit(fp,"","syscall\t","call write string");
        fprintf(fp,"\n\n");

    }
    else
    // writing an expression
    {
        emit_expr(p->s1,fp); // now $a0 has the expression value
        emit(fp,"","li $v0, 1", "About to print a number");
        emit(fp,"","syscall\t","call write number");
        fprintf(fp,"\n\n");
    }
} // emit_write

// PRE: PTR to A_EXPRSTMT
// POST: Emit expression or NULL

void emit_exprstmt(ASTnode *p, FILE *fp) {
        if(p->s1!=NULL)
        { // if there is an expression, EMIT, otherwise do nothing
            emit_expr(p->s1,fp);
        }
}

// PRE: PTR to A_READ
// POST: MIPS code to read in a value and place it in the VAR of READ

void emit_read(ASTnode *p, FILE *fp)
{
    emit_var(p->s1,fp); // $a0 will be the location of the variable
    emit(fp,"","li $v0, 5", "about to read in value");
    emit(fp, "", "syscall", "read in value $v0 now has the read in value");
    emit(fp,"","sw $v0, ($a0)", "store read in value to memory");
    fprintf(fp,"\n\n");
} // emit_read

// PRE: PTR to A_ASSIGN
// POST: MIPS code to assign a VAR

void emit_assign(ASTnode *p, FILE *fp)
{
    char s[100];

    emit_expr(p->s2,fp); // Get RHS for storing
    sprintf(s,"sw $a0 %d($sp)", p->symbol->offset*WSIZE);
    emit(fp,"",s,"Assign store RHS temporarily");
    emit_var(p->s1,fp); // Get LHS
    sprintf(s,"lw $a1 %d($sp)",p->symbol->offset*WSIZE);
    emit(fp,"",s,"Assign get RHS temporarily");
    emit(fp,"","sw $a1 ($a0)", "Assign place RHS into memory"); // Assign

} // emit_assign

// PRE: PTR to A_WHILE
// POST: MIPS code to implement while loop

void emit_while(ASTnode *p, FILE *fp)
{
    char s[100];
    // makes labels to handle WHILE
    char* whileTop = CreateLabel();
    char* whileEnd = CreateLabel();

    emit(fp,whileTop,"","WHILE TOP target");
    if(p->s1!=NULL)
    {
        emit_expr(p->s1,fp); // WHILE condition
    }
    sprintf(s,"beq $a0 $0 %s", whileEnd);
    emit(fp,"",s,"WHILE branch out"); // branches out of while loop
    EMIT_AST(p->s2,fp);
    sprintf(s,"j %s",whileTop);
    emit(fp,"",s,"WHILE jump back"); // jumps back to whileTop
    emit(fp,whileEnd,"","END of while");

} // emit_while

// PRE: PTR to A_CALL
// POST: MIPS code for a function call

void emit_call(ASTnode *p, FILE *fp)
{
    char s[100];

    emit(fp,"","","Setting Up Function Call");
    emit(fp,"","","evaluate  Function Parameters");
    emit_args(p->s1,fp); // getting ARGS 

    emit(fp,"","","place   Parameters into T registers");
    emit_argStore(p->s1,fp); // Storing ARGS
    fprintf(fp,"\n");

        // call the funciton
    sprintf(s,"jal %s\t", p->name);
    emit(fp,"",s,"Call the function\n\n");

} // emit_call

// PRE: PTR to A_ARGS
// POST: MIPS code for placing ARGS

void emit_args(ASTnode *p, FILE *fp)
{
    char s[100];

    if(p!=NULL)
    {
        // Store args
        emit_expr(p->s1,fp);
        sprintf(s,"sw $a0, %d($sp)",p->symbol->offset*WSIZE);
        emit(fp,"",s,"Store call Arg temporarily");
        fprintf(fp,"\n");
        emit_args(p->next,fp);
    }    
}

// PRE: PTR to A_ARGS
// POST: MIPS code for ARGS being stored

void emit_argStore(ASTnode *p, FILE *fp)
{
    char s[100];

    if(p!=NULL)
    {
        // store ARGS into temps using global variable
        sprintf(s,"lw $a0, %d($sp)",p->symbol->offset*WSIZE);
        emit(fp,"",s,"pull out stored  Arg ");
        sprintf(s,"move $t%d, $a0", countARG++);
        emit(fp,"",s,"move arg in temp ");
        emit_argStore(p->next,fp);
    }
}

// PRE: PTR to A_IF
// POST: MIPS code for a selesction statement

void emit_if(ASTnode *p, FILE *fp)
{
    char s[100];
    // Labels to use IF and ELSE
    char *labelElse = CreateLabel();
    char *labelIf = CreateLabel();

    if(p->s1!=NULL)
    {
        emit_expr(p->s1,fp); // IF condition
    }
    sprintf(s,"beq $a0 $0 %s",labelElse); 
    emit(fp,"",s, "IF branch to else part"); // go to else
    fprintf(fp,"\n");
    emit(fp,"",""," the positive portion of if");
    EMIT_AST(p->s2->s1,fp); // EMIT IF body
    sprintf(s,"j %s", labelIf);
    emit(fp,"",s,"IF S1 end"); // label to end IF
    emit(fp,labelElse,"","ELSE target"); // target for else
    emit(fp,"",""," the negative  portion of IF if there is an else");
    emit(fp,"",""," otherwise just these lines");
    EMIT_AST(p->s2->s2,fp); // EMIT ELSE body
    emit(fp,labelIf,"","END of IF");
    
}

// PRE: PTR to A_RETURN
// POST: MIPS code for a return function

void emit_return(ASTnode *p, FILE *fp)
{

    if(p->s1 == NULL) // if there is no expression
    {
        if(p->name == "main") // if in main
        {
            emit(fp,"","li $a0, 0","RETURN has no specified value set to 0");
            emit(fp,"","lw $ra ($sp)","restore old environment RA");
            emit(fp,"","lw $sp 4($sp)","Return from function store SP");
            fprintf(fp,"\n");
            emit(fp,"","li $v0, 10","Exit from Main we are done");
            emit(fp,"","syscall","EXIT everything");
        }
        else // if not in main
        {
            emit(fp,"","li $a0, 0","RETURN has no specified value set to 0");
            emit(fp,"","lw $ra ($sp)","restore old environment RA");
            emit(fp,"","lw $sp 4($sp)","Return from function store SP");
            fprintf(fp,"\n");
            emit(fp,"","jr $ra\t","return to the caller");
        }
    }
    else // if there is an expression
    {
        if(p->name == "main") // if in main
        {
            emit_expr(p->s1,fp);
            emit(fp,"","lw $ra ($sp)","restore old environment RA");
            emit(fp,"","lw $sp 4($sp)","Return from function store SP");
            fprintf(fp,"\n");
            emit(fp,"","li $v0, 10","Exit from Main we are done");
            emit(fp,"","syscall","EXIT everything");
        }
        else // if not in main
        {
            emit_expr(p->s1,fp);
            emit(fp,"","lw $ra ($sp)","restore old environment RA");
            emit(fp,"","lw $sp 4($sp)","Return from function store SP");
            fprintf(fp,"\n");
            emit(fp,"","jr $ra\t","return to the caller");
        }
    }

} // emit_return

// PRE: PTR to VAR
// POST: $a0 has exact memory location of (L-VALUE) of VAR

void emit_var(ASTnode *p, FILE *fp)
{
    char s[100];
    // handle internal offset if any

    if(p->symbol->SubType == SYM_SCALAR)
    {
    if(p->symbol->level == 0) // global variable
    {
        // get the direct address of Global Var
        sprintf(s,"la $a0, %s", p->name);
        emit(fp,"",s,"EMIT Var global variable");
    }
    else { // local variables

        emit(fp,"","move $a0 $sp", "VAR local make a copy of stackpointer");
        sprintf(s,"addi $a0 $a0 %d", p->symbol->offset*WSIZE);
        emit(fp,"",s,"VAR local stack pointer plus offset");

    }
    }

    //add array index if needed
    if(p->symbol->SubType == SYM_ARRAY)
    {
        if(p->symbol->level == 0) // global variable
        {
            emit_expr(p->s1,fp);
            emit(fp,"","move $a1, $a0","VAR copy index array in a1");
            sprintf(s,"sll $a1 $a1 %d", LOG_WSIZE);
            emit(fp,"",s,"multiply the index by wordsize via SLL");
            // get the direct address of Global Var
            sprintf(s,"la $a0, %s", p->name);
            emit(fp,"",s,"EMIT Var global variable");
            emit(fp,"","add $a0 $a0 $a1","VAR array add internal offset");
        }
        else
        { // local variable
            emit_expr(p->s1,fp);
	        emit(fp,"","move $a1, $a0","VAR copy index array in a1");
            sprintf(s,"sll $a1 $a1 %d", LOG_WSIZE);
            emit(fp,"",s,"multiply the index by wordsize via SLL");
	        emit(fp,"","move $a0 $sp","VAR local make a copy of stackpointer");
            sprintf(s,"addi $a0 $a0 %d",p->symbol->offset*WSIZE);
	        emit(fp,"",s,"VAR local stack pointer plus offset");
	        emit(fp,"","add $a0 $a0 $a1","VAR array add internal offset");
        }
    }
} // emit_var

// PRE: PTR to A_PARAMS
// POST: MIPS code for function parameters

void emit_params(ASTnode *p, FILE *fp) {

    char s[100];

    if(p != NULL)
    { // goes through params using global variable to keep track
        sprintf(s, "sw $t%d %d($sp)",countParam++, p->symbol->offset*WSIZE);
        emit(fp,"",s,"Paramater store start of function");
        emit_params(p->next,fp);
    }


}

// PRE: PTR to ASTnode A_FUNCTIONDEC
// POST: MIPS cpde in fp

void emit_function(ASTnode *p, FILE *fp) 
{
    char s[100];
    emit(fp,p->name,"\t","function definition");
    // Carve out the Stack for activation record

    emit(fp,"","move $a1, $sp", "Activation Record carve out copy SP");
    sprintf(s, "subi $a1 $a1 %d", p->symbol->offset*WSIZE);
    emit(fp,"",s,"Activation Record carve out copy size of function");
    emit(fp,"","sw $ra , ($a1)","Store Return address ");
    sprintf(s,"sw $sp %d($a1)",WSIZE);
    emit(fp,"",s,"Store the old Stack pointer");
    emit(fp,"","move $sp, $a1","Make SP the current activation record");
    fprintf(fp, "\n\n");

    // copy the parameters to the formal registers $t0 et

    emit_params(p->s1, fp);
    fprintf(fp,"\n\n");

    // Generate the compound statement

    EMIT_AST(p->s2, fp);

    // create an implicit return depending on if we are in main or not


    // Restore RA and SP before we return
    emit(fp,"","li $a0, 0","RETURN has no specified value set to 0");
    emit(fp,"","lw $ra ($sp)", "restore old environment RA");
    sprintf(s,"lw $sp %d($sp)", WSIZE);
    emit(fp,"",s,"Return from function store SP");
    fprintf(fp,"\n");

    if(strcmp(p->name,"main") == 0)
    { // exit the system
        emit(fp,"","li $v0, 10", "Exit from Main we are done");
        emit(fp,"","syscall\t","EXIT everything");
    }
    else
    { // jump back to the caller
        emit(fp,"","jr $ra\t","return to the caller");
    }
    fprintf(fp,"\n");
    emit(fp,"","","END OF FUNCTION");
    fprintf(fp,"\n\n\n");
}

// PRE: PTR to ASTnode or NULL
// POST: MIPS code into the file for the tree

void EMIT_AST(ASTnode * p, FILE *fp)
{
   if(p==NULL) return;

   switch (p->type) {

        case A_VARDEC: // no real action
                EMIT_AST(p->s1,fp);
                EMIT_AST(p->next, fp);
                break;
        case A_COMPOUND: // no action for s1
                EMIT_AST(p->s2,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_FUNCTIONDEC: // deal with using our helper fuunction
                emit_function(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_WRITE: // deal with using our helper function
                emit_write(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_READ: // deal with using our helper function
                emit_read(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_ASSIGN: // deal with using our helper function
                emit_assign(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_WHILE: // deal with using our helper function
                emit_while(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_RETURN: // deal with using our helper function
                emit_return(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_PARAM: // deal with using a helper function
                emit_params(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_IF: // deal with using a helper function
                emit_if(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_ARGS: // deal with using helper function
                emit_args(p,fp);
                EMIT_AST(p->next,fp);
                break;
        case A_EXPRSTMT: // deal wiht using our helper funciton
                emit_exprstmt(p,fp);
                EMIT_AST(p->next,fp);
                break;

        default: printf("EMIT_AST case %d not implemented \n", p->type);
                 printf("We should never be here!!!\n");


   } // of switch
} // EMIT_AST

// PRE: PTR to AST, PTR File
// POST: prints out MIPS code into file, using helper functions

void EMIT(ASTnode* p, FILE* fp)
{
    if (p==NULL) return;
    if (fp==NULL) return;

    fprintf(fp, "# MIPS CODE GENERATED by Compilers class\n\n");
    fprintf(fp, ".data\n\n");
    EMIT_STRINGS(p,fp);
    fprintf(fp, ".align 2\n");
    EMIT_GLOBALS(p,fp);
    fprintf(fp, ".text\n\n\n.globl main\n\n\n");
    EMIT_AST(p,fp);
}
