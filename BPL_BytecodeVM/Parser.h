#pragma once
#include "Tokenizer.h"
#include <optional>
#include <string>
#include <map>

struct Serialized_Data {
	
};

struct Block {
	enum Type {
		NONE,
		PROGRAM,
		IF,
		LOOP,
		ASSIGNMENT,
		FUNCTION_CALL,
		EXPRESSION_TERM,
		EXPRESSION_FACTOR,
		EXPRESSION_PRIMARY,
		INLINE_SWITCH,
		INLINE_SWITCH_CASE,
		INLINE_IF,
		BREAK,
		VARNAME,
		ARGS,
		OPERATOR,
		NUMBER,
		STRING,
		TYPE_BLOCK,
		STRING_EXPRESSION,
		FUNCTION,
		RETURN,
		COMP_FACTOR,
		COMP_PRIMARY,
		ELSE
	};
	Block(const Type& t = NONE) :type(t) {}
	Type type;
	template<typename T>
	static T& as(Block* b) { return *((T*)b); }
	virtual void operator<<(std::ostream& out) = 0;
};

struct None_Block :Block {
	None_Block() :Block(NONE){}
	virtual void operator<<(std::ostream& out) {}
};

struct Comp_Expression_Primary :Block {
	Comp_Expression_Primary() :Block(Block::COMP_PRIMARY) {}

	uint32_t left, right, op;
	bool b_right;

	virtual void operator<<(std::ostream& out) {
		out << type << left << right << op;
	}

};


struct Comp_Expression_Factor :Block {
	Comp_Expression_Factor() :Block(Block::COMP_FACTOR), b_right(false) {}

	uint32_t left, right, op;
	bool b_right;

	virtual void operator<<(std::ostream& out) {
		out << type << left<< right << b_right << op;
	}
};

struct if_Block :Block {
	if_Block() :Block(Block::IF), b_else(false){}

	uint32_t comp_exp;

	std::vector<uint32_t> inner_blocks;
	std::vector<uint32_t> elifs;
	uint32_t else_block;
	bool b_else;


	virtual void operator<<(std::ostream& out) {
		out << type << comp_exp <<inner_blocks.size();
		for (const uint32_t& ib : inner_blocks) {
			out << ib;
		}
	}

};
struct Else_block: Block{
	Else_block() :Block(Block::ELSE) {}

	std::vector<uint32_t> inner_blocks;

	virtual void operator<<(std::ostream& out) {
	}
};

struct Loop_Block : Block {
	Loop_Block() :Block(Block::LOOP) {}
	std::vector<uint32_t> inner_blocks;

	virtual void operator<<(std::ostream& out) {
		out << type << inner_blocks.size();
		for (const uint32_t& ib : inner_blocks) {
			out << ib;
		}
	}
};


struct Assignment_Block :Block {

	enum VarType {
		STRING,
		NUMBER,
		VOID,
	};
	
	Assignment_Block() :Block(Block::ASSIGNMENT) {}
	uint32_t varname, expression;
	VarType var_type;
	int32_t fast_val;
	bool b_fast_assign;
	bool b_expression;

	virtual void operator<<(std::ostream& out) {
		out << type << varname << expression<<var_type<<fast_val<<b_fast_assign<<b_expression;
	}

};

struct Function_Call_Block :Block {
	Function_Call_Block() :Block(Block::FUNCTION_CALL) {}
	uint32_t function_name, args;
	bool b_right;

	virtual void operator<<(std::ostream& out) {
		out << type << function_name << args << b_right;
	}

};

struct Args_Block :Block {
	Args_Block() :Block(Block::ARGS) {}
	std::vector<uint32_t>expressions;

	virtual void operator<<(std::ostream& out) {
		out << type << expressions.size();
		for (const uint32_t& ex : expressions) {
			out << ex;
		}
	}
};

struct Break_Block :Block {
	Break_Block() :Block(Block::BREAK) {}

	virtual void operator<<(std::ostream& out) {
		out << type;
	}

};

struct Expression_Term_Block :Block {
	Expression_Term_Block() :Block(Block::EXPRESSION_TERM) {}
	uint32_t left_term, right_term, add_op;
	bool b_right;

	virtual void operator<<(std::ostream& out) {
		out << type << left_term<<right_term<<add_op<<b_right;
	}

};

struct Exoression_Factor_Block :Block {
	Exoression_Factor_Block() :Block(Block::EXPRESSION_FACTOR) {}
	uint32_t left_factor, right_factor, mul_op;
	bool b_right;
	virtual void operator<<(std::ostream& out) {
		out << type << left_factor << right_factor << mul_op << b_right;
	}
};

struct Exoression_Primary_Block : Block {
	Exoression_Primary_Block() :Block(Block::EXPRESSION_PRIMARY) {}
	uint32_t primary_block;

	virtual void operator<<(std::ostream& out) {
		out << type << primary_block;
	}
};

struct Inline_Switch_Case_Block: Block{
	Inline_Switch_Case_Block() : Block(Block::INLINE_SWITCH_CASE) {}
	uint32_t case_expression, return_expression;
	virtual void operator<<(std::ostream& out) {
		out << type << case_expression << return_expression;
	}

};

