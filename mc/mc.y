
%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mc.h"
#define YYDEBUG 1

//报错处理
//一旦出现错误，退出
extern int yylineno;
extern int yytext;
    yyerror(const char *msg)  
    {  
		
           printf("%d:  %s  at  '%s'  \n",yylineno,msg,yytext);  
    }

%}
%union {
    char                *identifier;
    Expression          *expression;
    Statement           *statement;
    StatementList       *statement_list;
    Block               *block;
    ExpressionType       _type;
    int  int_value;
}
%token <expression>     INT_LITERAL
%token <identifier>     IDENTIFIER
%token  IF BREAK FOR WHILE DO
        LP RP LC RC SEMICOLON ASSIGN
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD
        PRINT ELSE
%type   <expression> expression 
        additive_expression multiplicative_expression
        logical_expression primary_expression expression_opt
	

%type   <statement> statement for_statement if_statement
        break_statement  define_statement while_statement

%type <statement> do_while_statement
%type   <statement_list> statement_list

%type   <block> block
%token <_type> INT
%%




program
        : definition_or_statement
        | program definition_or_statement
        ;

definition_or_statement
        : statement
	//I think it is wrong written,but now it will shift/reduce
	//:statement_list
        {
            MC_Interpreter *inter = mc_get_current_interpreter();

            inter->statement_list = mc_chain_statement_list(inter->statement_list, $1);
        }
        ;
//规约成block  再规约成statement 用于上面的规约
statement_list
        : statement
        {
            $$ = mc_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = mc_chain_statement_list($1, $2);
        }
        ;
expression
        : logical_expression
	/*| define_expression ASSIGN expression
		
	{
	    $$ = mc_create_assign_expression($1->u.identifier, $3);
	}*/
        | IDENTIFIER ASSIGN expression
        {
            $$ = mc_create_assign_expression($1, $3);
		//printf("(=:,%s,%d)\n",$1,$3->u.int_value);
//		gen_expr($2,$1,$3);//char *  char *  int 
        }
        ;
logical_expression
        : additive_expression
	| logical_expression NE additive_expression
	{
	    $$ = mc_create_binary_expression(NE_EXPRESSION, $1, $3);
	}
	| logical_expression EQ additive_expression
	{
	    $$ = mc_create_binary_expression(EQ_EXPRESSION, $1, $3);
	}
        | logical_expression GT additive_expression
        {
            $$ = mc_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | logical_expression GE additive_expression
        {
            $$ = mc_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | logical_expression LT additive_expression
        {
            $$ = mc_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | logical_expression LE additive_expression
        {
            $$ = mc_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = mc_create_binary_expression(ADD_EXPRESSION, $1, $3);
	    //gen($$,$1,$2,$3,temp++);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = mc_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;
multiplicative_expression
        : primary_expression
        | multiplicative_expression MUL primary_expression
        {
            $$ = mc_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV primary_expression
        {
            $$ = mc_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
	| multiplicative_expression MOD primary_expression
	{
	    $$ = mc_create_binary_expression(MOD_EXPRESSION, $1, $3);
	}
        ;
primary_expression
        : LP expression RP//递归
        {
          $$ = $2;
        }
        | IDENTIFIER
        {
	//printf("规约成基础表达式\n");
	$$ = mc_create_identifier_expression($1);
	//printf("创建基础表达式成功\n");
/*
		MC_Interpreter *inter = mc_get_current_interpreter();
	//printf("idname=%s\n",$1);		
	if(mc_search_global_variable(inter,$1)!=NULL) printf("这个变量存在！\n");
		else $$ = mc_create_identifier_expression($1),printf("不存在这个变量，我们要创建一个\n");
     */   }
	//| IDENTIFIER
        | INT_LITERAL
	{
		//printf("**%d\n",INT_LITERAL);
	}
        ;
/*
print_v
	:IDENTIFIER
	;
*/
/*
define_expression
	:INT IDENTIFIER
	{
	//	$$=mc_create_define_expression($2);
	}
	;
*/
statement
        : expression SEMICOLON
        {
          $$ = mc_create_expression_statement($1);
        }
/*
	| define_expression SEMICOLON
	{
	  $$ = mc_create_define_statement($1);
	}*/
        | PRINT LP expression RP SEMICOLON
        {
	//printf("exp type=%d",$3->type);
            $$ = mc_create_print_statement($3);
        }
        | if_statement
        | for_statement
        | break_statement
	| define_statement
	| while_statement//new add
	| do_while_statement
	;
///*



define_statement
	:INT IDENTIFIER SEMICOLON
	{
		//printf("name=%s\n",$2);
		//printf("规约成定义语句\n");
		$$ = mc_create_define_statement($2);
		//printf("创建定义语句成功\n");
		//printf("*\n");
	}
	;

if_statement
        : IF LP expression RP block//没有else语句
        {
            $$ = mc_create_if_statement($3, $5, NULL);
        }
        | IF LP expression RP block ELSE block//有else语句
        {
            $$ = mc_create_if_statement($3, $5, $7);
        }
        ;
while_statement
	:WHILE LP expression_opt RP block
	{
		$$ =mc_create_while_statement($3,$5);
	}
	;

do_while_statement
	:DO block WHILE LP expression_opt RP SEMICOLON
	{	
		$$ =mc_create_do_while_statement($2,$5);
	}
	;

for_statement
        : FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = mc_create_for_statement($3, $5, $7, $9);
        }
        ;
break_statement
        : BREAK SEMICOLON
        {
            $$ = mc_create_break_statement();
        }
        ;
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;

block
        : LC statement_list RC
        {
            $$ = mc_create_block($2);
        }
        | LC RC
        {
            $$ = mc_create_block(NULL);
        }
        ;
%%


