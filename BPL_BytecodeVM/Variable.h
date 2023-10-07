#pragma once
#include "Parser.h"
#include <variant>

struct Variable {

	enum Type { VOID, NUMBER, ARRAY, STRING };

	Variable() { set<float>(NUMBER, 0); }
	Variable(const float& v) { set<float>(NUMBER, v); }
	Variable(const std::string& str) { set<std::string>(STRING, str); }

	Type type;
	std::variant<number_t, string_t> value;

	template<typename T>
	T& get_as() { return std::get<T>(value); }

	template<typename T>
	const T& getc_as() const { return std::get<T>(value); }

	template<typename T>
	void set(const Type& t, const T& v) {
		value = v;
		type = t;
	}

	void operator=(const number_t& v) { set<number_t>(NUMBER, v); }
	void operator=(const string_t& v) { set<string_t>(STRING, v); }
	void operator=(const char* v) { set<string_t>(STRING, v); }

	Variable operator+(const Variable& v) { 
		if(this->type == NUMBER && this->type == v.type)
			return this->getc_as<number_t>() + v.getc_as<number_t>(); 

		Variable tmp("");

		if (this->type == STRING) tmp.get_as<string_t>() += this->getc_as<string_t>();
		else tmp.get_as<string_t>() += std::to_string(this->getc_as<number_t>());

		if (v.type == STRING) tmp.get_as<string_t>() += v.getc_as<string_t>();
		else tmp.get_as<string_t>() += std::to_string(v.getc_as<number_t>());
		
		return tmp;
	}

	Variable operator-(const Variable& v) { return this->getc_as<number_t>() - v.getc_as<number_t>(); }
	Variable operator*(const Variable& v) { return this->getc_as<number_t>() * v.getc_as<number_t>(); }
	Variable operator/(const Variable& v) { return this->getc_as<number_t>() / v.getc_as<number_t>(); }
	Variable operator %(const Variable& v) { return (int)this->getc_as<number_t>() % (int)v.getc_as<number_t>(); }

	void operator+=(const Variable& v) { *this = *this + v; }
	void operator-=(const Variable& v) { *this = *this - v; }
	void operator*=(const Variable& v) { *this = *this * v; }
	void operator/=(const Variable& v) { *this = *this / v; }

	bool operator ==(const Variable& v) {
		if (this->type == v.type) {
			if (this->type == NUMBER)return this->getc_as<number_t>() == v.getc_as<number_t>();
			return this->getc_as<string_t>() == v.getc_as<string_t>();
		}
		return false;
	}

	bool operator >(const Variable& v) { return this->getc_as<number_t>() > v.getc_as<number_t>(); }
	bool operator <(const Variable& v) { return this->getc_as<number_t>() < v.getc_as<number_t>(); }
	bool operator >=(const Variable& v) { return this->getc_as<number_t>() >= v.getc_as<number_t>(); }
	bool operator <=(const Variable& v) { return this->getc_as<number_t>() <= v.getc_as<number_t>(); }
	bool operator !=(const Variable& v) { return this->getc_as<number_t>() != v.getc_as<number_t>(); }

};
