#pragma once
#include <string>
#include <vector>
#include <stack>
#include "FilterNode.h"
#include "Condition.h"
#include "Warehouse.h"

struct Token {
	enum Type{
		LPARENTHESIS,
		RPARENTHESIS,
		STRING,
		NUMBER,
		END
	};
	Type type;
	std::string val;
};

class Parse {
private:
	const Warehouse& warehouse;
	std::vector<Token> tokens;
	size_t pos = 0;
	std::unique_ptr<FilterNode> root;

	bool tokenize(const std::string& expression);
	Token peek();
	Token consume();
	bool match(Token::Type type);
	std::unique_ptr<LeafNode> makeLeaf(std::vector<Token>& atomCond, std::vector<std::string>& errorLines);
	std::unique_ptr<FilterNode> parseExpression(std::vector<std::string>& errorLines);

public:
	Parse(const Warehouse& wh);
	const FilterNode* getRoot();
	void parse(const std::string& expression, std::vector<std::string>& errorLines);
};