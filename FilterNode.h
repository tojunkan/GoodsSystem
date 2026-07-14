#pragma once
#include<vector>
#include"Condition.h"
#include"Goods.h"

struct FilterNode {
	virtual ~FilterNode() = default;
	virtual bool evaluate(const Goods& goods, const std::string& categoryName) const = 0;
};

struct LeafNode : public FilterNode {
	Condition cond;
	explicit LeafNode(const Condition& c);
	bool evaluate(const Goods& goods, const std::string& categoryName) const override;
};

struct AndNode : public FilterNode {
	std::vector<std::unique_ptr<FilterNode>> children;
	void addChild(std::unique_ptr<FilterNode> child);
	bool evaluate(const Goods& goods, const std::string& categoryName) const override;
};

struct OrNode : public FilterNode {
	std::vector<std::unique_ptr<FilterNode>> children;
	void addChild(std::unique_ptr<FilterNode> child);
	bool evaluate(const Goods& goods, const std::string& categoryName) const override;
};

struct NotNode : public FilterNode {
	std::unique_ptr<FilterNode> child;
	explicit NotNode(std::unique_ptr<FilterNode> c);
	bool evaluate(const Goods& goods, const std::string& categoryName) const override;
};