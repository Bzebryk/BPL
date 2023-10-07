#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <stack>
#include <cstdio>

typedef float number_t;
typedef std::string string_t;

struct Tokenizer {
	static std::vector<std::string>keywords;
	static std::vector<std::string>operators;
	static std::vector<std::string>token_names;

	struct Token {
		enum Type {
			WHITESPACE,
			KEYWORD,
			WORD,
			OPERATOR,
			NUMBER,
			STRING
		};
		Token(const std::string& v = "", const Type& t = WHITESPACE) :type(t), value(v) {
		}

		Type type;
		std::string value;
	};

	typedef std::vector<Token> Tokens;

	static void Tokenize(const std::string& source, Tokens& tockens);

	static void debug_print(const Tokens& t);
};
