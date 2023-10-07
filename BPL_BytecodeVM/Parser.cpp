#include "Parser.h"

Tokenizer::Tokens::const_iterator  Parser::m_end_token = Tokenizer::Tokens::const_iterator();
Tokenizer::Tokens::const_iterator  Parser::m_start_token = Tokenizer::Tokens::const_iterator();
Tokenizer::Tokens::const_iterator  Parser::m_current_token = Tokenizer::Tokens::const_iterator();

BPL_Program* Parser::m_current_program = 0;


bool Parser::is_add_op				(const std::string& op) { return op == "+"	|| op == "-"; }
bool Parser::is_mul_op				(const std::string& op) { return op == "*"	|| op == "/" || op == "%"; }
bool Parser::is_comp_op				(const std::string& op) { return op == "<"	|| op == ">" || op == ">=" || op == "<=" || op == "==" || op == "!="; }
bool Parser::is_assign_op			(const std::string& op) { return op == "="	|| op == "+="|| op == "-=" || op == "*=" || op == "/="; }
bool Parser::is_fast_assign_op		(const std::string& op) { return op == "++"	|| op == "--"; }
bool Parser::is_expression_end_op	(const std::string& op)	{ return op == ","	|| op == ";" || op == "}" || is_comp_op(op); }
bool Parser::is_comp_expression_op	(const std::string& op) { return op == "&&" || op == "||"; }

bool Parser::not_end() {
	return m_current_token != m_end_token;
}

bool Parser::is_next_token() {
	return m_current_token + 1 != m_end_token;
}

bool Parser::next_token() {
	m_current_token++;

	if (m_current_token == m_end_token) {
		return false;
	}

	return true;
}

bool Parser::check_tocken(const Tokenizer::Token::Type& t, const std::string& v) {
	return not_end() && (is_token_type(t) && is_token_value(v));
}

void Parser::prev_token() {
	m_current_token--;
}

bool Parser::is_token_type(const Tokenizer::Token::Type& t) {
	return m_current_token->type == t;
}
bool Parser::is_token_value(const std::string& v) {
	return m_current_token->value == v;
}


const Tokenizer::Token& Parser::seek_token() { return *m_current_token; }

const Tokenizer::Token& Parser::seek_next() { 
	
	if (m_current_token + 1 == m_end_token) {
		std::cout << "Tokens end. Can't seek next token. returning current token\n";
		return *m_current_token;
	}

	return *(m_current_token + 1); 
}
const Tokenizer::Token& Parser::seek_prev() { return *(m_current_token - 1); }

BPL_Program  Parser::parse_program(const Tokenizer::Tokens& tokens) {
	
	m_end_token = tokens.end();
	m_start_token = tokens.begin();
	m_current_token = tokens.begin();

	BPL_Program  return_program;

	m_current_program = &return_program;

	while(not_end()){
		return_program.program.push_back(parse_block());
		next_token();
	}

	return return_program;
}

uint32_t Parser::parse_block() {
	
	switch (seek_token().type) {
	case Tokenizer::Token::KEYWORD:
		
		if(is_token_value("if") || is_token_value("?"))	return parse_if();
		if(is_token_value("loop"))	return parse_loop();
		if(is_token_value("break")) return parse_break();
		if(is_token_value("fn")) return parse_function_declaration();
		if(is_token_value("return")) return parse_return();

		std::cout << "unexpected token in parse_block keyword!\n";
		return 0;
		
		break;
	case Tokenizer::Token::WORD:

		if (!is_next_token()) {
			std::cout << "unexpected token end in parse_block word!\n"; return 0;
			return 0;
		}


		if (seek_next().type != Tokenizer::Token::OPERATOR) {
			std::cout << "unexpected token in parse_block word!\n"; return 0;
			return 0;
		}

		if (seek_next().value == "(" && seek_next().type == Tokenizer::Token::OPERATOR)
			return parse_function_call();

		if (is_assign_op(seek_next().value) || is_fast_assign_op(seek_next().value) ||
			(seek_next().value == ":" && seek_next().type == Tokenizer::Token::OPERATOR) || 
			(seek_next().value == ";" && seek_next().type == Tokenizer::Token::OPERATOR))
				return parse_assignment();
		
		std::cout << "unexpected token in parse_block word!\n"; return 0;
		
		return 0;

		break;
	}
	m_current_token;
	std::cout << "unexpected token in parse_block!\n"; return 0;
}


