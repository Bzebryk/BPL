#include "Interpreter.h"

void Interpreter::execute(BPL_Program&program,Context &context) {

	for (uint32_t &block_id : program.program) {
		exe_block(block_id, program,&context);
	}

}

void Interpreter::exe_block(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	Block* b = program.blocks[current_block];

	switch (b->type) {

	case Block::ASSIGNMENT:
		exe_assignment(current_block, program,ctx);
		break;
	case Block::IF:
		exe_if(current_block, program, std::make_shared<Context>(ctx).get());
		break;
	case Block::LOOP:
		exe_loop(current_block, program,ctx);
		break;
	case Block::BREAK:
		ctx->breake();
		break;
	case Block::FUNCTION_CALL:
		exe_function_call(current_block, program, std::make_shared<Context>(ctx).get());
		break;
	case Block::RETURN: {
		Return_Block* bl = (Return_Block*)b;
		ctx->set_return_value(evaluate_term(bl->return_expression,program,ctx));
		ctx->returned(true);
		break;
	}
	}
}

void Interpreter::exe_loop(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	Loop_Block* block = (Loop_Block*)program.blocks[current_block];
	
	ctx->breake(false);

	while (!ctx->breake_flag && !ctx->return_flag) {

		std::shared_ptr ctx_sh_ptr = std::make_shared<Context>(ctx);

		for (uint32_t& block_id : block->inner_blocks) {
			exe_block(block_id, program, ctx_sh_ptr.get());
			if (ctx->breake_flag)break;
			if (ctx->return_flag)break;
		}

	}

	ctx->breake(false);
}

Variable Interpreter::evaluate_comp_factor(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	Comp_Expression_Factor* block = (Comp_Expression_Factor*)program.blocks[current_block];

	Variable left_value = false;
	
	switch (program.blocks[block->left]->type) {
	case Block::COMP_FACTOR:
		left_value = evaluate_comp_factor(block->left, program, ctx);
		break;
	case Block::COMP_PRIMARY:
		left_value = evaluate_comp_primary(block->left, program, ctx);
		break;
	}

	if (!block->b_right) return left_value;

	Variable right_value;

	switch (program.blocks[block->right]->type) {
	case Block::COMP_FACTOR:
		right_value = evaluate_comp_factor(block->right, program, ctx);
		break;
	case Block::COMP_PRIMARY:
		right_value = evaluate_comp_primary(block->right, program, ctx);
		break;
	}

	Operator_Block* op = (Operator_Block*)program.blocks[block->op];

	if (op->operator_str == "&&") {
	
		return left_value.getc_as<number_t>() && right_value.getc_as<number_t>();
	}
	else if(op->operator_str == "||") {
		return left_value.getc_as<number_t>() || right_value.getc_as<number_t>();
	}
}
Variable Interpreter::evaluate_comp_primary(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	Comp_Expression_Primary* block = (Comp_Expression_Primary*)program.blocks[current_block];

	Variable left_value = false;

	if (program.blocks[block->left]->type == Block::COMP_FACTOR) {
		left_value = evaluate_comp_factor(block->left,program,ctx);
	}
	else {
		left_value = evaluate_term(block->left, program, ctx);
	}
	
	if (!block->b_right) return left_value == true;

	Variable right_value = false;

	if (program.blocks[block->right]->type == Block::COMP_FACTOR) {
		right_value = evaluate_comp_factor(block->right, program, ctx);
	}
	else {
		right_value = evaluate_term(block->right, program, ctx);
	}

	Operator_Block* op = (Operator_Block*)program.blocks[block->op];

	bool b_true = false;

	if (op->operator_str == "<") {
		b_true = left_value < right_value;
	}
	else if (op->operator_str == ">") {
		b_true = left_value > right_value;
	}
	else if (op->operator_str == ">=") {
		b_true = left_value >= right_value;
	}
	else if (op->operator_str == "<=") {
		b_true = left_value <= right_value;
	}
	else if (op->operator_str == "!=") {
		b_true = left_value != right_value;
	}
	else if (op->operator_str == "==") {
		b_true = left_value == right_value;
	}

	return b_true;

}

