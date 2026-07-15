#include "Warehouse.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_set>

Warehouse::Warehouse(const std::string& fname) : filename(fname), isDirty(false) {
}

bool Warehouse::getDirtiness() { return this->isDirty; }

void Warehouse::makeDirty() { this->isDirty = true; }

bool Warehouse::loadData(std::vector<std::string>& errorLines) {
    //  1. 打开文件 
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        errorLines.push_back("无法打开文件: " + filename);
        return false;
    }

    // 工具 Lambda: 读取下一行非注释非空的有效行
    // 用法: 每次调用会读取一行, 跳过空行和注释行, 将有效行存入 line
    // 返回 true 表示读到有效行, false 表示文件已结束
    std::string line;
    int lineNum = 0;  // 当前行号（用于错误定位）
    auto readNonCommentLine = [&]() -> bool {
        while (std::getline(inFile, line)) {
            ++lineNum;

            // ---- 去除行首空白字符（空格/制表符） ----
            size_t start = line.find_first_not_of(" \t");

            // 整行都是空白字符 -> 视为空行, 继续读取下一行
            if (start == std::string::npos) {
                continue;
            }

            // 如果第一个非空白字符是 '#', 说明是注释行, 跳过
            if (line[start] == '#') {
                continue;
            }

            // 否则是有效行, 返回 true
            return true;
        }
        // 文件读取完毕
        return false;
        };

    // 2. 读取分区总数（第一行有效数据）
    if (!readNonCommentLine()) {
        errorLines.push_back("文件为空或缺少分区总数");
        return false;
    }

    std::istringstream firstLine(line);
    int categoryCount = 0;
    if (!(firstLine >> categoryCount) || categoryCount <= 0) {
        errorLines.push_back("第 " + std::to_string(lineNum) + " 行: 分区总数必须为正整数");
        return false;
    }

    // 3. 清空旧数据并预分配空间
    categories.clear();
    categories.reserve(categoryCount);
    categoryPrefixMap.clear();
	categoryPrefixMap.reserve(categoryCount);

    // 临时存储每个分区的期望商品数量（只在 loadData 中使用）
    std::vector<int> expectedCounts;
    expectedCounts.reserve(categoryCount);

    // 4. 读取每个分区的元数据（分区总数 行）
    for (int i = 0; i < categoryCount; ++i) {
        if (!readNonCommentLine()) {
            errorLines.push_back("文件提前结束: 预期 " + std::to_string(categoryCount) +
                " 个分区, 实际只读到 " + std::to_string(i) + " 个");
            return false;
        }

        std::istringstream iss(line);
        std::string categoryName, prefixNumStr, totalNumberStr, maxSeqStr;
        int prefixNum = -1;
        int totalNumber = -1;
        int maxSeq = -1;

        // 解析: 分类名称 \t 前缀编号 \t 期望商品数 \t 当前最大流水号
        if (!(std::getline(iss, categoryName, '\t') &&
              std::getline(iss, prefixNumStr, '\t') &&
              std::getline(iss, totalNumberStr, '\t') &&
              std::getline(iss, maxSeqStr, '\t'))) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 分区元数据格式错误，需要 4 个字段: 名称 前缀编号 商品总数 最大流水号");
            return false;
        }
        try {
            prefixNum = std::stoi(prefixNumStr);
        }         catch (...) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 前缀编号无法转换为整数 (实际: " + prefixNumStr + ")");
            return false;
		}
        try {
            totalNumber = std::stoi(totalNumberStr);
        }		 catch (...) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 商品总数无法转换为整数 (实际: " + totalNumberStr + ")");
            return false;
		}
        try {
            maxSeq = std::stoi(maxSeqStr);
        }         catch (...) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 最大流水号无法转换为整数 (实际: " + maxSeqStr + ")");
            return false;
        }

        // 校验数值合法性
        if (categoryName.empty() || prefixNum < 0 || totalNumber < 0 || maxSeq < 0 || totalNumber > maxSeq) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 分区元数据包含非法值 (名称不可为空, 数值不可为负, 商品总数不可大于最大流水号)");
            return false;
        }

        // 检查编号是否与顺序一致（强制连续，避免 vector 索引混乱）
        if (prefixNum != i + 1) {
            errorLines.push_back("第 " + std::to_string(lineNum) +
                " 行: 分区编号不连续，期望 " + std::to_string(i + 1) +
                " 实际 " + std::to_string(prefixNum));
            return false;
        }

        // 构建 Category 对象并存入
        categories.emplace_back(categoryName, maxSeq, totalNumber);
        categoryPrefixMap[categoryName] = prefixNum;
        expectedCounts.push_back(totalNumber);
    }

    // 5. 读取商品数据（逐分区、逐行）
    for (int catIdx = 0; catIdx < categoryCount; ++catIdx) {
        int expected = expectedCounts[catIdx];
        Category& cat = categories[catIdx];

        // 读取 expected 行商品数据
        for (int goodsIdx = 0; goodsIdx < expected; ++goodsIdx) {
            if (!readNonCommentLine() || line[0] == '-') {
                errorLines.push_back("文件提前结束: 分区 [" + cat.name +
                    "] 预期 " + std::to_string(expected) +
                    " 件商品, 实际只读到 " + std::to_string(goodsIdx) + " 件");
                // 这里已经读取数量不足了，属于文件结构错误，返回 false
                return false;
            }

            double price = 0.0;
            int stock = 0;
            std::istringstream iss(line);
            std::string categoryNameFromLine, id, name, manufacturer, priceStr, stockStr, arrivalDate, expiryDate;
            if (!(std::getline(iss, categoryNameFromLine, '\t') &&
                std::getline(iss, id, '\t') &&
                std::getline(iss, name, '\t') &&
                std::getline(iss, priceStr, '\t') &&
                std::getline(iss, manufacturer, '\t') &&
                std::getline(iss, stockStr, '\t') &&
                std::getline(iss, arrivalDate, '\t') &&
                std::getline(iss, expiryDate, '\t'))) {

            // 解析: 分类名称 \t 商品ID \t 名称 \t 单价 \t 生产商 \t 库存量 \t 到货日期 \t 过期日期
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: 商品数据字段不足，需要 8 个字段: 分类名称 ID 名称 单价 生产商 库存量 到货日期 过期日期，读取到：" + line);
                // 构造一个占位空商品，标记为无效，保证数量匹配
                try {
                    price = std::stod(priceStr);
				}
                catch (...) {
                    price = 0.0;
                }
                try
                {
					stock = std::stoi(stockStr);
                }
                catch (...)
                {
                    stock = 0;
                }
                Goods emptyGoods(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
                cat.goodsList.push_back({ emptyGoods, false });
                continue;
            }


            // 转换 priceStr, stockStr
            try { 
                price = std::stod(priceStr); 
            }
            catch (...) {
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: 单价字段无法转换为数字 (实际: " + priceStr + ")");
                price = 0.0; // 赋默认值，继续处理
			}
            try {
                stock = std::stoi(stockStr);
            }
            catch (...) {
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: 库存字段无法转换为数字 (实际: " + stockStr + ")");
                stock = 0; // 赋默认值，继续处理
            }

            // ---- 校验1: 分类名称是否与当前分区匹配 ----
            if (categoryNameFromLine != cat.name) {
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: 分类名称不匹配 (期望: " + cat.name +
                    ", 实际: " + categoryNameFromLine + ")");
                // 虽然不匹配，但我们仍按当前分区存入（因为循环就是按分区走的）
                // 但标记为无效
                Goods invalidGoods(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
                cat.goodsList.push_back({ invalidGoods, false });
                continue;
            }

            // ---- 校验2: ID 前缀是否与分类编号匹配 ----
            int expectedPrefix = categoryPrefixMap[cat.name];
            int actualPrefix = 0;
            try {
                actualPrefix = std::stoi(id.substr(0, 2));
            }
            catch (...) {
                // 截取失败说明 ID 长度不足2位
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: 商品ID长度不足，无法提取前缀");
                Goods invalidGoods(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
                cat.goodsList.push_back({ invalidGoods, false });
                continue;
            }

            if (actualPrefix != expectedPrefix) {
                errorLines.push_back("第 " + std::to_string(lineNum) +
                    " 行: ID前缀不匹配 (期望: " + std::to_string(expectedPrefix) +
                    ", 实际: " + std::to_string(actualPrefix) + ")");
                Goods invalidGoods(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
                cat.goodsList.push_back({ invalidGoods, false });
                continue;
            }

            // ---- 校验3: 调用 Goods::isValidFields 检查字段合法性 ----
            std::string fieldErr = Goods::isValidFields(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
            bool valid = fieldErr.empty();
            if (!valid) {
                errorLines.push_back("第 " + std::to_string(lineNum) + " 行: " + fieldErr);
            }

            // ---- 构造商品并存入 ----
            Goods goods(id, name, price, manufacturer, stock, arrivalDate, expiryDate);
            cat.goodsList.push_back({ goods, valid });
        }

        // ---- 可选校验: 实际读取数量是否等于期望值 ----
        // 实际上，如果上面的循环没有因为文件提前结束而 return，那读取数量一定等于 expected
        // 但万一 file 中有多余的空行注释被忽略了，循环结束后数量是准确的
        // 如果担心文件末尾有额外的商品行，可以继续读但忽略，这里不做处理

		//每个分区结束后应当以一个分割线分割
        if (inFile.peek() != '-')
        {
			errorLines.push_back("第 " + std::to_string(lineNum) + " 行: 分区 [" + cat.name + "] 后缺少分割行分隔，或可能商品数量与分区元数据不匹配，文件格式错误");
            return false;
        }
        else {
            std::getline(inFile, line);// 读取分隔线避免影响下一次读取
        }
    }

    // ========================================================
    // 6. 加载完成
    // ========================================================
    // 如果 errorLines 非空，说明存在校验失败的商品，但文件结构完整
    // 返回 true 表示加载成功（调用者可根据 errorLines 是否为空判断有无警告）
    return true;
}

// 保存到文件
bool Warehouse::saveData(std::vector<std::string>& errorLines) {
    if (!isDirty) {
        return true;
    }
    std::ofstream outFile(filename);
    if (!outFile) {
        errorLines.push_back("无法保存文件，请检查文件路径或权限");
        return false;
    }
    outFile << "# ================================================================\n";
    outFile << "# 商品仓库数据文件 V2.0\n";
    outFile << "# 编码：UTF-8\n";
    outFile << "# 分类名称是语义化字符串，与商品ID前缀（01/02）建立映射关系\n";
    outFile << "# 以“#”开头的行是注释行，系统会忽略注释行和空行，\\t表示制表符，\\n表示换行符\n";
    outFile << "# 但保存文件以后所有自行添加的注释行会被删除，空行会被清理掉\n";
    outFile << "# ================================================================\n";
    outFile << "# ================================================================\n\n";
    outFile << "# ---------- 1. 分区总数 ----------\n";
	outFile << categories.size() << "\n";


    outFile << "\n# ---------- 2. 分区元数据清单 ----------\n";
    outFile << "# 格式：分类名称\t商品ID前缀(2位)\t期望商品总数\t当前最大流水号\n";
    for (Category cat : categories) {
		outFile << cat.name << "\t" << categoryPrefixMap[cat.name] << "\t" << cat.goodsList.size() << "\t" << cat.maxSeq << "\n";
    }
	outFile << "\n# ---------- 3. 商品数据 ----------------\n";
	for (const auto& cat : categories) {
        outFile << "# ---------- 分区: " << cat.name << " ----------\n";
        outFile << "# 格式：分类名称\t商品ID\t商品名称\t单价\t生产商\t库存量\t到货日期\t过期日期\n";
        for (const auto& tmp : cat.goodsList) {
            Goods goods = tmp.first;
            outFile << cat.name << "\t" << goods.getId() << "\t" << goods.getName() << "\t" 
                << goods.getPrice() << "\t" << goods.getManufacturer() << "\t" 
                << goods.getStock() << "\t" << goods.getArrivalDate() << "\t" << goods.getExpiryDate() << "\n";
        }
        outFile << "---------- 分区结束 ----------\n";
    }
    outFile.close();
	isDirty = false; // 保存后标记为干净
    return true;
}

std::vector<std::string> Warehouse::validateData() const {
    std::vector<std::string> invalidIds;
    for (const auto& category : categories) {
        for (const auto& tmp : category.goodsList) {
            if (tmp.second == false) continue;
            auto& goods = tmp.first;
            std::string error = Goods::isValidFields(goods.getId(), goods.getName(), goods.getPrice(), goods.getManufacturer(), goods.getStock(), goods.getArrivalDate(), goods.getExpiryDate());
            if(error != "") {
                invalidIds.push_back(goods.id);
            }
        }
    }
    return invalidIds;
}

bool Warehouse::createCategory(const std::string& categoryName) {
    if (categoryPrefixMap.find(categoryName) != categoryPrefixMap.end()) {
        return false; // 分类已存在
    }
    size_t newPrefix = categories.size() + 1; // 新分类编号为当前数量 + 1
    categories.emplace_back(categoryName);
    categoryPrefixMap[categoryName] = static_cast<int>(newPrefix);
    isDirty = true; // 数据已修改
    return true;
}

bool Warehouse::deleteCategory(const std::string& categoryName) {
    auto it = std::find_if(categories.begin(), categories.end(), [&](const Category& cat) {
        return cat.name == categoryName;
    });
    if (it == categories.end()) {
        return false; // 分类不存在
    }
    if (!it->goodsList.empty()) {
        return false; // 分类下有商品，拒绝删除
    }
    categories.erase(it);
    categoryPrefixMap.erase(categoryName);
    isDirty = true; // 数据已修改
    return true;
}

bool Warehouse::renameCategory(const std::string& oldName, const std::string& newName) {
    if (categoryPrefixMap.find(newName) != categoryPrefixMap.end()) {
        return false; // 新名称已存在
    }
    auto it = std::find_if(categories.begin(), categories.end(), [&](const Category& cat) {
        return cat.name == oldName;
    });
    if (it == categories.end()) {
        return false; // 旧分类不存在
    }
    it->name = newName;
    size_t prefixNum = categoryPrefixMap[oldName];
    categoryPrefixMap.erase(oldName);
    categoryPrefixMap[newName] = static_cast<int>(prefixNum);
    isDirty = true; // 数据已修改
    return true;
}

const std::vector<Warehouse::Category>& Warehouse::getCategories() const {
	return categories;
}

const Warehouse::Category* Warehouse::getCategoryByName(const std::string& categoryName) const {
	auto idx = categoryPrefixMap.find(categoryName);
    if (idx != categoryPrefixMap.end()) {
        return &categories[idx->second - 1]; // 返回指向分类的指针
    }
    return nullptr; // 分类不存在
}

std::vector<const Warehouse::Category*> Warehouse::getCategoryByNameFuzzy(const std::string& categoryName) const {
	std::vector<const Category*> results;
	for (const auto& category : categories) {
		if (category.name.find(categoryName) != std::string::npos) {
			results.push_back(&category);
		}
	}
	return results;
}

// 添加商品
std::string Warehouse::addGoods(const std::string& categoryName,
                                const std::string& name,
                                double price,
                                const std::string& manufacturer,
                                int stock,
                                const std::string& arrivalDate,
                                const std::string& expiryDate,
                                const std::string& picture) {
    auto idx = categoryPrefixMap.find(categoryName);
    Category* Cat = nullptr;
    if (idx != categoryPrefixMap.end()) {
        Cat = &categories[idx->second - 1]; // 返回指向分类的指针
    }
    else return "";
	int prefixNum = categoryPrefixMap[categoryName], nextnum = categories[prefixNum - 1].maxSeq + 1;
    std::string id;
    char buf[7];
    snprintf(buf, sizeof(buf), "%02d%04d", prefixNum, nextnum);
    std::string raw6(buf);
	id = Goods::generateFullId(raw6);
    if (!Goods::isValidFields(id, name, price, manufacturer, stock, arrivalDate, expiryDate).empty())return "";
	Goods newGoods = Goods(id, name, price, manufacturer, stock, arrivalDate, expiryDate, picture);

    Cat->goodsList.push_back(std::make_pair(newGoods, true));
    ++(Cat->maxSeq);
	isDirty = true; // 数据已修改
    return id;
}

//std::optional<Warehouse::GoodsWithCategory> Warehouse::searchGoodsById(const std::string& id, std::string* err) const {
//    std::string error = "";
//    Warehouse::GoodsWithCategory ans;
//    if (Goods::isValidId(id) == false) {
//        error = "id不合法。";
//        if(err)*err = error;
//        return std::nullopt;
//    }
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if (tmp.second && tmp.first.getId() == id) {
//                ans.CategoryName = category.name;
//                ans.goods = tmp.first;
//                if(err)*err = error;
//                return ans;
//            }
//        }
//    }
//    error = "未找到该商品。";
//    if(err)*err = error;
//    return std::nullopt; // 未找到
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::searchGoodsByName(const std::string& name) const {
//    std::vector<Warehouse::GoodsWithCategory> results;
//    Warehouse::GoodsWithCategory ans;
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if (tmp.second && tmp.first.getName() == name) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//                results.push_back(ans);
//            }
//        }
//    }
//    return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::searchGoodsByNameFuzzy(const std::string& name) const {
//	std::vector<Warehouse::GoodsWithCategory> results;
//    Warehouse::GoodsWithCategory ans;
//	for (const auto& category : categories) {
//		for (const auto& tmp : category.goodsList) {
//			if (tmp.second && tmp.first.getName().find(name) != std::string::npos) {
//                ans.CategoryName = category.name;
//                ans.goods = tmp.first;
//				results.push_back(ans);
//			}
//		}
//	}
//	return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::searchGoodsByManufacturer(const std::string& manufacturer) const {
//    std::vector<Warehouse::GoodsWithCategory> results;
//    Warehouse::GoodsWithCategory ans;
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if (tmp.second && tmp.first.getManufacturer() == manufacturer) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//                results.push_back(ans);
//            }
//        }
//    }
//    return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::searchGoodsByManufacturerFuzzy(const std::string& manufacturer) const {
//    std::vector<Warehouse::GoodsWithCategory> results;
//    Warehouse::GoodsWithCategory ans;
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if (tmp.second && tmp.first.getManufacturer().find(manufacturer) != std::string::npos) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//                results.push_back(ans);
//            }
//        }
//    }
//    return results;
//}
//
std::vector<Warehouse::GoodsWithCategory> Warehouse::browseAll() const {
    std::vector<Warehouse::GoodsWithCategory> allGoods;
    Warehouse::GoodsWithCategory ans;
    for (const auto& category : categories) {
        for (const auto& tmp : category.goodsList) {
            if (tmp.second) { 
                ans.CategoryName = category.name, ans.goods = tmp.first;
                allGoods.push_back(ans); 
            }
        }
    }
    return allGoods;
}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByCategory(const std::string& categoryName, std::string* err) const {
//    std::string error = "";
//    std::vector<Warehouse::GoodsWithCategory> results;
//    if (this->categoryPrefixMap.find(categoryName) == categoryPrefixMap.end())
//    {
//        error = "不存在该分区。";
//        if(err)*err = error;
//        return results;
//    }
//    Warehouse::GoodsWithCategory ans;
//    ans.CategoryName = categoryName;
//    for (const auto& category : categories) {
//        if (category.name == categoryName) {
//            for (const auto& tmp : category.goodsList) {
//                if (tmp.second) {
//                    ans.goods = tmp.first;
//                    results.push_back(ans);
//                }
//            }
//            break;
//        }
//    }
//    if(err)*err = error;
//    return results;
//}
//
std::vector<Goods> Warehouse::browseInvalid() const {
    std::vector<Goods> invalidGoods;
    for (const auto& category : categories) {
        for (const auto& tmp : category.goodsList) {
            if (!tmp.second) {
                invalidGoods.push_back(tmp.first);
            }
        }
    }
    return invalidGoods;
}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByPriceRange(double minPrice, double maxPrice, std::string* err) const {
//    std::string error = "";
//    std::vector<Warehouse::GoodsWithCategory> results;
//    if (!Goods::isValidPrice(minPrice) || !Goods::isValidPrice(maxPrice) || minPrice > maxPrice) {
//        error = "价格区间不合法。";
//        if(err)*err = error;
//        return results;
//    }
//    Warehouse::GoodsWithCategory ans;
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if(tmp.second && tmp.first.getPrice() >= minPrice && tmp.first.getPrice() <= maxPrice) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//                results.push_back(ans);
//            }
//        }
//    }
//    if(err)*err = error;
//    return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByStockRange(int minStock, int maxStock, std::string* err) const {
//    std::string error = "";
//    std::vector<Warehouse::GoodsWithCategory> results;
//    if (!Goods::isValidStock(minStock) || !Goods::isValidStock(maxStock) || minStock > maxStock) {
//        error = "库存区间不合法。";
//        if(err)*err = error;
//        return results;
//    }
//    Warehouse::GoodsWithCategory ans;
//    for (const auto& category : categories) {
//        for (const auto& tmp : category.goodsList) {
//            if(tmp.second && tmp.first.getStock() >= minStock && tmp.first.getStock() <= maxStock) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//                results.push_back(ans);
//            }
//        }
//    }
//    if(err)*err = error;
//    return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByArrivalDateRange(const std::string& startDate, const std::string& endDate, std::string* err) const {
//    std::string error = "";
//	std::vector<Warehouse::GoodsWithCategory> results;
//    if (!Goods::isValidDate(startDate) || !Goods::isValidDate(endDate) || startDate > endDate || endDate > Goods::CURRENT_DATE) {
//        error = "日期区间不合法。";
//        if(err)*err = error;
//        return results;
//    }
//    Warehouse::GoodsWithCategory ans;
//	for (const auto& category : categories) {
//		for (const auto& tmp : category.goodsList) {
//			if (tmp.second && tmp.first.getArrivalDate() >= startDate && tmp.first.getArrivalDate() <= endDate) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//				results.push_back(ans);
//			}
//		}
//	}
//    if(err)*err = error;
//	return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByExpiryDateRange(const std::string& startDate, const std::string& endDate, std::string* err) const {
//    std::string error = "";
//    std::vector<Warehouse::GoodsWithCategory> results;
//    if (!Goods::isValidDate(startDate) || !Goods::isValidDate(endDate) || startDate > endDate || endDate > Goods::CURRENT_DATE) {
//        error = "日期区间不合法。";
//        if(err)*err = error;
//        return results;
//    }
//    Warehouse::GoodsWithCategory ans;
//	for (const auto& category : categories) {
//		for (const auto& tmp : category.goodsList) {
//			if (tmp.second && tmp.first.getExpiryDate() >= startDate && tmp.first.getExpiryDate() <= endDate) {
//                ans.CategoryName = category.name, ans.goods = tmp.first;
//				results.push_back(ans);
//			}
//		}
//	}
//    if(err)*err = error;
//	return results;
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByArrivalDateRecent(int days, std::string* err) const {
//	std::string start = Goods::DaysOffset(Goods::CURRENT_DATE, -days);
//	return browseByArrivalDateRange(start, Goods::CURRENT_DATE, err);
//}
//
//std::vector<Warehouse::GoodsWithCategory> Warehouse::browseByExpiryDateSoon(int days, std::string* err) const {
//	std::string end = Goods::DaysOffset(Goods::CURRENT_DATE, days);
//	return browseByExpiryDateRange(Goods::CURRENT_DATE, end, err);
//}

Goods* Warehouse::findAuxiliary(const std::string& id, int& categoryIndex, int& goodsIndex, bool invalid_filter) {
    for (auto index = 0; index != categories.size(); ++index) {
        for (auto gIndex = 0; gIndex != categories[index].goodsList.size(); ++gIndex) {
            if (categories[index].goodsList[gIndex].first.getId() == id && (!invalid_filter || categories[index].goodsList[gIndex].second)) {
				categoryIndex = index;
				goodsIndex = gIndex;
                return &categories[index].goodsList[gIndex].first;
            }
        }
    }
    return nullptr; // 未找到
}

bool Warehouse::removeGoods(const std::string& id) {
	int categoryIndex = -1, goodsIndex = -1;
	Goods* goodsPtr = findAuxiliary(id, categoryIndex, goodsIndex, false);
	if (!goodsPtr) {
        return false; // 未找到
    }
	categories[categoryIndex].goodsList.erase(categories[categoryIndex].goodsList.begin() + goodsIndex);
	isDirty = true; // 数据已修改
	return true;
}

bool Warehouse::updateGoods(const std::string& id, const Goods& updatedGoods, std::string* err) {
    std::string error;
    int categoryIndex = -1, goodsIndex = -1;
    Goods* goodsPtr = findAuxiliary(id, categoryIndex, goodsIndex, false);
    if (!goodsPtr) {
        error = "未找到商品。";
        if (err)*err = error;
        return false; // 未找到
    }
    categories[categoryIndex].goodsList[goodsIndex].first = updatedGoods;
    // 重新计算有效性
    auto& entry = categories[categoryIndex].goodsList[goodsIndex];
    error = Goods::isValidFields(
        updatedGoods.getId(),
        updatedGoods.getName(),
        updatedGoods.getPrice(),
        updatedGoods.getManufacturer(),
        updatedGoods.getStock(),
		updatedGoods.getArrivalDate(),
        updatedGoods.getExpiryDate()
    );
    entry.second = error.empty();
    if(err)*err = error;
	isDirty = true; // 数据已修改
    return true;
}

std::string Warehouse::moveGoodsToCategory(const std::string& id, const std::string& newCategoryName) {
    int categoryIndex = -1, goodsIndex = -1;
	Goods* goodsPtr = findAuxiliary(id, categoryIndex, goodsIndex, true);//不支持移动无效商品
    if (!goodsPtr) {
        return ""; // 未找到
    }
	auto newCatcheck = categoryPrefixMap.find(newCategoryName);
	if (newCatcheck == categoryPrefixMap.end()) {
        return ""; // 新分类不存在
    }
	auto newCatIt = categories.begin() + (newCatcheck->second - 1);
    // 移动商品到新分类
	Goods movedGoods = categories[categoryIndex].goodsList[goodsIndex].first;
    std::string newid = addGoods(newCategoryName, movedGoods.getName(), movedGoods.getPrice(), movedGoods.getManufacturer(), movedGoods.getStock(), movedGoods.getArrivalDate(), movedGoods.getExpiryDate(), movedGoods.getPicture());
    if(newid == "") {
		return  ""; // 添加到新分类失败
    }
    categories[categoryIndex].goodsList.erase(categories[categoryIndex].goodsList.begin() + goodsIndex);
	isDirty = true; // 数据已修改
    return newid; // 返回新id表示成功
}

bool Warehouse::sellGoods(const std::string& id, int quantity, int& newStock, std::string* err) {
    std::string error = "";
    int categoryIndex = -1, goodsIndex = -1;
    Goods* goodsPtr = findAuxiliary(id, categoryIndex, goodsIndex, true);//不支持销售无效商品
    if (!goodsPtr) {
        error = "未找到商品";
        if(err)*err = error;
        return false; // 未找到
    }
    if (quantity <= 0) {
        error = "输入数量不合法";
        if(err)*err = error;
        return false;
    }
    if (goodsPtr->getStock() < quantity) {
        error = "库存不足";
        if(err)*err = error;
        return false;
    }
    // 扣减库存
    goodsPtr->setStock(goodsPtr->getStock() - quantity);
    newStock = goodsPtr->getStock();
    isDirty = true; // 数据已修改
    if(err)*err = error;
    return true;
}

void Warehouse::GoodsWithCategory::display(std::ostream& os) const {
    os << "--------------------------------------------------\n";
    os << "分区: " << this->CategoryName << "\n";
    os << "编号: " << this->goods.id << "\n";
    os << "名称: " << this->goods.name << "\n";
    os << "单价: " << this->goods.price << " 元\n";
    os << "厂商: " << this->goods.manufacturer << "\n";
    os << "库存: " << this->goods.stock << "\n";
    os << "到货日期: " << this->goods.arrivalDate << "\n";
    if (this->goods.expiryDate != Goods::DEFAULT_EXPIRY_DATE) os << "保质期: " << this->goods.expiryDate << "\n";
    else os << "保质期: 长期\n";
    os << "--------------------------------------------------\n";
}