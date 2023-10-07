#pragma once
#include "Context.h"

struct Interpreter {

	static void execute(BPL_Program& program,Context &context);

	static void exe_block(uint32_t& current_block,  BPL_Program& program, Context* ctx);

	static void exe_assignment(uint32_t&current_block, BPL_Program&program, Context* ctx);
	static void exe_if(uint32_t& current_block,  BPL_Program& program, Context* ctx);

	static Variable evaluate_comp_factor(uint32_t& current_block, BPL_Program& program, Context* ctx);
	static Variable evaluate_comp_primary(uint32_t& current_block, BPL_Program& program, Context* ctx);

	static void exe_loop(uint32_t& current_block, BPL_Program& program, Context* ctx);

	static Variable exe_function_call(uint32_t& block, BPL_Program& program, Context* ctx);
	static Variable exe_inline_switch(uint32_t& block, BPL_Program& program, Context* ctx);

	static Variable exe_inline_if(uint32_t& block, BPL_Program& program, Context* ctx);

	static Variable evaluate_term(uint32_t& block,  BPL_Program& program, Context* ctx);
	static Variable evaluate_farctor(uint32_t& block,  BPL_Program& program, Context* ctx);
	static Variable evaluate_primary(uint32_t& block, BPL_Program& program, Context* ctx);
};