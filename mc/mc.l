%option yylineno
%{

    //词法分析器
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "mc.h"
    #include "y.tab.h"
	 int lineno;
    /*
    解析一个文件完毕后会调用yywrap,返回表示1结束,返回0表示继续
    */
    int yywrap(void) {
        return 1;
    }
extern yylineno;
    //定义保留字
%}

%%
<INITIAL>"if"           return IF;
<INITIAL>"else"         return ELSE;


<INITIAL>"for"          return FOR;
<INITIAL>"break"        return BREAK;
<INITIAL>"while"        return WHILE;
<INITIAL>"do"           return DO;

<INITIAL>"("            return LP;
<INITIAL>")"            return RP;
<INITIAL>"{"            return LC;
<INITIAL>"}"            return RC;
<INITIAL>";"            return SEMICOLON;


<INITIAL>"="            return ASSIGN;

<INITIAL>"==" return EQ;
<INITIAL>"!=" return NE;
<INITIAL>">"            return GT;
<INITIAL>">="           return GE;
<INITIAL>"<"            return LT;
<INITIAL>"<="           return LE;

<INITIAL>"+"            return ADD;
<INITIAL>"-"            return SUB;
<INITIAL>"*"            return MUL;
<INITIAL>"/"            {return DIV;}
<INITIAL>"%"		{return MOD;}

<INITIAL>"print"        return PRINT;
<INITIAL>"int" return INT;

<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    //标识符(变量名，函数名)
    char *new_str;
    new_str = (char *)malloc(strlen(yytext) + 1);
    strcpy(new_str, yytext);
    yylval.identifier = new_str;
    return IDENTIFIER;
}

<INITIAL>([1-9][0-9]*)|0 {
    //整数类型
    Expression *expression = mc_alloc_expression(INT_EXPRESSION);
    sscanf(yytext, "%d", &expression->u.int_value);
    yylval.expression = expression;
    return INT_LITERAL;
}

<INITIAL>\n   ;//{lineno++;printf("lineno=%d\n",lineno);} debug
<INITIAL>[ \t] ;/*对于空格和制表符，无动作*/

%%