uint32_t Parser::parse_comp_primary() {

	Comp_Expression_Primary block;

	if (check_tocken(Tokenizer::Token::OPERATOR,"(")) {
		next_token();
		block.left = parse_comp_factor();
	}
	else {
		block.left = parse_expression_term();
	}

	if (seek_token().value == ")" || seek_token().value == "{") {
		block.b_right = false;
		next_token();
		return add_block<Comp_Expression_Primary>(&block);

	}

	if (!is_comp_op(seek_token().value)) {
		std::cout << "unexpected tokens after first expression in parse if!\n" << seek_token().value << std::endl;
		return 0;
	}

	block.b_right = true;
	block.op = parse_operator();
	
	if (check_tocken(Tokenizer::Token::OPERATOR,"(")) {
		next_token();
		block.right = parse_comp_factor();
	}
	else {
		block.right = parse_expression_term();
	}

	return add_block<Comp_Expression_Primary>(&block);

}


uint32_t Parser::parse_comp_factor() {
	
	uint32_t left = parse_comp_primary();
	bool right = false;
	
	m_current_token;

	while (is_token_type(Tokenizer::Token::OPERATOR) && is_comp_expression_op(m_current_token->value) && (!(check_tocken(Tokenizer::Token::OPERATOR, ")") || check_tocken(Tokenizer::Token::OPERATOR, "}")))) {

		right = true;
		
		Comp_Expression_Factor b;
		b.left = left;
		b.op = parse_operator();
		b.right = parse_comp_primary();
		b.b_right = true;

		left = add_block<Comp_Expression_Factor>(&b);
		if (!not_end())
			break;
	}

	if (right) {
		return left;
	}
		
	m_current_token;

	Comp_Expression_Factor b;
	b.left = left;
	b.b_right = false;

	return add_block<Comp_Expression_Factor>(&b);

}



uint32_t Parser::parse_if() {
	
	if_Block i;

	if (!next_token()) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }
	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) { std::cout << "unexpected tokens in parse if!\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }
	
	m_current_token;
	i.comp_exp = parse_comp_factor();
	m_current_token;

	if (!not_end) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }
	if (check_tocken(Tokenizer::Token::OPERATOR, ")"))
		next_token();
	m_current_token;
	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpected tokens in parse if!\n"; return 0; }
	
	m_current_token;
	if (!next_token()) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }
	m_current_token;

	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		i.inner_blocks.push_back(parse_block());
		next_token();
	}

	if (!is_next_token()) {
		return add_block<if_Block>(&i);
	}

	next_token();
	
	bool b_skip = false;

	while(check_tocken(Tokenizer::Token::KEYWORD,"elif")) {
		i.elifs.push_back(parse_elif());
		b_skip = true;
	}

	if (b_skip) {
		next_token();
	}

	bool b_prev = false;

	if (check_tocken(Tokenizer::Token::KEYWORD,"else")) {
		i.else_block = parse_else();
		i.b_else = true;
		b_prev = true;
	}

	if (!b_prev)prev_token();

	return add_block<if_Block>(&i);
}

uint32_t Parser::parse_elif() {

	if_Block i;

	if (!next_token()) { std::cout << "unexpected end of tokens in parse elif!\n"; return 0; }
	if (!check_tocken(Tokenizer::Token::OPERATOR, ":")) { std::cout << "unexpected tokens in parse elif!\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse elif!\n"; return 0; }

	i.comp_exp = parse_comp_factor();

	if (!not_end) { std::cout << "unexpected end of tokens in parse elif!\n"; return 0; }

	if (check_tocken(Tokenizer::Token::OPERATOR, ")"))
		next_token();

	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpected tokens in parse elif!\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse elif!\n"; return 0; }
	
	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		i.inner_blocks.push_back(parse_block());
		next_token();
	}

	return add_block<if_Block>(&i);
}


