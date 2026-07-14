#include "Parse.h"
#include "Condition.h"
#include "Goods.h"
#include "Warehouse.h"
#include <string>
#include <vector>
#include <stack>
#include <cctype>
#include <sstream>
#include <cassert>

/**一个条件的格式：
* 查询        ::=  (AND 条件列表) | (OR 条件列表) | (NOT 条件) | 原子条件
* 条件列表    ::=  (条件1, 条件2 ...)
* 原子条件    ::=  (字段 操作符 值)
* 字段        ::=  "category" | "id" | "name" | "price" | "stock" | "manufacturer" | "arrival_date" | "expiry_date"
* 操作符      ::=  "equal" | "not_equal" | "greater_than" | "less_than" | "greater_equal" | "less_equal" | "contains" | "startswith"
* 例如：(AND (AND (price greater_than 10.0), (price less_than 50.0)), (category contains 华为), (arrival_date greater_than 2026-05-20))
* 如果一个字符串内部有空格，那么应该用双引号括起来。
*/

Parse::Parse(const Warehouse& wh) :warehouse(wh) {}

const FilterNode* Parse::getRoot() { return root.get(); }

void Parse::parse(const std::string& expression, std::vector<std::string>& errorLines) {
	errorLines.clear();
	pos = 0;
	if(!Parse::tokenize(expression))root = nullptr;
	else root = parseExpression(errorLines);
}

bool Parse::tokenize(const std::string& expression) {
	if (expression.empty())return false;
	auto CleanUpSpaces = [](const std::string expression) -> std::vector<std::string> {
		bool isProtected = false;
		std::vector<std::string> ans;
		std::string tmp = "";
		for (auto ch : expression) {
			if (ch == '"') {
				isProtected = !isProtected;
				continue;
			}
			if (!isProtected && (ch == '(' || ch == ')') ){
				ans.push_back(std::string(1, ch));
				tmp = "";
			}
			else if (((isspace(ch) && !isProtected)|| ch == ',' )&& !tmp.empty()) {
				ans.push_back(tmp);
				tmp = "";
			}
			else {
				tmp += ch;
			}
		}
		if (!tmp.empty())ans.push_back(tmp);
		return ans;
	};
	auto exp = CleanUpSpaces(expression);
	tokens.resize(exp.size() + 1);
	std::string tmp;
	for (int i = 0; i < exp.size(); ++i) {
		tmp = exp[i];
		if (tmp == "(")tokens[i] = { Token::Type::LPARENTHESIS, tmp };
		else if (tmp == ")")tokens[i] = { Token::Type::RPARENTHESIS, tmp };
		else {
			try {
				std::stod(tmp);
				tokens[i] = { Token::Type::NUMBER, tmp };
			}
			catch (const std::invalid_argument)	{
				tokens[i] = { Token::Type::STRING, tmp };
			}
		}
	}
	tokens[exp.size()] = { Token::Type::END, "" };
	return true;
}

Token Parse::peek() {
	if (pos >= tokens.size())return { Token::Type::END, "" };
	//assert(pos < tokens.size() && pos >= 0, "invalid index.");
	return tokens[pos];
}
Token Parse::consume() {
	if (pos >= tokens.size())return { Token::Type::END, "" };
	//assert(pos < tokens.size() && pos >= 0, "invalid index.");
	return tokens[pos++];
}

bool Parse::match(Token::Type type) {
	return peek().type == type;
}

