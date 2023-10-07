#include "Tokenizer.h"

std::vector<std::string> Tokenizer::keywords({ "if","elif","else","loop","break","?","#","num","str","fn","return"});
std::vector<std::string> Tokenizer::operators({"+","-","*","/","%",",","=",">","<","!",">=","<=","==","!=","->","{","}","(",")",":",";","++","--","+=","-=","/=","*=","..",",","&","|","&&","||"});
std::vector<std::string> Tokenizer::token_names({ "WHITESPACE","KEYWORD","WORD","OPERATOR","NUMBER" });

static char seek_next(const std::string::const_iterator& i, const std::string::iterator& end) {
	if (i + 1 != end) {
		return *(i + 1);
	}
	return 0;
}

static bool is_operator(const std::string& o) {

	for (const std::string& i : Tokenizer::operators) {
		if (i == o)return true;
	}

	return false;
}

static bool is_keyword(const std::string& k) {
	for (const std::string& i : Tokenizer::keywords) {
		if (k == i)return true;
	}
	return false;
}

static bool is_number(const char& c) {
	return (c >= 48 && c <= 57) || c == '.';
}
static bool is_hex_number(const char& c) {
	return (c >= 48 && c <= 57) || c == 'X' || c =='x' || ((c >= 65 && c <= 70) || (c >= 97 && c <= 102));
}


void Tokenizer::Tokenize(const std::string& source, Tokens& Tokens) {
	
	std::string code = "";

	//Prepare code for tockenization
	
	for (std::string::const_iterator c = source.begin(); c != source.end(); c++) {
		if (*c == '"') {

			do{
				code += *c; 
				c++; 

			} while (*c != '"');

			code += *c;
			continue;
		}
		else if (*c > 32 && *c <= 126)code += *c;
	}

	std::string line = "";

	Tokens.clear();

	for (std::string::const_iterator current_char = code.begin(); current_char < code.end(); current_char++) {
	
		//Check for strings
		if (*current_char == '"') {
			current_char++;
			while (*current_char != '"'){
				line += *current_char;
				current_char++;
			}
			Tokens.push_back(Token(line, Token::STRING));
			line.clear();
			continue;
		}


		//Check for operator

		if (is_operator(std::string() + *current_char)) {
			//if it is operator, check if current string is word or keyword
			if (line.length() != 0) {
				if (is_keyword(line)) {
					Tokens.push_back(Token(line, Token::KEYWORD));
					line.clear();
				}
				else {
					Tokens.push_back(Token(line, Token::WORD));
					line.clear();
				}
			}

			//Save operator
			if (is_operator(std::string() + *current_char + seek_next(current_char, code.end()))) {
				Tokens.push_back(Token(std::string() + *current_char + seek_next(current_char, code.end()), Token::OPERATOR));
				current_char++;
			}
			else {
				Tokens.push_back(Token(std::string() + *current_char, Token::OPERATOR));
			}

			line.clear();
			continue;
		}

		//Check for numbers
		if (is_number(*current_char) && line.length() == 0) {

			std::string number = "";

			bool is_hex = false;

			while ((is_number(*current_char) || (is_hex_number(*current_char))) && current_char != code.end()) {
				
				if (*current_char == 'x') is_hex = true;

				number += *current_char;
				current_char++;
			
			}

			current_char--;

			Tokens.push_back(Token(is_hex == true ? std::to_string(std::stoi(number,0,16)) : number, Token::NUMBER));
			line.clear();
			
			continue;

		}

		line += *current_char;
	}

	//Check residues from for loop
	if (is_keyword(line)) {
		Tokens.push_back(Token(line, Token::KEYWORD));
		line.clear();
	}

	else if(line.length() > 0) {
		Tokens.push_back(Token(line, Token::WORD));
		line.clear();
	}

}

void Tokenizer::debug_print(const Tokens& t)
{
	std::cout << "Tokens debug print:\n";

	for (const Token& tv : t) {
		std::cout << token_names[tv.type] << ": " << tv.value << std::endl;
	}
}