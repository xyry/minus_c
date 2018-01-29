#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mc.h"
#define check_


static  MC_Value eval_int_expression(int int_value) {
    MC_Value v;
    v.type = MC_INT_VALUE;
    v.u.int_value = int_value;
    return v;
}


//获取变量
Variable *mc_search_local_variable(LocalEnvironment *env, char *identifier) {
    Variable *pos;
    if (env == NULL) {
        return NULL;
    }

    for (pos = env->variable; pos != NULL; pos = pos->next) {
        if(!strcmp(pos->name, identifier)) {
            break;
        }
    }
    return pos;
}

//获取全局变量
Variable *mc_search_global_variable(MC_Interpreter *inter, char *identifier) {
    Variable *pos;
	int cnt=1;
    for (pos = inter->variable; pos != NULL; pos = pos->next) {
	//printf("%d:%s _value=%d\n",cnt++,pos->name,pos->value.u.int_value);
        if (!strcmp(pos->name,identifier)) {
            break;
        }
    }
	//if(pos==NULL) printf("indeed null!\n");
    return pos;
}

static Variable *search_global_variable_form_env(MC_Interpreter *inter, LocalEnvironment *env, char *name) {
    GlobalVariableRef *pos;
    if (env == NULL) {
        return mc_search_global_variable(inter, name);
    }
/*寻找局部变量
    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }
*/
    return NULL;
}

//变量名出现在表达式中,此函数被调用
static MC_Value eval_identifier_expression(MC_Interpreter *inter, LocalEnvironment *env, Expression *expr) {
    MC_Value v;
    Variable *vp;

    //首先查找局部变量
    vp = mc_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        v = vp->value;
    } else {
        //如果没有,则通过全局查找
        vp = search_global_variable_form_env(inter, env, expr->u.identifier);
        if (vp != NULL) {
            v = vp->value;
        } else {
            //仍然没有则报错
            //runtime error
            exit(1);
        }
    }
    return  v;
}

static MC_Value eval_expression(MC_Interpreter *inter, LocalEnvironment *env, Expression *expr);

//布尔表达式
static MC_Value eval_boolean_expression(MC_Boolean boolean_value) {
    MC_Value v;
    v.type = MC_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    return v;
}

/**
 * 变量赋值时调用
 * identifier
 */
static MC_Value eval_assign_expression(MC_Interpreter *inter, LocalEnvironment *env, char *identifier, Expression *expression) {
    MC_Value v;
    Variable *left;
    //获得右侧表达式结果
    v = eval_expression(inter, env, expression);

    //查找局部变量
    left = mc_search_local_variable(env, identifier);

    if (left == NULL) {//找不到在全局查找
        left = search_global_variable_form_env(inter, env, identifier);
    }

    if (left != NULL) {
        left->value = v;
    } else { // 没有找到变量,注册新的变量
        if (env != NULL) {
            // 函数内的布局变量
            //mc_add_local_variable(env, identifier, &v);
        } else {
            // 函数外的全局变量
            mc_add_global_variable(inter, identifier, &v);
        }
    }
    return v;
}

void eval_binary_int(MC_Interpreter *inter, ExpressionType op,
                            int left, int right, MC_Value *result, int line_number) {

    if (dkc_is_math_operator(op)) {
        result->type = MC_INT_VALUE;
    } else if (dkc_is_compare_operator(op)) {
        result->type = MC_BOOLEAN_VALUE;
    } else {
        printf("op=%d", op);
        //debug
    }

    switch (op) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
        case ASSIGN_EXPRESSION:
            //debug
            break;
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.int_value = left / right;
		//printf("div_result=%d\n",result->u.int_value);
	    break;
        case MOD_EXPRESSION:
            result->u.int_value = left % right;
		//printf("mod_result=%d\n",result->u.int_value);
            break;
        case EQ_EXPRESSION:
            result->u.int_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            //printf("");
            //DBG_panic(("bad case...%d", operator));
            break;
    }
}

/**
 * 函数对所有二元运算进行评估
判断二元运算两边类型是否一致，
 */
MC_Value mc_eval_binary_expression(MC_Interpreter *inter, LocalEnvironment *env,
                                     ExpressionType op, Expression *left, Expression *right) {

    MC_Value left_val;
    MC_Value right_val;
    MC_Value result;

    left_val = eval_expression(inter, env, left);
    right_val = eval_expression(inter, env, right);
    //printf("%s\n", right_val.type);
    if (left_val.type == MC_INT_VALUE && right_val.type == MC_INT_VALUE) { //  1 + 2
        eval_binary_int(inter, op, left_val.u.int_value, right_val.u.int_value, &result, left->line_number);
    } else {
        //char *op_str = mc_get_operator_string(op);
        //runtime error

        exit(1);
    }

    return result;

}

