#include "BackendAPI.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

// ========== 仓库文件管理 ==========
std::vector<std::string> apiScanWarehouseFiles(const std::string& dir) {
    std::vector<std::string> files;
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
        return files;
    }
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

bool apiCreateWarehouseFile(const std::string& path) {
    if (fs::exists(path)) return false;
    Warehouse newWarehouse(path);
    std::vector<std::string> errors;
    return newWarehouse.saveData(errors) && errors.empty();
}

bool apiLoadWarehouse(Warehouse& wh, const std::string& path, std::vector<std::string>& errors) {
    wh = Warehouse(path);
    return wh.loadData(errors);
}

bool apiSaveWarehouse(Warehouse& wh, std::vector<std::string>& errors) {
    return wh.saveData(errors);
}

// ========== 分类管理 ==========
bool apiCreateCategory(Warehouse& wh, const std::string& name) {
    return wh.createCategory(name);
}

bool apiDeleteCategory(Warehouse& wh, const std::string& name) {
    return wh.deleteCategory(name);
}

bool apiRenameCategory(Warehouse& wh, const std::string& oldName, const std::string& newName) {
    return wh.renameCategory(oldName, newName);
}

std::vector<std::string> apiListCategories(const Warehouse& wh) {
    std::vector<std::string> names;
    for (const auto& cat : wh.getCategories()) {
        names.push_back(cat.name);
    }
    return names;
}

// ========== 商品管理 ==========
std::string apiAddGoods(Warehouse& wh,
    const std::string& category,
    const std::string& name,
    double price,
    const std::string& manufacturer,
    int stock,
    const std::string& arrivalDate,
    const std::string& expiryDate,
    const std::string& picture) {
    return wh.addGoods(category, name, price, manufacturer, stock, arrivalDate, expiryDate, picture);
}

bool apiRemoveGoods(Warehouse& wh, const std::string& id) {
    return wh.removeGoods(id);
}

bool apiUpdateGoods(Warehouse& wh, const std::string& id,
    const Goods& updated, std::string& errMsg) {
    return wh.updateGoods(id, updated, &errMsg);
}

std::string apiMoveGoods(Warehouse& wh, const std::string& id,
    const std::string& newCategory) {
    return wh.moveGoodsToCategory(id, newCategory);
}

