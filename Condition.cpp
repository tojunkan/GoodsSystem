/**一个条件的格式：
* 查询        ::=  (AND 条件列表) | (OR 条件列表) | (NOT 条件) | 原子条件
* 条件列表    ::=  {条件1, 条件2}
* 原子条件    ::=  (字段 操作符 值)
* 字段        ::=  "category" | "id" | "name" | "price" | "stock" | "manufacturer" | "arrival_date" | "expiry_date"
* 操作符      ::=  "equal" | "not_equal" | "greater_than" | "less_than" | "greater_equal" | "less_equal" | "contains" | "startswith"
* 注：这里的解析不考虑校验，校验逻辑请见Query.cpp文件。
*/

#include<string>
#include"Condition.h"

Condition INVALID_CONDITION = { Field::UNKNOWN, Op::UNKNOWN, std::monostate{} };

Field fieldFromString(const std::string& s) {
	if (s == "category")return Field::CATEGORY;
	else if (s == "id")return Field::ID;
	else if (s == "name")return Field::NAME;
	else if (s == "price")return Field::PRICE;
	else if (s == "stock")return Field::STOCK;
	else if (s == "manufacturer")return Field::MANUFACTURER;
	else if (s == "arrival_date")return Field::ARRIVAL_DATE;
	else if (s == "expiry_date")return Field::EXPIRY_DATE;
	else return Field::UNKNOWN;
}

Op opFromString(const std::string& s) {
	if (s == "equal")return Op::EQ;
	else if (s == "not_equal")return Op::NE;
	else if (s == "greater_than")return Op::GT;
	else if (s == "less_than")return Op::LT;
	else if (s == "greater_equal")return Op::GE;
	else if (s == "less_equal")return Op::LE;
	else if (s == "contains")return Op::CONTAINS;
	else if (s == "startswith")return Op::STARTSWITH;
	else return Op::UNKNOWN;
}