#pragma once
#include<string>
#include<variant>

enum class Field {
	CATEGORY,
	ID,
	NAME,
	PRICE,
	STOCK,
	MANUFACTURER,
	ARRIVAL_DATE,
	EXPIRY_DATE,
	UNKNOWN
};

enum class Op {
	EQ,
	NE,
	GT,
	LT,
	GE,
	LE,
	CONTAINS,
	STARTSWITH,
	UNKNOWN
};

struct Condition {
	Field field;
	Op op;
	std::variant<
		std::monostate,
		std::string,
		double,
		int
	> value;
};

extern Condition INVALID_CONDITION;

Field fieldFromString(const std::string& s);
Op opFromString(const std::string& s);