#include "Interpreter.h"

#include <fstream>
#include <sstream>
#include <Windows.h>

Variable cls(const uint8_t& argc, Variable* args) {
	system("cls");
	return 0;
}

Variable test_print(const uint8_t& argc, Variable* args) {
	
	for (int i = 0; i < argc; i++) {
		if(args[i].type == Variable::NUMBER)
		std::cout << args[i].getc_as<number_t>() << (i == argc - 1 ? "." : ", ");
		else
		std::cout << args[i].getc_as<string_t>() << " ";

	}
	std::cout << std::endl;

	return 0;
}
Variable get_type(const uint8_t& argc, Variable* args) {
	
	if (argc > 0) {
		return args[0].type;
	}
	else return 0;
}

Variable test_get(const uint8_t& argc, Variable* args) {
	number_t i;
	std::cin >> i;
	return Variable(i);
}
Variable test_clock(const uint8_t& argc, Variable* args) {
	return Variable(number_t(clock()));
}
Variable test_system(const uint8_t& argc, Variable* args) {
	system(args[0].getc_as<string_t>().c_str());
	return Variable(0);
}

int main() {

	std::fstream file;
	file.open("Examples/test.bpl", std::ios::in);
	std::stringstream ss;
	ss << file.rdbuf();

	Tokenizer::Tokens tokens;

	Tokenizer::Tokenize(ss.str(), tokens);

	BPL_Program test_parser_program = Parser::parse_program(tokens);

	Context ctx;

	ctx.register_host_func("print",test_print);
	ctx.register_host_func("get",test_get);
	ctx.register_host_func("cls",cls);
	ctx.register_host_func("get_type",get_type);
	ctx.register_host_func("clock",test_clock);
	ctx.register_host_func("sys", test_system);

	Interpreter::execute(test_parser_program,ctx);

}