uint32_t Parser::parse_else() {
	
	Else_block es;
	next_token();
	if (!not_end) { std::cout << "unexpected end of tokens in parse else!\n"; return 0; }
	m_current_token->value;
	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpected tokens in parse else!\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse else!\n"; return 0; }

	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		es.inner_blocks.push_back(parse_block());
		next_token();
	}

	return add_block<Else_block>(&es);
}


uint32_t Parser::parse_loop() {
	
	Loop_Block l;
	
	next_token();

	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpected token in parse loop!\n"; return 0; }

	next_token();

	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		l.inner_blocks.push_back(parse_block());
		next_token();
	}

	return add_block<Loop_Block>(&l);

}
uint32_t Parser::parse_assignment() {

	Assignment_Block a;
	a.varname = parse_varname();


	a.b_fast_assign = false;
	a.b_expression = false;

	if (!is_token_type(Tokenizer::Token::OPERATOR)) { std::cout << "unexpected token in parse assignment!\n"; return 0; }

	if (check_tocken(Tokenizer::Token::OPERATOR,";")) {
		a.b_expression = false;
		return add_block<Assignment_Block>(&a);
	}

	bool is_type_specifed = false;

	if (check_tocken(Tokenizer::Token::OPERATOR, ":")) {
		Assignment_Block::VarType t = Assignment_Block::VarType::NUMBER;
		next_token();
		is_type_specifed = true;
		if (seek_token().value == "str") t = Assignment_Block::VarType::STRING;
		else if(seek_token().value != "num") { std::cout << "unknown type " << seek_token().value << " assignment, setting type to number !\n"; }
		next_token();
		a.var_type = t;
	}


	if (is_fast_assign_op(seek_token().value)) {
		a.b_fast_assign = true;
		a.b_expression = false;
		a.fast_val = seek_token().value == "++" ? 1 : -1;

		next_token();

		return add_block<Assignment_Block>(&a);
	}

	if (!is_assign_op(seek_token().value)) { std::cout << "unexpected token in parse assignment!\n"; return 0; }
	
	next_token();
	/*
	
	if (seek_token().type == Tokenizer::Token::STRING) {
		if (is_type_specifed == false || (is_type_specifed == true && a.var_type == Assignment_Block::VarType::STRING)) {
			a.var_type = Assignment_Block::STRING;
			a.expression = parse_string_expression();
			return add_block<Assignment_Block>(&a);
		}
		else {
			//std::cout << "cannot set variable " << m_current_program->blocks[a.varname] << " to string !\n";
			a.b_expression = false;
			while(seek_token().value!=";")next_token();
			return add_block<Assignment_Block>(&a);
		}
	}
	
	if (!is_type_specifed || (is_type_specifed && a.var_type == Assignment_Block::NUMBER)) {
	
	}
	*/
	a.b_expression = true;
	a.expression = parse_expression_term();

	return add_block<Assignment_Block>(&a);
}