void Interpreter::exe_if(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	if_Block* block = (if_Block*)program.blocks[current_block];

	Variable val  = evaluate_comp_factor(block->comp_exp,program,ctx);

	if (val == true) {
		ctx->breake(false);
		for (uint32_t &block_id : block->inner_blocks) {
			exe_block(block_id, program,ctx);
			if (ctx->breake_flag)break;
			if (ctx->return_flag)break;
		}
		return;
	}
	for (uint32_t el : block->elifs) {

		if_Block* elif = (if_Block*)program.blocks[el];

		val = evaluate_comp_factor(elif->comp_exp, program, ctx);

		if (val == true) {
			ctx->breake(false);
			for (uint32_t& block_id : elif->inner_blocks) {
				exe_block(block_id, program, ctx);
				if (ctx->breake_flag)break;
				if (ctx->return_flag)break;
			}
			return;
		}
	}

	if (block->b_else) {
		Else_block *els = (Else_block*)program.blocks[block->else_block];

		for (uint32_t& block_id : els->inner_blocks) {
			exe_block(block_id, program, ctx);
			if (ctx->breake_flag)break;
			if (ctx->return_flag)break;
		}
		return;


	}
}

Variable Interpreter::exe_function_call(uint32_t& block, BPL_Program& program, Context* ctx) {

	Function_Call_Block* f = (Function_Call_Block*)program.blocks[block];
	Varname_Block* b = (Varname_Block*)program.blocks[f->function_name];


	Interpreter_host_func func = ctx->get_host_function(b->varname);

	if (func) {
			Args_Block* args = (Args_Block*)program.blocks[f->args];

			Variable* args_table = new Variable[args->expressions.size()];

			for (uint32_t i = 0; i < args->expressions.size(); i++) {
				if (program.blocks[args->expressions[i]]->type == Assignment_Block::VarType::STRING) {
					String_Block* strb = (String_Block*)program.blocks[args->expressions[i]];
					args_table[i].set<string_t>(Variable::STRING, strb->value);
				}
				else {
					args_table[i] = evaluate_term(args->expressions[i], program, ctx);
				}
			}

			Variable return_value = func(args->expressions.size(), args_table);
			
			delete[] args_table;

			return return_value;
	}

	try {
			Function_Block& func = program.program_functions.at(b->varname);

			Args_Block* args = (Args_Block*)program.blocks[f->args];
			
			Context func_ctx;
			
			func_ctx.host_functions = *ctx->cpy_host_functions();

			for (uint32_t i = 0; i < args->expressions.size(); i++) {
				if(i < func.varnames.size())
				if (program.blocks[args->expressions[i]]->type == Assignment_Block::VarType::STRING) {
					String_Block* strb = (String_Block*)program.blocks[args->expressions[i]];
					*func_ctx.get_variable(func.varnames[i]) = strb->value;
				}
				else {
					*func_ctx.get_variable(func.varnames[i]) = evaluate_term(args->expressions[i], program, ctx);
				}
			}

			func_ctx.returned(false);
			
			for (uint32_t& bid : func.inner_blocks) {
				
				exe_block(bid, program,&func_ctx);
				if (func_ctx.return_flag)break;

			}

			return func_ctx.get_return_value();

	} catch (...) {}


	std::cout << "Function " << b->varname << " not reckognized !\n";

	return 0;
}

void Interpreter::exe_assignment(uint32_t& current_block, BPL_Program& program, Context* ctx) {

	Assignment_Block* block = (Assignment_Block*)program.blocks[current_block];

	block->varname;

	Varname_Block* varn = (Varname_Block*)program.blocks[block->varname];

	std::string& varname = varn->varname;

	if (block->b_fast_assign) {
		*ctx->get_variable(varname) += block->fast_val;
		return;
	}

	if (!block->b_expression) {
		*ctx->get_variable(varname) = 0.0;
		return;
	}

	*ctx->get_variable(varname) = evaluate_term(block->expression, program, ctx);

}

Variable Interpreter::evaluate_term(uint32_t& block, BPL_Program& program, Context* ctx){

	Expression_Term_Block* b = (Expression_Term_Block*)program.blocks[block];
	
	Variable left_value = 0;

	switch (program.blocks[b->left_term]->type) {
	case Block::EXPRESSION_TERM:
		left_value = evaluate_term(b->left_term, program, ctx);
		break;
	case Block::EXPRESSION_FACTOR:
		left_value = evaluate_farctor(b->left_term, program, ctx);
		break;
	case Block::EXPRESSION_PRIMARY:
		left_value = evaluate_primary(b->left_term, program, ctx);
		break;
	}

	if (!b->b_right)return left_value;

	Variable right_value = 0;
	switch (program.blocks[b->right_term]->type) {
	case Block::EXPRESSION_TERM:
		right_value = evaluate_term(b->right_term, program, ctx);
		break;
	case Block::EXPRESSION_FACTOR:
		right_value = evaluate_farctor(b->right_term, program, ctx);
		break;
	case Block::EXPRESSION_PRIMARY:
		right_value = evaluate_primary(b->right_term, program, ctx);
		break;
	}
	
	
	Operator_Block* o = (Operator_Block*)program.blocks[b->add_op];


	if (o->operator_str == "+") {
		return left_value + right_value;
	}

	return left_value - right_value;

}

