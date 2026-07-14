#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <cmath>
#include "Warehouse.h"
#include "Statistics.h"
#include "BackendAPI.h"

namespace fs = std::filesystem;

// ---------- 常量 ----------
const std::string WAREHOUSE_DIR = "./warehouses";

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
void displayGoodsList(const std::vector<Warehouse::GoodsWithCategory>& goodsList, const std::string& title) {
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

// ---------- 统计结果打印辅助函数 ----------
void printOverallStatistics(const OverallStatistics& stats) {
    std::cout << "\n========== 整体统计 ==========\n";
    std::cout << "  有效商品总数 : " << stats.totalGoods << "\n";
    std::cout << "  分类总数     : " << stats.totalCategories << "\n";
    std::cout << "  总库存量     : " << stats.totalStock << "\n";
    std::cout << "  总价值       : " << std::fixed << std::setprecision(2) << stats.totalValue << "\n";
    std::cout << "  平均单价     : " << stats.averagePrice << "\n";
}

void printCategoryStatistics(const std::vector<CategoryStatistics>& stats) {
    std::cout << "\n========== 分类统计 ==========\n";
    if (stats.empty()) {
        std::cout << "（无分类）\n";
        return;
    }
    std::cout << std::left << std::setw(20) << "分类名称"
        << std::setw(12) << "商品数"
        << std::setw(12) << "总库存"
        << std::setw(15) << "总价值" << "\n";
    std::cout << std::string(60, '-') << "\n";
    for (const auto& s : stats) {
        std::cout << std::left << std::setw(20) << s.categoryName
            << std::setw(12) << s.goodsCount
            << std::setw(12) << s.stockSum
            << std::fixed << std::setprecision(2) << std::setw(15) << s.valueSum << "\n";
    }
}

void printPriceHistogram(const std::vector<PriceInterval>& hist) {
    std::cout << "\n========== 价格直方图 ==========\n";
    if (hist.empty()) {
        std::cout << "（无商品）\n";
        return;
    }
    std::cout << std::left << std::setw(15) << "价格区间"
        << std::setw(10) << "数量" << "\n";
    std::cout << std::string(30, '-') << "\n";
    for (const auto& h : hist) {
        std::string range;
        if (std::isinf(h.upperBound)) {
            range = "≥ " + std::to_string(h.lowerBound);
        }
        else {
            range = std::to_string(h.lowerBound) + " - " + std::to_string(h.upperBound);
        }
        std::cout << std::left << std::setw(15) << range
            << std::setw(10) << h.count << "\n";
    }
}

void printStockDistribution(const StockDistribution& dist) {
    std::cout << "\n========== 库存分布 ==========\n";
    std::cout << "  低库存（≤ 阈值）   : " << dist.lowCount << "\n";
    std::cout << "  中库存（阈值之间） : " << dist.mediumCount << "\n";
    std::cout << "  高库存（> 阈值）   : " << dist.highCount << "\n";
}

void printManufacturerStatistics(const std::vector<ManufacturerStatistics>& stats) {
    std::cout << "\n========== 生产商统计 ==========\n";
    if (stats.empty()) {
        std::cout << "（无商品）\n";
        return;
    }
    std::cout << std::left << std::setw(25) << "生产商"
        << std::setw(12) << "商品数"
        << std::setw(15) << "总价值" << "\n";
    std::cout << std::string(55, '-') << "\n";
    for (const auto& s : stats) {
        std::cout << std::left << std::setw(25) << s.manufacturer
            << std::setw(12) << s.goodsCount
            << std::fixed << std::setprecision(2) << std::setw(15) << s.totalValue << "\n";
    }
}

// ---------- 主函数 ----------
int main() {
    apiCreateWarehouseFile(WAREHOUSE_DIR + "/default.txt");
    auto files = apiScanWarehouseFiles(WAREHOUSE_DIR);
    if (files.empty()) {
        files.push_back("warehouse.txt");
    }

    std::cout << "\n请选择要打开的仓库：\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << files[i] << "\n";
    }
    int choice;
    while (true) {
        choice = readInt("请输入编号: ");
        if (choice >= 1 && choice <= static_cast<int>(files.size()))
            break;
        std::cout << "无效编号，请重新输入。\n";
    }
    std::string selectedFile = files[choice - 1];
    std::string fullPath = WAREHOUSE_DIR + "/" + selectedFile;
    if (!fs::exists(fullPath)) {
        fullPath = selectedFile;
    }

    std::unique_ptr<Warehouse> warehouse = std::make_unique<Warehouse>(fullPath);
    Goods::getCurrentDate();

    std::vector<std::string> errors;
    bool loadOk = apiLoadWarehouse(*warehouse, fullPath, errors);
    if (!loadOk) {
        std::cout << "[错误]: 数据加载失败（文件严重损坏），程序退出。\n";
        for (const auto& e : errors) std::cout << "  - " << e << "\n";
        return 1;
    }
    if (!errors.empty()) {
        std::cout << "[警告]: 数据加载成功，但存在 " << errors.size() << " 个问题：\n";
        for (const auto& e : errors) std::cout << "  - " << e << "\n";
    }
    else {
        std::cout << "[信息]: 数据加载成功。\n";
    }

    int menuChoice;
    std::string errorMsg;
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
        std::cout << "  【日期浏览】\n";
        std::cout << "  20. 浏览最近到货的商品（按天数）\n";
        std::cout << "  21. 浏览即将过期的商品（按天数）\n";
        std::cout << "  【仓库管理】\n";
        std::cout << "  22. 切换仓库\n";
        std::cout << "  23. 创建新仓库\n";
        std::cout << "  【统计】\n";
        std::cout << "  24. 整体统计\n";
        std::cout << "  25. 分类统计\n";
        std::cout << "  26. 价格直方图\n";
        std::cout << "  27. 库存分布\n";
        std::cout << "  28. 生产商统计\n";
        std::cout << "  【系统】\n";
        std::cout << "  0.  保存并退出\n";
        std::cout << "====================================\n";
        std::cout << "请选择操作: ";

        if (!(std::cin >> menuChoice)) {
            clearInput();
            std::cout << "无效输入，请输入数字。\n";
            continue;
        }
        clearInput();

        switch (menuChoice) {
        case 1: {
            std::string name = readNonEmptyString("请输入新分类名称: ");
            if (apiCreateCategory(*warehouse, name))
                std::cout << "[信息]: 分类创建成功。\n";
            else
                std::cout << "[错误]: 创建失败：分类已存在。\n";
            break;
        }
        case 2: {
            std::string name = readNonEmptyString("请输入要删除的分类名称: ");
            if (apiDeleteCategory(*warehouse, name))
                std::cout << "[信息]: 分类删除成功。\n";
            else
                std::cout << "[错误]: 删除失败：分类不存在或非空。\n";
            break;
        }
        case 3: {
            std::string oldName = readNonEmptyString("请输入原分类名称: ");
            std::string newName = readNonEmptyString("请输入新分类名称: ");
            if (apiRenameCategory(*warehouse, oldName, newName))
                std::cout << "[信息]: 重命名成功。\n";
            else
                std::cout << "[错误]: 重命名失败：原分类不存在或新名称已被占用。\n";
            break;
        }
        case 4: {
            auto names = apiListCategories(*warehouse);
            if (names.empty()) {
                std::cout << "暂无分类。\n";
            }
            else {
                std::cout << "\n=== 所有分类 ===\n";
                for (const auto& n : names) {
                    std::cout << "  - " << n << "\n";
                }
            }
            break;
        }
        case 5: {
            std::string category = readNonEmptyString("请输入分类名称: ");
            std::string name = readNonEmptyString("请输入商品名称: ");
            double price = readDouble("请输入单价: ");
            std::string manufacturer = readNonEmptyString("请输入生产商: ");
            int stock = readPositiveInt("请输入库存量: ");
            std::string arrivalDate = readNonEmptyString("请输入到货日期（YYYY-MM-DD）: ");
            std::string expiryDate = readNonEmptyString("请输入过期日期（YYYY-MM-DD）: ");
            std::string picture = readNonEmptyString("请输入图片路径（没有则直接回车）: ");
            std::string newId = apiAddGoods(*warehouse, category, name, price, manufacturer, stock,
                arrivalDate, expiryDate, picture);
            if (newId.empty())
                std::cout << "[错误]: 上架失败：分类不存在或数据不合法。\n";
            else
                std::cout << "[信息]: 上架成功！新商品编号: " << newId << "\n";
            break;
        }
        case 6: {
            std::string id = readNonEmptyString("请输入要删除的商品编号: ");
            if (apiRemoveGoods(*warehouse, id))
                std::cout << "[信息]: 下架成功。\n";
            else
                std::cout << "[错误]: 下架失败：商品不存在。\n";
            break;
        }
        case 7: {
            std::string id = readNonEmptyString("请输入要更新的商品编号: ");
            std::string name = readNonEmptyString("请输入新的商品名称: ");
            double price = readDouble("请输入新的单价: ");
            std::string manufacturer = readNonEmptyString("请输入新的生产商: ");
            int stock = readPositiveInt("请输入新的库存量: ");
            std::string arrivalDate = readNonEmptyString("请输入新的到货日期（YYYY-MM-DD）: ");
            std::string expiryDate = readNonEmptyString("请输入新的过期日期（YYYY-MM-DD）: ");
            Goods updated(id, name, price, manufacturer, stock, arrivalDate, expiryDate, "");
            if (apiUpdateGoods(*warehouse, id, updated, errorMsg))
                std::cout << "[信息]: 更新成功。\n";
            else
                std::cout << "[错误]: " << errorMsg << "\n";
            break;
        }
        case 8: {
            std::string id = readNonEmptyString("请输入要移动的商品编号: ");
            std::string newCategory = readNonEmptyString("请输入目标分类名称: ");
            std::string newId = apiMoveGoods(*warehouse, id, newCategory);
            if (newId.empty())
                std::cout << "[错误]: 移动失败：商品不存在、无效或目标分类不存在。\n";
            else
                std::cout << "[信息]: 移动成功！新商品编号: " << newId << "\n";
            break;
        }
        case 9: {
            auto list = apiBrowseAll(*warehouse);
            displayGoodsList(list, "全部商品");
            break;
        }
        case 10: {
            std::string category = readNonEmptyString("请输入分类名称: ");
            auto list = apiBrowseByCategory(*warehouse, category, errorMsg);
            if (!errorMsg.empty()) {
                std::cout << "[错误]: " << errorMsg << "\n";
            }
            else if (list.empty()) {
                std::cout << "该分类下没有有效商品。\n";
            }
            else {
                displayGoodsList(list, "分类 [" + category + "] 的商品");
            }
            break;
        }
        case 11: {
            double min = readDouble("请输入最低价格: ");
            double max = readDouble("请输入最高价格: ");
            auto list = apiBrowseByPriceRange(*warehouse, min, max, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "价格区间 [" + std::to_string(min) + ", " + std::to_string(max) + "] 的商品");
            break;
        }
        case 12: {
            int min = readInt("请输入最低库存: ");
            int max = readInt("请输入最高库存: ");
            auto list = apiBrowseByStockRange(*warehouse, min, max, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "库存区间 [" + std::to_string(min) + ", " + std::to_string(max) + "] 的商品");
            break;
        }
        case 13: {
            std::string start = readNonEmptyString("请输入起始到货日期（YYYY-MM-DD）: ");
            std::string end = readNonEmptyString("请输入结束到货日期（YYYY-MM-DD）: ");
            auto list = apiBrowseByArrivalDateRange(*warehouse, start, end, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "到货日期区间 [" + start + ", " + end + "] 的商品");
            break;
        }
        case 14: {
            std::string start = readNonEmptyString("请输入起始过期日期（YYYY-MM-DD）: ");
            std::string end = readNonEmptyString("请输入结束过期日期（YYYY-MM-DD）: ");
            auto list = apiBrowseByExpiryDateRange(*warehouse, start, end, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "过期日期区间 [" + start + ", " + end + "] 的商品");
            break;
        }
        case 15: {
            auto list = apiBrowseInvalid(*warehouse);
            displayGoodsList(list, "无效商品（待修复）");
            break;
        }
        case 16: {
            std::string id = readNonEmptyString("请输入商品编号: ");
            auto opt = apiSearchById(*warehouse, id, errorMsg);
            if (opt.has_value()) {
                std::cout << "\n找到商品：\n";
                opt->display();
            }
            else {
                std::cout << "[错误]: " << errorMsg << "\n";
            }
            break;
        }
        case 17: {
            std::string name = readNonEmptyString("请输入商品名称: ");
            auto list = apiSearchByName(*warehouse, name, true); // 模糊
            displayGoodsList(list, "名称包含 \"" + name + "\" 的商品");
            break;
        }
        case 18: {
            std::string manufacturer = readNonEmptyString("请输入生产商: ");
            auto list = apiSearchByManufacturer(*warehouse, manufacturer, false); // 精确
            displayGoodsList(list, "生产商为 \"" + manufacturer + "\" 的商品");
            break;
        }
        case 19: {
            std::string id = readNonEmptyString("请输入商品编号: ");
            int quantity = readPositiveInt("请输入购买数量: ");
            int newStock = 0;
            if (apiSellGoods(*warehouse, id, quantity, newStock, errorMsg))
                std::cout << "[信息]: 销售成功！剩余库存: " << newStock << "\n";
            else
                std::cout << "[错误]: " << errorMsg << "\n";
            break;
        }
        case 20: {
            int days = readPositiveInt("请输入天数（最近多少天到货）: ");
            auto list = apiBrowseByArrivalDateRecent(*warehouse, days, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "最近 " + std::to_string(days) + " 天到货的商品");
            break;
        }
        case 21: {
            int days = readPositiveInt("请输入天数（多少天内过期）: ");
            auto list = apiBrowseByExpiryDateSoon(*warehouse, days, errorMsg);
            if (!errorMsg.empty())
                std::cout << "[错误]: " << errorMsg << "\n";
            else
                displayGoodsList(list, "即将在 " + std::to_string(days) + " 天内过期的商品");
            break;
        }
        case 22: {
            // 切换仓库
            std::vector<std::string> saveErrors;
            if (!apiSaveWarehouse(*warehouse, saveErrors) || !saveErrors.empty()) {
                std::cout << "[错误]: 保存当前仓库失败，无法切换。\n";
                for (const auto& e : saveErrors) std::cout << "  - " << e << "\n";
                break;
            }
            auto newFiles = apiScanWarehouseFiles(WAREHOUSE_DIR);
            if (newFiles.empty()) {
                std::cout << "[错误]: 仓库目录下没有可用的仓库文件。\n";
                break;
            }
            std::cout << "\n请选择要打开的仓库：\n";
            for (size_t i = 0; i < newFiles.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << newFiles[i] << "\n";
            }
            int newChoice;
            while (true) {
                newChoice = readInt("请输入编号: ");
                if (newChoice >= 1 && newChoice <= static_cast<int>(newFiles.size()))
                    break;
                std::cout << "无效编号，请重新输入。\n";
            }
            std::string newFile = newFiles[newChoice - 1];
            std::string newPath = WAREHOUSE_DIR + "/" + newFile;
            if (!fs::exists(newPath)) {
                std::cout << "[错误]: 文件不存在。\n";
                break;
            }
            warehouse = std::make_unique<Warehouse>(newPath);
            errors.clear();
            loadOk = apiLoadWarehouse(*warehouse, newPath, errors);
            if (!loadOk) {
                std::cout << "[错误]: 新仓库加载失败，程序将退出。\n";
                for (const auto& e : errors) std::cout << "  - " << e << "\n";
                return 1;
            }
            if (!errors.empty()) {
                std::cout << "[警告]: 新仓库加载成功，但存在 " << errors.size() << " 个问题：\n";
                for (const auto& e : errors) std::cout << "  - " << e << "\n";
            }
            else {
                std::cout << "[信息]: 成功切换到仓库: " << newFile << "\n";
            }
            break;
        }
        case 23: {
            std::string name = readNonEmptyString("请输入新仓库名称（不含扩展名）: ");
            std::string filename = name + ".txt";
            std::string fullpath = WAREHOUSE_DIR + "/" + filename;
            if (fs::exists(fullpath)) {
                std::cout << "[错误]: 仓库文件已存在。\n";
                break;
            }
            if (!apiCreateWarehouseFile(fullpath)) {
                std::cout << "[错误]: 创建仓库失败。\n";
                break;
            }
            std::cout << "[信息]: 新仓库创建成功。\n";
            std::cout << "是否立即切换到新仓库？(y/n): ";
            std::string ans;
            std::getline(std::cin, ans);
            if (ans == "y" || ans == "Y") {
                std::vector<std::string> saveErrors;
                if (!apiSaveWarehouse(*warehouse, saveErrors) || !saveErrors.empty()) {
                    std::cout << "[错误]: 保存当前仓库失败，无法切换。\n";
                    for (const auto& e : saveErrors) std::cout << "  - " << e << "\n";
                }
                else {
                    warehouse = std::make_unique<Warehouse>(fullpath);
                    errors.clear();
                    loadOk = apiLoadWarehouse(*warehouse, fullpath, errors);
                    if (!loadOk) {
                        std::cout << "[错误]: 新仓库加载失败。\n";
                        for (const auto& e : errors) std::cout << "  - " << e << "\n";
                    }
                    else {
                        std::cout << "[信息]: 已切换到新仓库。\n";
                    }
                }
            }
            break;
        }
        case 24: {
            auto stats = apiGetOverallStatistics(*warehouse);
            printOverallStatistics(stats);
            break;
        }
        case 25: {
            auto stats = apiGetCategoryStatistics(*warehouse);
            printCategoryStatistics(stats);
            break;
        }
        case 26: {
            double step = readDouble("请输入价格步长（默认 100）: ");
            if (step <= 0) step = 100.0;
            auto hist = apiGetPriceHistogram(*warehouse, step);
            printPriceHistogram(hist);
            break;
        }
        case 27: {
            int low = readInt("请输入低库存阈值（默认 10）: ");
            int high = readInt("请输入高库存阈值（默认 50）: ");
            if (low < 0) low = 10;
            if (high < low) high = low + 1;
            auto dist = apiGetStockDistribution(*warehouse, low, high);
            printStockDistribution(dist);
            break;
        }
        case 28: {
            auto stats = apiGetManufacturerStatistics(*warehouse);
            printManufacturerStatistics(stats);
            break;
        }
        case 0: {
            std::cout << "正在保存数据...\n";
            std::vector<std::string> saveErrors;
            bool saveOk = apiSaveWarehouse(*warehouse, saveErrors);
            if (!saveOk || !saveErrors.empty()) {
                std::cout << "[错误]: 保存时出现问题：\n";
                for (const auto& e : saveErrors) std::cout << "  - " << e << "\n";
                std::cout << "请检查文件权限。\n";
            }
            else {
                std::cout << "[信息]: 数据已保存。\n";
            }
            std::cout << "感谢使用，再见！\n";
            return 0;
        }
        default:
            std::cout << "无效选项，请重新选择。\n";
        }
        pressAnyKeyToContinue();
    }
}