uint32_t Parser::parse_function_declaration() {

	Function_Block function_block;
	
	std::string function_name;

	next_token();
	
	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) {
		std::cout << "unexpected token in function declaration block. Expected ':' instead of " << m_current_token->value << "\n";
		return 0;
	}

	next_token();
	
	if (!is_token_type(Tokenizer::Token::WORD)) {
		std::cout << "unexpected token in function declaration block. Expected function name instead of " << m_current_token->value << " (Type "<<m_current_token->type<<")\n";
		return 0;
	}

	function_name = m_current_token->value;

	next_token();

	if (!check_tocken(Tokenizer::Token::OPERATOR,"(")) {
		std::cout << "unexpected token in function declaration block after function name . Expected '(' instead of " << m_current_token->value << " (Type " << m_current_token->type << ")\n";
		return 0;
	}
	next_token();

	while (!check_tocken(Tokenizer::Token::OPERATOR,")")) {
		
		if (check_tocken(Tokenizer::Token::OPERATOR,",")) {
			next_token();
			continue;
		}

		if (is_token_type(Tokenizer::Token::WORD)) {
			function_block.varnames.push_back(m_current_token->value);
			next_token();
			continue;
		}

		next_token();
		continue;
	}
	next_token();

	m_current_token->value;
	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) {
		std::cout << "unexpected token in function declaration block after varnames. Expected '{' instead of " << m_current_token->value << " (Type " << m_current_token->type << ")\n";
		return 0;
	}
	next_token();

	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		function_block.inner_blocks.push_back(parse_block());
		next_token();
	}

	add_function_body(function_name,function_block);

	None_Block b;
	return add_block<None_Block>(&b);
}


uint32_t Parser::parse_function_call() {

	Function_Call_Block f;
	
	f.function_name = parse_varname();
	f.args = parse_args();
	
	return add_block<Function_Call_Block>(&f);

}
uint32_t Parser::parse_args() {

	if (!check_tocken(Tokenizer::Token::OPERATOR,"(")) { std::cout << "unexpected token in parse args\n"; return 0; }

	Args_Block args;

	next_token();

	while (!check_tocken(Tokenizer::Token::OPERATOR, ")")) {
		args.expressions.push_back(parse_expression_term());
		
		if (check_tocken(Tokenizer::Token::OPERATOR,","))
			next_token();

	}

	next_token();

	return add_block<Args_Block>(&args);
}

uint32_t Parser::parse_break() {

	next_token();
	Break_Block b;
	return add_block<Break_Block>(&b);

}

uint32_t Parser::parse_expression_term() {
	
	
	uint32_t left = parse_expression_factor();
	bool b_right = false;

	while(!check_tocken(Tokenizer::Token::OPERATOR,")") && is_add_op(seek_token().value)) {
		b_right = true;

		Expression_Term_Block e;
		e.left_term = left;
		e.add_op = parse_operator();
		e.right_term = parse_expression_factor();
		e.b_right = true;

		left = add_block<Expression_Term_Block>(&e);

		if (!not_end())break;
	}

	if(b_right) return left;

	Expression_Term_Block e;
	e.left_term = left;
	e.b_right = false;

	return add_block<Expression_Term_Block>(&e);

}
uint32_t Parser::parse_expression_factor() { 

	uint32_t left = parse_expression_primary();
	bool b_right = false;

	m_current_token->value;

	while (!check_tocken(Tokenizer::Token::OPERATOR,")") && is_mul_op(seek_token().value)) {
		b_right = true;
		Exoression_Factor_Block e;
		e.left_factor = left;
		e.mul_op = parse_operator();
		e.right_factor= parse_expression_primary();

		left = add_block<Exoression_Factor_Block>(&e);

		if (!not_end())break;
	}

	if (b_right) return left;

	Exoression_Factor_Block e;
	e.left_factor = left;
	e.b_right = false;

	return add_block<Exoression_Factor_Block>(&e);
}
uint32_t Parser::parse_expression_primary() { 
	
	if (is_token_type(Tokenizer::Token::NUMBER)) {
		Exoression_Primary_Block p;
		p.primary_block = parse_number();
		return add_block<Exoression_Primary_Block>(&p);
	
	}
	if (is_token_type(Tokenizer::Token::STRING)) {
		Exoression_Primary_Block p;
		p.primary_block = parse_string_expression();
		return add_block<Exoression_Primary_Block>(&p);

	}

	if (is_token_type(Tokenizer::Token::WORD)) {

		Exoression_Primary_Block p;
		
		if (seek_next().value == "(") p.primary_block = parse_function_call();
		else p.primary_block = parse_varname();

		return add_block<Exoression_Primary_Block>(&p);
	
	}
	if (check_tocken(Tokenizer::Token::OPERATOR,"(")) {
		next_token();

		Exoression_Primary_Block p;
		p.primary_block = parse_expression_term();

		next_token();

		return add_block<Exoression_Primary_Block>(&p);
	}
	if (is_token_type(Tokenizer::Token::KEYWORD)) {
		if (is_token_value("#")){
			Exoression_Primary_Block p;
			p.primary_block = parse_inline_switch();

			next_token();

			return add_block<Exoression_Primary_Block>(&p);
		}
		else if (is_token_value("?")) {
			Exoression_Primary_Block p;
			p.primary_block = parse_inline_if();

			next_token();

			return add_block<Exoression_Primary_Block>(&p);
		}
	}

	return 0;
}

