#include <malloc.h>
#include "mc.h"


static MC_Interpreter *st_current_interperter; // 全局解释器

MC_Interpreter *mc_get_current_interpreter(void) {
    return st_current_interperter;
}

void mc_set_current_interpreter(MC_Interpreter *inter) {
    st_current_interperter = inter;
}

//申请存放表达式的地址空间
//并且对表达式类型进行赋值
Expression *mc_alloc_expression(ExpressionType type) {
    Expression *exp;
	//申请一个表达式类型的指针指向该表达式
    exp = (Expression *)malloc(sizeof(Expression));
    exp->type = type;//表达式类型在这里赋值


    return exp;
}

//申请语句内存空间
//并且对语句类型进行赋值
static Statement *mc_alloc_statement(StatementType type) {
    Statement *st;
    st = (Statement *) malloc(sizeof(Statement));
    st->type = type;


    return st;
}




//创建赋值表达式
Expression *mc_create_assign_expression(char *variable, Expression *operand) {
    Expression *exp;
    exp = mc_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;//变量名称
    exp->u.assign_expression.operand = operand;//赋值表达的右部
    return exp;
}


//创建二元表达式 比较运算符
Expression *mc_create_binary_expression(ExpressionType op, Expression *left, Expression *right) {


        Expression *exp;
        exp = mc_alloc_expression(op);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    
}

//变量表达式
Expression *mc_create_identifier_expression(char *identifier) {
    MC_Interpreter *inter = mc_get_current_interpreter();
    Variable *var;
    var=mc_search_global_variable(inter,identifier);
    if(var!=NULL) {
              //  printf("这个变量存在！\n");
                Expression *exp;
                exp->type=INT_EXPRESSION;
                exp->u.identifier=var;
                return exp;
     
    }else{ 
        
       // printf("不存在这个变量，我们要创建一个\n");   
        
    Expression *exp;
    exp = mc_alloc_expression(IDENTIFIER_EXPRESSION);//变量
    exp->u.identifier = identifier;
    return exp;
    }
}

Expression *mc_create_define_expression(char *identifier){
	Expression *exp;
	exp=mc_alloc_expression(DEFINE_EXPRESSION);
	exp->u.identifier=identifier;
	return exp;
}

// 创建表达式语句

Statement *mc_create_define_statement(char *name){
	Statement *st;
	st=mc_alloc_statement(DEFINE_STATEMENT);
	//st->u.expression_s->type=DEFINE_EXPRESSION;
	Expression *exp;
	exp=mc_alloc_expression(INT_EXPRESSION);
	st->u.expression_s=exp;
//	printf("1\n");
	DefineStatement ds;
	char *tmp;
	tmp=(char *)malloc(strlen(name)+1);
	strcmp(tmp,name);
	
//	printf("1.1\n");
	//strcpy(st->u.d_s->name,name);
	//printf("name=%s\n",name);
	st->d_s.name=name;
	//st->u.d_s=define_statement;
//	printf("2\n");
	return st;
}

Statement *mc_create_expression_statement(Expression *expression) {
    Statement *st;
    st = mc_alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;//对表达式语句还要进行表达式类型赋值
    return st;
}

//全局语句
/*
Statement *mc_create_global_statement(IdentifierList *identifier_list) {
    Statement *st;
    st = mc_alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;

    return st;
}

*/

//print语句
Statement *mc_create_print_statement(Expression *p) {
    Statement *st;
    st = mc_alloc_statement(PRINT_STATEMENT);
    st->u.p_s.exp = p;

    return st;
}

//if语句
Statement *mc_create_if_statement(Expression *condition, Block *then_block,
                                   Block *else_block) {

    Statement *st;
    st = mc_alloc_statement(IF_STATEMENT);

    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.else_block = else_block;

    return st;
}









// for 语句
Statement *mc_create_for_statement(Expression *init, Expression *condition, Expression *post, Block *block) {
    Statement *st;
    st = mc_alloc_statement(FOR_STATEMENT);
    st->u.for_s.init = init;
    st->u.for_s.condition = condition;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}


//do while 语句
Statement *mc_create_do_while_statement(Block *block,Expression *condition){
	Statement *st;
	st=mc_alloc_statement(DO_WHILE_STATEMENT);
	st->u.d_w_s.condition=condition;
	st->u.d_w_s.block=block;
	return st;
}

//while 语句
Statement *mc_create_while_statement(Expression *condition,Block *block){
        Statement *st;
        st=mc_alloc_statement(WHILE_STATEMENT);
        st->u.w_s.condition=condition;
        st->u.w_s.block=block;
        return st;
}

// break 语句
Statement *mc_create_break_statement(void) {
    Statement *st;
    st = mc_alloc_statement(BREAK_STATEMENT);
    return st;
}

// 创建块段
Block *mc_create_block(StatementList *statement_list) {
    Block *block;
    block = (Block *)malloc(sizeof(Block));
    block->statement_list = statement_list;
    return block;
}

//创建全局变量


//添加全局变量
 void mc_add_global_variable(MC_Interpreter *inter, char *identifier, MC_Value *value) {
    Variable *new_variable;
    //new_variable = mc_execute_malloc(inter, sizeof(Variable));
    //new_variable->name = mc_execute_malloc(inter, strlen(identifier) + 1);
//this function is unuseful so i use another way to achieve it
//and it works 2018.1.1 13:28
	new_variable =(Variable *)malloc(sizeof(Variable));
	new_variable->name=(char *)malloc(strlen(identifier)+1);
    strcpy(new_variable->name, identifier);
	//printf("name=%s\n",identifier);
	//printf("new_variable->name=%s",new_variable->name);
    new_variable->value = *value;
    new_variable->next = inter->variable;
    inter->variable = new_variable;
	//printf("%s=%d\n",inter->variable->name,inter->variable->value.u.int_value);
	//printf("执行完毕\n");
}

//创建语句 数组
//尾插法 创建一条语句链
StatementList *mc_create_statement_list(Statement *statement) {
    StatementList *sl;
    sl = (StatementList *)malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL; 
    return sl;
}


StatementList *mc_chain_statement_list(StatementList *list, Statement *statement) {
    StatementList *pos;
	//如果list不存在，那么就返回这条语句，即这条语句为语句链表链的第一条语句
    if (list == NULL) {
        return mc_create_statement_list(statement);
    }
	//如果存在语句链表链，for,一直找到最后一条语句链表
    for (pos = list; pos->next != NULL; pos = pos->next);
//把这条语句链加入到语句链表中
    pos->next = mc_create_statement_list(statement);
    return list;
}