// 运行表达式 或者表达式结果
static MC_Value eval_expression(MC_Interpreter *inter, LocalEnvironment *env, Expression *expr) {
    MC_Value v;
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            v = eval_boolean_expression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
	#ifdef debug
	printf("INT_EXPRESSION\n");
	#endif
            v = eval_int_expression(expr->u.int_value);
            break;
        case IDENTIFIER_EXPRESSION:
            v = eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            v = eval_assign_expression(inter, env, expr->u.assign_expression.variable, expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:      
        case SUB_EXPRESSION:     
        case MUL_EXPRESSION:      
        case DIV_EXPRESSION:       
        case MOD_EXPRESSION:      
        case EQ_EXPRESSION: 
        case NE_EXPRESSION: 
        case GT_EXPRESSION: 
        case GE_EXPRESSION: 
        case LT_EXPRESSION: 
        case LE_EXPRESSION:
            v = mc_eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            printf("expr bad case. type..%d\n", expr->type);
            break;
    }
    return v;
}

MC_Value mc_define_expression(MC_Interpreter *inter,char *name){
	MC_Value v;
	
	//printf("执行定义语句\n");
	v=eval_int_expression(0);
	//printf("1name=%s\n",name);	
	mc_add_global_variable(inter,name,&v);
	
	//printf("type=%d,value=%d\n",v.type,v.u.int_value);
	return v;
}

MC_Value mc_eval_expression(MC_Interpreter *inter, LocalEnvironment *env, Expression *expr) {
    return eval_expression(inter, env, expr);
}


//执行声明语句

static StatementResult execute_define_statement(MC_Interpreter *inter,LocalEnvironment *env,Statement *statement){
    StatementResult result;
    MC_Value value;
    result.type = NORMAL_STATEMENT_RESULT;
   // value=mc_define_expression(inter,statement->d_s.name);
	//printf("here!\n");
	//printf("_%d_\n",statement->u.expression_s->type);
   value = mc_define_expression(inter, statement->d_s.name);
	//printf("now what?\n");    
	result.u.return_value=value;
    return result;
}

//执行表达式语句
static StatementResult execute_expression_statement(MC_Interpreter *inter, LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    MC_Value value;
    result.type = NORMAL_STATEMENT_RESULT;

    value = mc_eval_expression(inter, env, statement->u.expression_s);
    result.u.return_value = value;
    return result;
}




//执行if语句
static StatementResult execute_if_statement(MC_Interpreter *inter, LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    MC_Value cond;
    result.type = NORMAL_STATEMENT_RESULT;
    cond = mc_eval_expression(inter, env, statement->u.if_s.condition);
    if (cond.type != MC_BOOLEAN_VALUE) {
        //runtime error
        printf("runtime error");
        exit(1);
    }
	
	
    if (cond.u.boolean_value) {
	//printf("if_s.then_block  addr=%d\n",statement->u.if_s.then_block->statement_list);//debug
	//printf("expr_s   and its addr=%x\n",s)
        result = mc_execute_statement_list(inter, env, statement->u.if_s.then_block->statement_list);
    } else {
//if else_statement exists!!
        if (statement->u.if_s.else_block) {
	//printf("if_s.else_block  addr=%d\n",statement->u.if_s.else_block->statement_list);//debug
	//printf("expr_s   and its addr=%x\n",s)
            result = mc_execute_statement_list(inter, env, statement->u.if_s.else_block->statement_list);
        }
    }
    return result;
}

static StatementResult execute_break_statement(MC_Interpreter *inter, LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = BREAK_STATEMENT_RESULT;
    return result;
}

StatementResult execute_do_while_statement(MC_Interpreter *inter,LocalEnvironment *env,Statement *statement){
	StatementResult result;
	MC_Value cond;
	result.type=NORMAL_STATEMENT_RESULT;
	for(;;){
		if(statement->u.d_w_s.condition){
			cond=mc_eval_expression(inter,env,statement->u.d_w_s.condition);
			if(cond.type!=MC_BOOLEAN_VALUE){
				printf("runtime error\n");
                        	printf("条件类型必须是布尔类型\n");
				exit(1);                 		                 }
			if(cond.u.boolean_value==0) break; 			
		}
		result=mc_execute_statement_list(inter,env,statement->u.d_w_s.block->statement_list);
		if(result.type==RETURN_STATEMENT_RESULT){break;}
		else if(result.type==BREAK_STATEMENT_RESULT){result.type=NORMAL_STATEMENT_RESULT;break;}
	}
	return result;
}