// ========== 浏览（通过 findGoodsIf 实现） ==========
std::vector<Warehouse::GoodsWithCategory> apiBrowseAll(const Warehouse& wh) {
    return wh.browseAll();
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByCategory(const Warehouse& wh,
    const std::string& category,
    std::string& errMsg) {
    std::vector<Warehouse::GoodsWithCategory> result;
    auto catPtr = wh.getCategoryByName(category);
    if (!catPtr) {
        errMsg = "分类不存在";
        return result;
    }
    // 利用 findGoodsIf
    return wh.findGoodsIf([&category](const Goods& g, const std::string& catName) {
        return catName == category;
        });
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByPriceRange(const Warehouse& wh,
    double minPrice,
    double maxPrice,
    std::string& errMsg) {
    if (!Goods::isValidPrice(minPrice) || !Goods::isValidPrice(maxPrice) || minPrice > maxPrice) {
        errMsg = "价格区间不合法";
        return {};
    }
    return wh.findGoodsIf([minPrice, maxPrice](const Goods& g, const std::string&) {
        return g.getPrice() >= minPrice && g.getPrice() <= maxPrice;
        });
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByStockRange(const Warehouse& wh,
    int minStock,
    int maxStock,
    std::string& errMsg) {
    if (!Goods::isValidStock(minStock) || !Goods::isValidStock(maxStock) || minStock > maxStock) {
        errMsg = "库存区间不合法";
        return {};
    }
    return wh.findGoodsIf([minStock, maxStock](const Goods& g, const std::string&) {
        return g.getStock() >= minStock && g.getStock() <= maxStock;
        });
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByArrivalDateRange(const Warehouse& wh,
    const std::string& startDate,
    const std::string& endDate,
    std::string& errMsg) {
    if (!Goods::isValidDate(startDate) || !Goods::isValidDate(endDate) || startDate > endDate) {
        errMsg = "日期区间不合法";
        return {};
    }
    return wh.findGoodsIf([startDate, endDate](const Goods& g, const std::string&) {
        return g.getArrivalDate() >= startDate && g.getArrivalDate() <= endDate;
        });
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByExpiryDateRange(const Warehouse& wh,
    const std::string& startDate,
    const std::string& endDate,
    std::string& errMsg) {
    if (!Goods::isValidDate(startDate) || !Goods::isValidDate(endDate) || startDate > endDate) {
        errMsg = "日期区间不合法";
        return {};
    }
    return wh.findGoodsIf([startDate, endDate](const Goods& g, const std::string&) {
        return g.getExpiryDate() >= startDate && g.getExpiryDate() <= endDate;
        });
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByArrivalDateRecent(const Warehouse& wh,
    int days,
    std::string& errMsg) {
    if (days <= 0) {
        errMsg = "天数必须为正";
        return {};
    }
    std::string start = Goods::DaysOffset(Goods::CURRENT_DATE, -days);
    return apiBrowseByArrivalDateRange(wh, start, Goods::CURRENT_DATE, errMsg);
}

std::vector<Warehouse::GoodsWithCategory> apiBrowseByExpiryDateSoon(const Warehouse& wh,
    int days,
    std::string& errMsg) {
    if (days <= 0) {
        errMsg = "天数必须为正";
        return {};
    }
    std::string end = Goods::DaysOffset(Goods::CURRENT_DATE, days);
    return apiBrowseByExpiryDateRange(wh, Goods::CURRENT_DATE, end, errMsg);
}

std::vector<Goods> apiBrowseInvalid(const Warehouse& wh) {
    return wh.browseInvalid();
}

// ========== 搜索 ==========
std::optional<Warehouse::GoodsWithCategory> apiSearchById(const Warehouse& wh,
    const std::string& id,
    std::string& errMsg) {
    if (!Goods::isValidId(id)) {
        errMsg = "ID不合法";
        return std::nullopt;
    }
    auto result = wh.findGoodsIf([&id](const Goods& g, const std::string&) {
        return g.getId() == id;
        });
    if (result.empty()) {
        errMsg = "未找到该商品";
        return std::nullopt;
    }
    return result.front();
}

std::vector<Warehouse::GoodsWithCategory> apiSearchByName(const Warehouse& wh,
    const std::string& name,
    bool fuzzy) {
    if (fuzzy) {
        return wh.findGoodsIf([&name](const Goods& g, const std::string&) {
            return g.getName().find(name) != std::string::npos;
            });
    }
    else {
        return wh.findGoodsIf([&name](const Goods& g, const std::string&) {
            return g.getName() == name;
            });
    }
}

std::vector<Warehouse::GoodsWithCategory> apiSearchByManufacturer(const Warehouse& wh,
    const std::string& manufacturer,
    bool fuzzy) {
    if (fuzzy) {
        return wh.findGoodsIf([&manufacturer](const Goods& g, const std::string&) {
            return g.getManufacturer().find(manufacturer) != std::string::npos;
            });
    }
    else {
        return wh.findGoodsIf([&manufacturer](const Goods& g, const std::string&) {
            return g.getManufacturer() == manufacturer;
            });
    }
}

// ========== 销售 ==========
bool apiSellGoods(Warehouse& wh, const std::string& id, int quantity,
    int& newStock, std::string& errMsg) {
    return wh.sellGoods(id, quantity, newStock, &errMsg);
}

// ========== 统计 ==========
OverallStatistics apiGetOverallStatistics(const Warehouse& wh) {
    Statistics stats(wh);
    return stats.getOverallStatistics();
}

std::vector<CategoryStatistics> apiGetCategoryStatistics(const Warehouse& wh) {
    Statistics stats(wh);
    return stats.getCategoryStatistics();
}

std::vector<PriceInterval> apiGetPriceHistogram(const Warehouse& wh, double step) {
    Statistics stats(wh);
    return stats.getPriceHistogram(step);
}

StockDistribution apiGetStockDistribution(const Warehouse& wh, int lowThreshold, int highThreshold) {
    Statistics stats(wh);
    return stats.getStockDistribution(lowThreshold, highThreshold);
}

std::vector<int> apiGetAllStockValues(const Warehouse& wh) {
    Statistics stats(wh);
    return stats.getAllStockValues();
}

std::vector<ManufacturerStatistics> apiGetManufacturerStatistics(const Warehouse& wh) {
    Statistics stats(wh);
    return stats.getManufacturerStatistics();
}