std::unique_ptr<LeafNode> Parse::makeLeaf(std::vector<Token>& atomCond, std::vector<std::string>& errorLines) {
	auto Error = [&](const std::string& err) -> std::unique_ptr<LeafNode> {
		errorLines.push_back(err);
		return std::make_unique<LeafNode>(INVALID_CONDITION);
		};
	if (atomCond.size() != 3) {
		return Error("原子条件的参数数量不正确。");
	}
	Condition ans;
	Token top = atomCond[0];
	ans.field = fieldFromString(top.val);
	top = atomCond[1];
	ans.op = opFromString(top.val);
	if (ans.op == Op::UNKNOWN)return Error("未识别到正确的操作符");
	top = atomCond[2];
	switch (ans.field)
	{
	case Field::ID:
		if (!(ans.op == Op::EQ || ans.op == Op::NE))return Error("关于ID的条件只支持等于/不等于");
		if (!Goods::isValidId(top.val))return Error("该ID不合法");
		ans.value = top.val;
		break;
	case Field::NAME:
		if (!(ans.op == Op::EQ || ans.op == Op::NE || ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于商品名称的条件只支持：等于/不等于/包含/以……开头");
		ans.value = top.val; break;
	case Field::MANUFACTURER: 
		if (!(ans.op == Op::EQ || ans.op == Op::NE || ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于供货商的条件只支持：等于/不等于/包含/以……开头");
		ans.value = top.val; break;
	case Field::CATEGORY:
		if (!(ans.op == Op::EQ || ans.op == Op::NE))return Error("关于分区的条件只支持等于/不等于");
		if (!warehouse.getCategoryByName(top.val))return Error("该分区不合法");
		ans.value = top.val;
		break;
	case Field::PRICE: {
			if ((ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于价格的条件不支持：包含/以……开头");
			if (top.type != Token::Type::NUMBER)return Error("价格必须是数字");
			double p = std::stod(top.val);
			if (!Goods::isValidPrice(p))return Error("价格必须是非负的");
			ans.value = p;
		}
		break;
	case Field::STOCK: {
			if ((ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于库存的条件不支持：包含/以……开头");
			if (top.type != Token::Type::NUMBER)return Error("库存必须是数字");
			int s = std::stoi(top.val);
			if (!Goods::isValidStock(s))return Error("库存必须是非负的");
			ans.value = s;
		}
		break;
	case Field::ARRIVAL_DATE:
		if ((ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于上次进货时间的条件不支持：包含/以……开头");
		if (!Goods::isValidDate(top.val))return Error("上次进货时间不合法");
		if (top.val > Goods::CURRENT_DATE)return Error("进货时间不能晚于今天");
		ans.value = top.val;
		break;
	case Field::EXPIRY_DATE:
		if ((ans.op == Op::CONTAINS || ans.op == Op::STARTSWITH))return Error("关于最早过期时间的条件不支持：包含/以……开头");
		if (!Goods::isValidDate(top.val))return Error("最早过期时间不合法");
		ans.value = top.val;
		break;
	default:
		return Error("未识别到正确的字段");
	}
	return std::make_unique<LeafNode>(ans);
}

std::unique_ptr<FilterNode> Parse::parseExpression(std::vector<std::string>& errorLines) {
	//Query 在处理这里返回的空指针的时候应该看一眼是否有errorLines，没有的话默认选择全部
	if (tokens.empty()) return nullptr;
	if (!match(Token::Type::LPARENTHESIS)) {
		errorLines.push_back("解析崩溃：条件应该以左英文圆括号开头，即\"(\"");
		return nullptr;
	}
	consume();

	Token first = peek();
	if (!match(Token::Type::STRING)) {
		errorLines.push_back("解析崩溃：括号内应该以字符串开头");
		return nullptr;
	}
	consume();
	if (first.val == "AND") {
		auto node = std::make_unique<AndNode>();

		while (!match(Token::RPARENTHESIS) && !match(Token::END)) {
			auto child = parseExpression(errorLines); // 递归解析子表达式
			// 即使 child 为 nullptr，我们也跳过该子表达式，但错误已收集
			if (child) {
				node->addChild(std::move(child));
			} // 若为 nullptr，说明子表达式解析失败，错误已记录，我们继续解析下一个
		}
		if (!match(Token::RPARENTHESIS)) {
			errorLines.push_back("解析崩溃：条件应该以右英文圆括号结尾，即\")\"");
			// 为了恢复，我们可以继续，但这里可能无法恢复，选择返回 nullptr
			return nullptr;
		}
		consume();
		if (node->children.size()<2) {
			errorLines.push_back("解析崩溃：AND 缺少子表达式");
			return nullptr;
		}
		return node;
	} else 	if (first.val == "OR") {
		auto node = std::make_unique<OrNode>();

		while (!match(Token::RPARENTHESIS) && !match(Token::END)) {
			auto child = parseExpression(errorLines); // 递归解析子表达式
			// 即使 child 为 nullptr，我们也跳过该子表达式，但错误已收集
			if (child) {
				node->addChild(std::move(child));
			} // 若为 nullptr，说明子表达式解析失败，错误已记录，我们继续解析下一个
		}
		if (!match(Token::RPARENTHESIS)) {
			errorLines.push_back("解析崩溃：条件应该以右英文圆括号结尾，即\")\"");
			// 为了恢复，我们可以继续，但这里可能无法恢复，选择返回 nullptr
			return nullptr;
		}
		consume();
		if (node->children.size() < 2) {
			errorLines.push_back("解析崩溃：OR 缺少子表达式");
			return nullptr;
		}
		return node;
	}
	else if (first.val == "NOT") {
		auto child = parseExpression(errorLines);
		if (!child) {
			// 子表达式失败，但我们可以继续？NOT 必须有一个子表达式，若失败则无法构造，只能返回 nullptr
			errorLines.push_back("解析崩溃：NOT 缺少子表达式");
			return nullptr;
		}
		if (!match(Token::RPARENTHESIS)) {
			errorLines.push_back("解析崩溃：条件应该以右英文圆括号结尾，即\")\"");
			return nullptr;
		}
		consume();
		return std::make_unique<NotNode>(std::move(child));
	}
	else {
		std::vector<Token> atom;
		while (!match(Token::Type::RPARENTHESIS) && !match(Token::Type::END))atom.push_back(consume());
		auto node = makeLeaf(atom, errorLines);

		if (!match(Token::RPARENTHESIS)) {
			errorLines.push_back("解析崩溃：条件应该以右英文圆括号结尾，即\")\"");
			return nullptr;
		}
		consume();
		return node;
	}
}