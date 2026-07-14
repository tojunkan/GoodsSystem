#include"FilterNode.h"
#include"Condition.h"
#include<variant>
#include<string>
#include<algorithm>
#include<cmath>

LeafNode::LeafNode(const Condition& c):cond(c){}

bool LeafNode::evaluate(const Goods& goods, const std::string& categoryName) const {
	std::variant <
		std::monostate,
		std::string,
		double,
		int
	> actual;

	switch (cond.field)
	{
	case Field::CATEGORY:		actual = categoryName; break;
	case Field::ID:				actual = goods.getId(); break;
	case Field::NAME:			actual = goods.getName(); break;
	case Field::PRICE:			actual = goods.getPrice(); break;
	case Field::STOCK:			actual = goods.getStock(); break;
	case Field::MANUFACTURER:	actual = goods.getManufacturer(); break;
	case Field::ARRIVAL_DATE:	actual = goods.getArrivalDate(); break;
	case Field::EXPIRY_DATE:	actual = goods.getExpiryDate(); break;
	default: return false;
	}

	return std::visit(
		//auto&& 是万能引用
		[&](auto&& actualVal, auto&& expectVal) -> bool {
			using Tactual = std::decay_t<decltype(actualVal)>;//解开引用（如果有）
			using Texpect = std::decay_t<decltype(expectVal)>;

			//如果空直接返回伪
			if constexpr (std::is_same_v<Tactual, std::monostate> || std::is_same_v<Texpect, std::monostate>) {
				return false;
			}
			// 处理字符串操作符（CONTAINS, STARTSWITH）
			//这两个操作符只支持string类型。（is_same_v是is_same<...>::value的语法糖，直接返回布尔值）
			if constexpr (std::is_same_v<Tactual, std::string> && std::is_same_v<Texpect, std::string>) {
				const auto& actualStr = actualVal;
				const auto& expectStr = expectVal;
				switch (cond.op) {
				case Op::CONTAINS:
					return actualStr.find(expectStr) != std::string::npos;
				case Op::STARTSWITH:
					return actualStr.find(expectStr) == 0;
				default:
					// 若不是字符串操作符，则 fall through 到下面的通用比较
					break;
				}
			}
			if constexpr (std::is_same_v<Tactual, Texpect>) {
				switch (cond.op) {
				case Op::EQ: return actualVal == expectVal;
				case Op::NE: return actualVal != expectVal;
				case Op::GT: return actualVal > expectVal;
				case Op::LT: return actualVal < expectVal;
				case Op::GE: return actualVal >= expectVal;
				case Op::LE: return actualVal <= expectVal;
				default: return false;
				}
				
			}
			else { return false; }
		},
	actual,
	cond.value);
}

// ---------- AndNode ----------
void AndNode::addChild(std::unique_ptr<FilterNode> child) {
	children.push_back(std::move(child));
}
bool AndNode::evaluate(const Goods& goods, const std::string& categoryName) const {
	if (children.empty())return false;
	for (const auto& child : children) {
		if (!child->evaluate(goods, categoryName)) return false;
	}
	return true;
}

// ---------- OrNode ----------
void OrNode::addChild(std::unique_ptr<FilterNode> child) {
	children.push_back(std::move(child));
}
bool OrNode::evaluate(const Goods& goods, const std::string& categoryName) const {
	if (children.empty())return false;
	for (const auto& child : children) {
		if (child->evaluate(goods, categoryName)) return true;
	}
	return false;
}

// ---------- NotNode ----------
NotNode::NotNode(std::unique_ptr<FilterNode> c) : child(std::move(c)) {}
bool NotNode::evaluate(const Goods& goods, const std::string& categoryName) const {
	return !child->evaluate(goods, categoryName);
}