Variable Interpreter::evaluate_farctor(uint32_t& block, BPL_Program& program, Context* ctx) {
	
	Exoression_Factor_Block* b = (Exoression_Factor_Block *)program.blocks[block];

	Variable left_value = 0;

	switch (program.blocks[b->left_factor]->type) {
	case Block::EXPRESSION_TERM:
		left_value = evaluate_term(b->left_factor, program, ctx);
		break;
	case Block::EXPRESSION_FACTOR:
		left_value = evaluate_farctor(b->left_factor, program, ctx);
		break;
	case Block::EXPRESSION_PRIMARY:
		left_value = evaluate_primary(b->left_factor, program, ctx);
		break;
	}

	if (!b->b_right)return left_value;

	Variable right_value = 0;
	switch (program.blocks[b->right_factor]->type) {
	case Block::EXPRESSION_TERM:
		right_value = evaluate_term(b->right_factor, program, ctx);
		break;
	case Block::EXPRESSION_FACTOR:
		right_value = evaluate_farctor(b->right_factor, program, ctx);
		break;
	case Block::EXPRESSION_PRIMARY:
		right_value = evaluate_primary(b->right_factor, program, ctx);
		break;
	}


	Operator_Block* o = (Operator_Block*)program.blocks[b->mul_op];

	if (o->operator_str == "*") {
		return left_value * right_value;
	}
	else if (o->operator_str == "/") {
		return left_value / right_value;
	}

	return left_value % right_value;

}
Variable Interpreter::evaluate_primary(uint32_t& block, BPL_Program& program, Context* ctx) {

	Exoression_Primary_Block* b = (Exoression_Primary_Block*)program.blocks[block];
	
	Variable ret_val = 0;

	switch (program.blocks[b->primary_block]->type) {
	case Block::VARNAME: {
		Varname_Block* v = (Varname_Block*)program.blocks[b->primary_block];
		ret_val = *ctx->get_variable(v->varname);
		break;
	}
	case Block::STRING: {
		String_Block* s = (String_Block*)program.blocks[b->primary_block];
		ret_val = s->value;
		
		break;
	}
	case Block::NUMBER: {
		Number_Block* n = (Number_Block*)program.blocks[b->primary_block];
		ret_val = n->value;
		break;
	}
	case Block::EXPRESSION_TERM: {
		ret_val = evaluate_term(b->primary_block, program, ctx);
		break;
	}
	case Block::FUNCTION_CALL: {

		ret_val = exe_function_call(b->primary_block, program,std::make_shared<Context>(ctx).get());

		break;
	}
	case Block::INLINE_SWITCH: {
		ret_val = exe_inline_switch(b->primary_block, program, ctx);

		break;
	}
	case Block::INLINE_IF: {
		ret_val = exe_inline_if(b->primary_block, program, ctx);

		break;
	}

	}

	return ret_val;
}
Variable Interpreter::exe_inline_switch(uint32_t& block, BPL_Program& program, Context* ctx) {

	Inline_Switch_Block* b = (Inline_Switch_Block*)program.blocks[block];

	Variable case_value = evaluate_term(b->case_expression, program,ctx);

	for (uint32_t i = 0; i < b->cases.size(); i++) {
		
		Inline_Switch_Case_Block* c = (Inline_Switch_Case_Block*)program.blocks[b->cases[i]];

		Variable value = evaluate_term(c->case_expression, program, ctx);
		
		if (value == case_value) return evaluate_term(c->return_expression, program, ctx);
	}

	return 0;
}


Variable Interpreter::exe_inline_if(uint32_t& block, BPL_Program& program, Context* ctx) {

	Inline_If_Block* b = (Inline_If_Block*)program.blocks[block];

	Variable val = evaluate_comp_factor(b->comp_exp, program, ctx);

	return val == true ? evaluate_term(b->true_expression, program, ctx) : evaluate_term(b->false_expression, program, ctx);

}