struct  Inline_Switch_Block: Block {
	Inline_Switch_Block() :Block(Block::INLINE_SWITCH) {}
	
	uint32_t case_expression;
	std::vector<uint32_t>cases;

	virtual void operator<<(std::ostream& out) {
		out << type << case_expression << cases.size();
		for (const uint32_t& c : cases) {
			out << c;
		}
	}

};

struct Inline_If_Block : Block {
	Inline_If_Block() : Block(Block::INLINE_IF) {}
	uint32_t comp_exp;
	
	uint32_t true_expression, false_expression;

	virtual void operator<<(std::ostream& out) {
		out << type<<comp_exp<<true_expression<<false_expression;
	}
};

struct Operator_Block:Block {
	Operator_Block() :Block(Block::OPERATOR) {}
	std::string operator_str;
	virtual void operator<<(std::ostream& out) {
		out << type << operator_str;
	}
};

struct Number_Block:Block {
	Number_Block() :Block(Block::NUMBER) {}
	number_t value;
	virtual void operator<<(std::ostream& out) {
		out << type << value;
	}
};

struct String_Block :Block {
	String_Block() :Block(Block::STRING) {}
	std::string value;
	virtual void operator<<(std::ostream& out) {
		out << type << value;
	}
};

struct Varname_Block:Block {
	Varname_Block() :Block(Block::VARNAME) {}
	std::string varname;
	virtual void operator<<(std::ostream& out) {
		out << type << varname;
	}
};


struct String_Expression_Block :Block {
	String_Expression_Block() :Block(Block::STRING_EXPRESSION) {}
	uint32_t left, right;
	bool is_right;
	virtual void operator<<(std::ostream& out) {
		out << type << left << right<<is_right;
	}
};

struct Function_Block :Block {
	Function_Block() :Block(Block::FUNCTION) {}
	std::vector<uint32_t> inner_blocks;
	std::vector<std::string>varnames;
	
	virtual void operator<<(std::ostream& out) {
		out << type << inner_blocks.size() << varnames.size();

		for (const uint32_t& i : inner_blocks) {
			out << i;
		}
		for (const std::string& i : varnames) {
			out << i;
		}

	}
};

struct Return_Block : Block {
	Return_Block() : Block(Block::RETURN) {}
	uint32_t return_expression;

	virtual void operator<<(std::ostream &out) {
		out << type << return_expression;
	}
};

struct BPL_Program {
	std::map<std::string, Function_Block>program_functions;
	std::vector<uint32_t> program;
	std::vector<Block*> blocks;

	void operator<<(std::ostream& out) {
		
		for (Block* b : blocks) {
			*b << out;
		}
	}

	~BPL_Program() {
		for (Block* b : blocks) {
			delete b;
		}
		blocks.clear();
		program.clear();
		program_functions.clear();
	}

};

struct Parser {

	static BPL_Program  parse_program(const Tokenizer::Tokens& tokens);

private:

	static Tokenizer::Tokens::const_iterator m_end_token;
	static Tokenizer::Tokens::const_iterator m_start_token;
	static Tokenizer::Tokens::const_iterator m_current_token;

	static BPL_Program * m_current_program;

	static uint32_t parse_block();
	static uint32_t	parse_if();
	static uint32_t	parse_elif();
	static uint32_t parse_else();
	static uint32_t parse_loop();
	static uint32_t parse_assignment();
	static uint32_t parse_function_call();
	static uint32_t parse_args();
	static uint32_t parse_break();
	static uint32_t parse_expression_term();
	static uint32_t parse_expression_factor();
	static uint32_t parse_expression_primary();
	static uint32_t parse_number();
	static uint32_t parse_varname();
	static uint32_t parse_inline_switch();
	static uint32_t parse_inline_switch_case();
	static uint32_t parse_inline_if();
	static uint32_t parse_string_expression();
	static uint32_t parse_function_declaration();
	static uint32_t parse_return();
	static uint32_t parse_comp_factor();
	static uint32_t parse_comp_primary();
	static uint32_t parse_operator();


	static bool is_add_op(const std::string& op);
	static bool is_mul_op(const std::string& op);
	static bool is_comp_op(const std::string& op);
	static bool is_assign_op(const std::string& op);
	static bool is_fast_assign_op(const std::string& op);
	static bool is_expression_end_op(const std::string& op);
	static bool is_comp_expression_op(const std::string& op);

	static bool next_token();
	static void prev_token();

	static const Tokenizer::Token& seek_token();
	static const Tokenizer::Token& seek_next();
	static const Tokenizer::Token& seek_prev();

	static bool is_next_token();

	static bool check_tocken(const Tokenizer::Token::Type&t, const std::string& v);

	static bool not_end();
	static bool is_token_type(const Tokenizer::Token::Type&t);
	static bool is_token_value(const std::string&v);

	template<typename T>
	static uint32_t add_block(T* block) {
		m_current_program->blocks.push_back(new T(*block));
		return m_current_program->blocks.size() - 1;
	
	}

	static void add_function_body(const std::string &name, const Function_Block& f) {
		m_current_program->program_functions[name] = f;
	}
};