uint32_t Parser::parse_inline_if() {

	Inline_If_Block block;

	if (!next_token()) { std::cout << "unexpeced end of tokens in inline if!\n"; return 0; }
	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpeced tokens in inline if! expected {, not " << seek_token().value << "\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in inline if!\n"; return 0; }

	block.comp_exp = parse_comp_factor();

	if (!not_end) { std::cout << "unexpected end of tokens in parse inline if!\n"; return 0; }

	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) { std::cout << "unexpected tokens in parse if! expected :, not "<<seek_token().value << "\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }

	block.true_expression = parse_expression_term();
	
	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) { std::cout << "unexpected tokens in parse inline if! expected :, not " << seek_token().value << "\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in parse if!\n"; return 0; }

	block.false_expression = parse_expression_term();

	return add_block<Inline_If_Block>(&block);
}


uint32_t Parser::parse_inline_switch() {

	Inline_Switch_Block switch_block;

	if (!next_token()) { std::cout << "unexpected end of tokens in inline switch!\n"; return 0; }
	if (!check_tocken(Tokenizer::Token::OPERATOR,"{")) { std::cout << "unexpected tokens in inline switch!\n"; return 0; }
	if (!next_token()) { std::cout << "unexpected end of tokens in inline switch!\n"; return 0; }

	switch_block.case_expression = parse_expression_term();

	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) { std::cout << "unexpected tokens in inline switch!\n"; return 0; }

	next_token();

	while (!check_tocken(Tokenizer::Token::OPERATOR,"}")) {
		switch_block.cases.push_back(parse_inline_switch_case());
	}

	return add_block<Inline_Switch_Block>(&switch_block);

}

uint32_t Parser::parse_inline_switch_case() {
	
	Inline_Switch_Case_Block case_block;

	case_block.case_expression = parse_expression_term();

	if (!check_tocken(Tokenizer::Token::OPERATOR,"->")) { std::cout << "unexpected tokens in inline switch case!\n"; return 0; }
	next_token();

	case_block.return_expression = parse_expression_term();

	next_token();
	return add_block<Inline_Switch_Case_Block>(&case_block);

}


uint32_t Parser::parse_number() {
	Number_Block n;
	n.value = std::stof(seek_token().value);
	next_token();

	return add_block<Number_Block>(&n);
}
uint32_t Parser::parse_varname() {

	Varname_Block v;
	v.varname = seek_token().value;
	next_token();

	return add_block<Varname_Block>(&v);
}

uint32_t Parser::parse_operator() {
	Operator_Block op;

	op.operator_str = m_current_token->value;
	next_token();

	return add_block<Operator_Block>(&op);

}

uint32_t Parser::parse_string_expression() {
	String_Block str;
	str.value = seek_token().value;
	next_token();
	return add_block<String_Block>(&str);
}

uint32_t Parser::parse_return() {

	next_token();
	if (!check_tocken(Tokenizer::Token::OPERATOR,":")) { std::cout << "unexpected tokens in parse return!\n"; return 0; }
	Return_Block ret;
	next_token();
	ret.return_expression = parse_expression_term();

	m_current_token->value;

	return add_block<Return_Block>(&ret);
}