#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>
#include "Warehouse.h"

// ---------- 工具函数（输入辅助） ----------
void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string readNonEmptyString(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (!input.empty()) break;
        std::cout << "输入不能为空，请重新输入。\n";
    }
    return input;
}

double readDouble(const std::string& prompt) {
    double val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) break;
        std::cout << "请输入有效的数字。\n";
        clearInput();
    }
    clearInput();
    return val;
}

int readInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) break;
        std::cout << "请输入有效的整数。\n";
        clearInput();
    }
    clearInput();
    return val;
}

int readPositiveInt(const std::string& prompt) {
    int val;
    while (true) {
        val = readInt(prompt);
        if (val > 0) break;
        std::cout << "请输入正整数。\n";
    }
    return val;
}

void pressAnyKeyToContinue() {
    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ---------- 显示商品列表 ----------
void displayGoodsList(const std::vector<Goods>& goodsList, const std::string& title) {
    if (goodsList.empty()) {
        std::cout << "（无商品）\n";
        return;
    }
    std::cout << "\n=== " << title << " ===\n";
    for (const auto& g : goodsList) {
        g.display();
        std::cout << "\n";
    }
}

// ---------- 主菜单 ----------
int main() {
    Warehouse warehouse; // 默认使用 "warehouse.txt"

    Goods::getCurrentDate(); // 初始化当前日期
    // 1. 加载数据
    std::vector<std::string> errors;
    bool loadOk = warehouse.loadData(errors);
    if (!loadOk) {
        std::cout << "[错误]:  数据加载失败（文件严重损坏），程序退出。\n";
        for (const auto& e : errors) std::cout << "  - " << e << "\n";
        return 1;
    }
    if (!errors.empty()) {
        std::cout << "[警告]:  数据加载成功，但存在 " << errors.size() << " 个问题：\n";
        for (const auto& e : errors) std::cout << "  - " << e << "\n";
    }
    else {
        std::cout << "[信息]:  数据加载成功。\n";
    }

    int choice;
    while (true) {
        std::cout << "\n========== 商品销售管理系统 ==========\n";
        std::cout << "  【分类管理】\n";
        std::cout << "  1. 创建分类\n";
        std::cout << "  2. 删除分类（必须为空）\n";
        std::cout << "  3. 重命名分类\n";
        std::cout << "  4. 查看所有分类\n";
        std::cout << "  【商品管理】\n";
        std::cout << "  5. 上架商品（添加）\n";
        std::cout << "  6. 下架商品（删除）\n";
        std::cout << "  7. 更新商品信息\n";
        std::cout << "  8. 移动商品到另一分类\n";
        std::cout << "  【浏览】\n";
        std::cout << "  9.  浏览全部商品\n";
        std::cout << "  10. 按分类浏览\n";
        std::cout << "  11. 按价格区间浏览\n";
        std::cout << "  12. 按库存区间浏览\n";
        std::cout << "  13. 按到货日期区间浏览\n";
        std::cout << "  14. 按过期日期区间浏览\n";
        std::cout << "  15. 浏览无效商品（待修复）\n";
        std::cout << "  【搜索】\n";
        std::cout << "  16. 按编号搜索\n";
        std::cout << "  17. 按名称搜索\n";
        std::cout << "  18. 按生产商搜索\n";
        std::cout << "  【销售】\n";
        std::cout << "  19. 销售商品\n";
        // ====== 新增两个浏览功能 ======
        std::cout << "  【日期浏览】\n";
        std::cout << "  20. 浏览最近到货的商品（按天数）\n";
        std::cout << "  21. 浏览即将过期的商品（按天数）\n";
        // ==============================
        std::cout << "  【系统】\n";
        std::cout << "  0.  保存并退出\n";
        std::cout << "====================================\n";
        std::cout << "请选择操作: ";

        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "无效输入，请输入数字。\n";
            continue;
        }
        clearInput(); // 清除换行

        // ------ 分类管理 ------
        if (choice == 1) {
            std::string name = readNonEmptyString("请输入新分类名称: ");
            if (warehouse.createCategory(name))
                std::cout << "[信息]:  分类创建成功。\n";
            else
                std::cout << "[错误]:  创建失败：分类已存在。\n";
        }
        else if (choice == 2) {
            std::string name = readNonEmptyString("请输入要删除的分类名称: ");
            if (warehouse.deleteCategory(name))
                std::cout << "[信息]:  分类删除成功。\n";
            else
                std::cout << "[错误]:  删除失败：分类不存在或非空。\n";
        }
        else if (choice == 3) {
            std::string oldName = readNonEmptyString("请输入原分类名称: ");
            std::string newName = readNonEmptyString("请输入新分类名称: ");
            if (warehouse.renameCategory(oldName, newName))
                std::cout << "[信息]:  重命名成功。\n";
            else
                std::cout << "[错误]:  重命名失败：原分类不存在或新名称已被占用。\n";
        }
        else if (choice == 4) {
            const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
            if (categories.empty()) {
                std::cout << "暂无分类。\n";
            }
            else {
                std::cout << "\n=== 所有分类 ===\n";
                for (const auto& cat : categories) {
                    std::cout << "  - " << cat.name
                        << "（商品数：" << cat.goodsList.size()
                        << "，最大流水号：" << cat.maxSeq << "）\n";
                }
            }
        }

        // ------ 商品管理 ------
        else if (choice == 5) {
            std::string category = readNonEmptyString("请输入分类名称: ");
            std::string name = readNonEmptyString("请输入商品名称: ");
            double price = readDouble("请输入单价: ");
            std::string manufacturer = readNonEmptyString("请输入生产商: ");
            int stock = readPositiveInt("请输入库存量: ");
            std::string picture = readNonEmptyString("请输入图片路径（没有则直接回车）: ");
            std::string newId = warehouse.addGoods(category, name, price, manufacturer, stock, picture);
            if (newId.empty())
                std::cout << "[错误]:  上架失败：分类不存在或数据不合法。\n";
            else
                std::cout << "[信息]:  上架成功！新商品编号: " << newId << "\n";
        }
        else if (choice == 6) {
            std::string id = readNonEmptyString("请输入要删除的商品编号: ");
            if (warehouse.removeGoods(id))
                std::cout << "[信息]:  下架成功。\n";
            else
                std::cout << "[错误]:  下架失败：商品不存在。\n";
        }
        else if (choice == 7) {
            std::string id = readNonEmptyString("请输入要更新的商品编号: ");
            std::string name = readNonEmptyString("请输入新的商品名称: ");
            double price = readDouble("请输入新的单价: ");
            std::string manufacturer = readNonEmptyString("请输入新的生产商: ");
            int stock = readPositiveInt("请输入新的库存量: ");
            std::string arrivalDate = readNonEmptyString("请输入新的到货日期（YYYY-MM-DD）: ");
            std::string expiryDate = readNonEmptyString("请输入新的过期日期（YYYY-MM-DD）: ");
            Goods updated(id, name, price, manufacturer, stock, arrivalDate, expiryDate, "");
            if (warehouse.updateGoods(id, updated))
                std::cout << "[信息]:  更新成功。\n";
            else
                std::cout << "[错误]:  更新失败：商品不存在或编号不匹配。\n";
        }
        else if (choice == 8) {
            std::string id = readNonEmptyString("请输入要移动的商品编号: ");
            std::string newCategory = readNonEmptyString("请输入目标分类名称: ");
            std::string newId = warehouse.moveGoodsToCategory(id, newCategory);
            if (newId.empty())
                std::cout << "[错误]:  移动失败：商品不存在、无效或目标分类不存在。\n";
            else
                std::cout << "[信息]:  移动成功！新商品编号: " << newId << "\n";
        }

        // ------ 浏览 ------
        else if (choice == 9) {
            auto list = warehouse.browseAll();
            displayGoodsList(list, "全部商品");
        }
        else if (choice == 10) {
            std::string category = readNonEmptyString("请输入分类名称: ");
            auto list = warehouse.browseByCategory(category);
            if (list.empty())
                std::cout << "该分类下没有有效商品。\n";
            else
                displayGoodsList(list, "分类 [" + category + "] 的商品");
        }
        else if (choice == 11) {
            double min = readDouble("请输入最低价格（直接回车使用默认0）: ");
            double max = readDouble("请输入最高价格（直接回车使用默认最大值）: ");
            auto list = warehouse.browseByPriceRange(min, max);
            displayGoodsList(list, "价格区间 [" + std::to_string(min) + ", " + std::to_string(max) + "] 的商品");
        }
        else if (choice == 12) {
            int min = readInt("请输入最低库存（直接回车使用默认0）: ");
            int max = readInt("请输入最高库存（直接回车使用默认最大值）: ");
            auto list = warehouse.browseByStockRange(min, max);
            displayGoodsList(list, "库存区间 [" + std::to_string(min) + ", " + std::to_string(max) + "] 的商品");
        }
        else if (choice == 13) {
            std::string start = readNonEmptyString("请输入起始到货日期（YYYY-MM-DD）: ");
            std::string end = readNonEmptyString("请输入结束到货日期（YYYY-MM-DD）: ");
            auto list = warehouse.browseByArrivalDateRange(start, end);
            displayGoodsList(list, "到货日期区间 [" + start + ", " + end + "] 的商品");
        }
        else if (choice == 14) {
            std::string start = readNonEmptyString("请输入起始过期日期（YYYY-MM-DD）: ");
            std::string end = readNonEmptyString("请输入结束过期日期（YYYY-MM-DD）: ");
            auto list = warehouse.browseByExpiryDateRange(start, end);
            displayGoodsList(list, "过期日期区间 [" + start + ", " + end + "] 的商品");
        }
        else if (choice == 15) {
            auto list = warehouse.browseInvalid();
            displayGoodsList(list, "无效商品（待修复）");
        }

        // ------ 搜索 ------
        else if (choice == 16) {
            std::string id = readNonEmptyString("请输入商品编号: ");
            const Goods* g = warehouse.searchGoodsById(id);
            if (g) {
                std::cout << "\n找到商品：\n";
                g->display();
            }
            else {
                std::cout << "未找到该商品。\n";
            }
        }
        else if (choice == 17) {
            std::string name = readNonEmptyString("请输入商品名称: ");
            auto list = warehouse.searchGoodsByName(name);
            displayGoodsList(list, "名称包含 \"" + name + "\" 的商品");
        }
        else if (choice == 18) {
            std::string manufacturer = readNonEmptyString("请输入生产商: ");
            auto list = warehouse.searchGoodsByManufacturer(manufacturer);
            displayGoodsList(list, "生产商为 \"" + manufacturer + "\" 的商品");
        }

        // ------ 销售 ------
        else if (choice == 19) {
            std::string id = readNonEmptyString("请输入商品编号: ");
            int quantity = readPositiveInt("请输入购买数量: ");
            int newStock = 0;
            SellResult result = warehouse.sellGoods(id, quantity, newStock);
            switch (result) {
            case SellResult::SUCCESS:
                std::cout << "[信息]:  销售成功！剩余库存: " << newStock << "\n";
                break;
            case SellResult::NOT_FOUND:
                std::cout << "[错误]:  销售失败：商品不存在或无效。\n";
                break;
            case SellResult::INSUFFICIENT_STOCK:
                std::cout << "[错误]:  销售失败：库存不足。\n";
                break;
            case SellResult::INVALID_QUANTITY:
                std::cout << "[错误]:  销售失败：数量无效。\n";
                break;
            default: break;
            }
        }

        // ====== 新增功能：按天数浏览最近到货 / 即将过期 ======
        else if (choice == 20) {
            int days = readPositiveInt("请输入天数（最近多少天到货）: ");
            auto list = warehouse.browseByArrivalDateRecent(days);
            displayGoodsList(list, "最近 " + std::to_string(days) + " 天到货的商品");
        }
        else if (choice == 21) {
            int days = readPositiveInt("请输入天数（多少天内过期）: ");
            auto list = warehouse.browseByExpiryDateSoon(days);
            displayGoodsList(list, "即将在 " + std::to_string(days) + " 天内过期的商品");
        }
        // ====================================================

        // ------ 退出 ------
        else if (choice == 0) {
            std::cout << "正在保存数据...\n";
            std::vector<std::string> saveErrors;
            bool saveOk = warehouse.saveData(saveErrors);
            if (!saveOk || !saveErrors.empty()) {
                std::cout << "[错误]:  保存时出现问题：\n";
                for (const auto& e : saveErrors) std::cout << "  - " << e << "\n";
                std::cout << "请检查文件权限。\n";
            }
            else {
                std::cout << "[信息]:  数据已保存。\n";
            }
            std::cout << "感谢使用，再见！\n";
            break;
        }
        else {
            std::cout << "无效选项，请重新选择。\n";
        }

        pressAnyKeyToContinue();
    }

    return 0;
}