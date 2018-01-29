#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mc.h"

MC_Interpreter *MC_create_interpreter(void) {
    MC_Interpreter *interpreter;
    interpreter = (MC_Interpreter *)malloc(sizeof(MC_Interpreter));

    interpreter->variable = NULL;
    interpreter->statement_list = NULL;

    mc_set_current_interpreter(interpreter);
    //add_native_functions(interpreter);

    return interpreter;
}

// 编译
void MC_compile(MC_Interpreter *inter, FILE *fp) {
    extern int yyparse(void);
    extern FILE  *yyin;
    //mc_set_current_interpreter(inter);	
/*
yyparse()

yacc生成的语法分析程序的入口点是yyparse()，当程序调用yyparse()时，语法分析程序就试图分析输入流。如果分析成功，语法分析程序就返回一个零值，反之，则返回一个非零值。每次调用该函数，语法分析程序就重新开始进行分析，而忘记上次返回的状态，这与lex扫描程序十分不同，它能够获得调用后的每一次状态。
*/
    yyin = fp;
    if (yyparse()) {
        if (inter->statement_list) {
            fprintf(stderr, "MC_compile Error \n");
        }
    }
    //mc_reset_string_literal_buffer();
}

void MC_interpret_run(MC_Interpreter *inter) {
    //inter->execute_storage = MEM_open_storage(0);
    //mc_add_std_fp(inter);
    mc_execute_statement_list(inter, NULL, inter->statement_list);
}

int main(int argc, char **argv) {
    MC_Interpreter     *interpreter;
    FILE *fp;

    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        exit(1);
    }
    interpreter = MC_create_interpreter();
    MC_compile(interpreter, fp);
    MC_interpret_run(interpreter);
    fclose(fp);
    return 0;
}
