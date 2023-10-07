#pragma once
#include "Variable.h"

typedef Variable(*Interpreter_host_func)(const uint8_t& argc, Variable* args);
typedef std::unordered_map<std::string, Variable> Variables;
typedef std::unordered_map<std::string, Interpreter_host_func> Host_Functions;


struct Context {

	Variable* get_variable(const std::string& var_name, uint32_t counter = 0) {

		if (up_context != nullptr) {
			Variable* v = up_context->get_variable(var_name, counter + 1);

			if (v != nullptr) { return v; }

			if (counter == 0) { return &vars[var_name]; }

			try {
				return &vars.at(var_name);
			}
			catch (...) {
				return nullptr;
			}
		}

		try {
			return &vars.at(var_name);
		}
		catch (...) {
			if (counter == 0) return &vars[var_name];
			return nullptr;
		}

	}

	Interpreter_host_func get_host_function(const std::string& func_name, uint32_t c = 0) {

		if (up_context != nullptr) {
			Interpreter_host_func func = up_context->get_host_function(func_name, c + 1);
			if (func != nullptr) { return func; }
			try {
				if (c == 0) {
					return host_functions.at(func_name);
				}
			}
			catch (...) {
				//std::cout << "Host function with name " << func_name << " not declared\n";
				return nullptr;
			}
		}
		else {
			try {
				return host_functions.at(func_name);
			}
			catch (...) {
				//std::cout << "Host function with name " << func_name << " not declared\n";
				return nullptr;
			}
		}


	}

	void register_host_func(const std::string& name, Interpreter_host_func f) { host_functions[name] = f; }

	void breake(const bool& b = true) {
		breake_flag = b;
		if (up_context != nullptr)up_context->breake(b);
	}

	void returned(const bool& b = true) {
		return_flag = b;
		if (up_context != nullptr)up_context->return_flag = b;
	}

	const Host_Functions* cpy_host_functions() {
		if (up_context != nullptr) {
			return up_context->cpy_host_functions();

		}
		return &host_functions;
	}

	~Context() {
		vars.clear();
		host_functions.clear();
		up_context = nullptr;
	}

	void set_return_value(const Variable& v) {
		if (up_context != nullptr) {
			up_context->set_return_value(v);
		}
		else {
			return_value = v;
		}
	}

	Variable get_return_value() {
		if (up_context != nullptr) {
			return up_context->get_return_value();
		}
		return return_value;
	}

	Variables vars;
	Host_Functions host_functions;

	Context* up_context;
	bool breake_flag;
	bool return_flag;
	Variable return_value;

	Context(Context* up_ctx = nullptr) :up_context(up_ctx), breake_flag(false), return_flag(false) {}

};