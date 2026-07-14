// Query.h
#pragma once
#include <vector>
#include <string>
#include "Warehouse.h"
#include "FilterNode.h"
#include "Parse.h"

class Query {
public:
    explicit Query(const Warehouse& wh);

    std::vector<Warehouse::GoodsWithCategory> queryBySExpr(
        const std::string& sexpr,
        std::vector<std::string>& errorLines
    );

    const Parse& getParser();

private:
    const Warehouse& warehouse;
    Parse parser;
    std::vector<Warehouse::GoodsWithCategory> execute(const FilterNode* node) const;
};