#include "Query.h"

Query::Query(const Warehouse& wh) : warehouse(wh), parser(wh){}

const Parse& Query::getParser() { return parser; }

std::vector<Warehouse::GoodsWithCategory> Query::queryBySExpr(
    const std::string& sexpr,
    std::vector<std::string>& errorLines
) { 
    parser.parse(sexpr, errorLines);
    if (errorLines.empty())return execute(parser.getRoot());
    else return {};
}

std::vector<Warehouse::GoodsWithCategory> Query::execute(const FilterNode* node) const {
    if (!node) return warehouse.browseAll();
    return warehouse.findGoodsIf(
        [node](const Goods& goods, const std::string& categoryName) ->bool {
            return node->evaluate(goods, categoryName);
        }
    );
}