//执行 while
 StatementResult execute_while_statement(MC_Interpreter *inter,
LocalEnvironment *env,Statement *statement){
        StatementResult result;
        MC_Value cond;
        result.type=NORMAL_STATEMENT_RESULT;
        for(;;){
             if(statement->u.w_s.condition){
               cond=mc_eval_expression(inter,env,statement->u.w_s.condition);
                if(cond.type!= MC_BOOLEAN_VALUE){
                        printf("runtime error\n");
                        printf("条件类型必须是布尔类型\n");exit(1);
                 } 
                if(cond.u.boolean_value==0) break;
              }
                result=mc_execute_statement_list(inter,env,statement->u.w_s.block->statement_list);
                if(result.type ==RETURN_STATEMENT_RESULT) {
                        break;
                }else if(result.type ==BREAK_STATEMENT_RESULT){
                        result.type=NORMAL_STATEMENT_RESULT;
                        break;
                } 
	}
        return result;
}

//执行 for
static StatementResult execute_for_statement(MC_Interpreter *inter, LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    MC_Value cond;
    result.type = NORMAL_STATEMENT_RESULT;
    if (statement->u.for_s.init) {
        mc_eval_expression(inter, env, statement->u.for_s.init);
    }
    for (;;) {
        //for的判断循环条件
        if (statement->u.for_s.condition) {
            cond = mc_eval_expression(inter, env, statement->u.for_s.condition);
            if (cond.type != MC_BOOLEAN_VALUE) {
                printf("runtime error\n");
                exit(1);
            }
            if (!cond.u.boolean_value) {
                break;
            }
        }
        result = mc_execute_statement_list(inter, env, statement->u.for_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
		//先写上这句话，暂时未实现返回return 语句
        } else if (result.type == BREAK_STATEMENT_RESULT) {
	    //这句话就是说 执行break，但是for语句正常结束 否则不会执行下面的语句
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }

        if (statement->u.for_s.post) {
            mc_eval_expression(inter, env, statement->u.for_s.post);
        }
    }
    return result;
}


void check ( StatementType type,Statement *s){
	switch(type){
		case 100:printf("expr_s   and its addr=%x\n",s);break;
		case 101:printf("global_s and its addr=%x\n",s);break;
		case 102:printf("if_s     and its addr=%x\n",s);break;
		case 103:printf("for_s    and its addr=%x\n",s);break;
		case 104:printf("break_s  and its addr=%x\n",s);break;
		case 105:printf("print_s  and its addr=%x\n",s);break;
		case 106:printf("defines  and its addr=%x\n",s);break;
		case 107:printf("while_s  and its addr=%x\n",s);break;
		case 108:printf("dowhile  and its addr=%x\n",s);break;
		default: printf("error    and its addr=%x\n",s);break;
	}
}
static StatementResult execute_statement(MC_Interpreter *inter, LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    MC_Value v;
#ifdef check_
	check(statement->type,statement);
#endif

    switch (statement->type) {
        case EXPRESSION_STATEMENT:
	#ifdef debug
		printf("EXPRESSION_STATEMETN\n");
	#endif
            result = execute_expression_statement(inter, env, statement);
            break;
	case DEFINE_STATEMENT:
	#ifdef debug
		printf("DEFINE_STATEMENT\n");
	#endif
	    result = execute_define_statement(inter,env,statement);		
	    break;
       /* case GLOBAL_STATEMENT:
            result = execute_global_statement(inter, env, statement);
            break;*/// 未实现
        case IF_STATEMENT:
            result = execute_if_statement(inter, env, statement);
            break;
        case FOR_STATEMENT:
            result = execute_for_statement(inter, env, statement);
            break;
	case WHILE_STATEMENT:
	    result= execute_while_statement(inter,env,statement);
	    break;
	case DO_WHILE_STATEMENT:
            result= execute_do_while_statement(inter,env,statement);
	    break;
        case BREAK_STATEMENT:
            result = execute_break_statement(inter, env, statement);
            break;
        case PRINT_STATEMENT:
	#ifdef debug
		printf("PRINT_STATEMENT\n");	
	#endif
/*计算表达式的值*/
            v = eval_expression(inter, env, statement->u.p_s.exp);
            printf("%d",v.u.int_value);
            printf("\n");
            break;
        case STATEMENT_TYPE_CONUT_PLUS_1:
            break;
        default:
            printf("statement bad case %d\n",statement->type);
    }
    return result;
}


//执行语句链表
StatementResult mc_execute_statement_list(MC_Interpreter *inter, LocalEnvironment *env, StatementList *list) {
    StatementList *pos;
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    for (pos = list; pos; pos = pos->next) {
	//printf("addr=%x\n",pos);
        result = execute_statement(inter, env, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            return result;
        }
    }
    return result;
}
