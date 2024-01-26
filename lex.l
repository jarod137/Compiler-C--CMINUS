/* Lex file to support CMINUS TOKENS 
 Source code from
 Shaun Cooper
 January 2015

 Jared Kaiser CS 370 Feb, 2023
 added variable recongnition functionality
*/
%{

int mydebug=0;
int lineno=1;
#include "ast.h"
#include "y.tab.h"
%}


%%

int         {return(T_INT);}
void        {return(T_VOID);}
read        {return(T_READ);}
if          {return(T_IF);}
else        {return(T_ELSE);}
while       {return(T_WHILE);}
return      {return(T_RETURN);}
write       {return(T_WRITE);}
\+          {return(T_PLUS);}
\-          {return(T_MINUS);}
\%          {return(T_MOD);}
"*"         {return(T_MULT);}
"<"         {return(T_GT);}
"<="        {return(T_GE);}
">"         {return(T_LT);}
">="        {return(T_LE);}
"=="        {return(T_EQ);}
"!="        {return(T_NE);}

\/\/.* {if (mydebug) fprintf(stderr,"Comment found\n"); }


[a-zA-Z][a-zA-Z0-9_]*	{if (mydebug) fprintf(stderr,"Letter found\n"); 
                            yylval.string=strdup(yytext);
                            return(T_ID);}

\".*\" {if (mydebug) fprintf(stderr,"String found\n"); 
                            yylval.string=strdup(yytext);
                            return(T_STRING);}

[0-9][0-9]*	{if (mydebug) fprintf(stderr,"Number found\n"); 
                            yylval.value=atoi((const char *)yytext); 
                            return(T_NUM);}

[ \t]		{if (mydebug) fprintf(stderr,"Whitespace found\n");}

[;=*%&/|(),{}'\[\]'""] 	{ if (mydebug) fprintf(stderr,"return a token %c\n",*yytext); 
                       return (*yytext);}



\n 		{if (mydebug) fprintf(stderr,"cariage return %c\n",*yytext); 
            lineno++;}

%%

int yywrap(void)
{ return 1;}
