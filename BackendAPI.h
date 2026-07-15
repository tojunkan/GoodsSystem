#pragma once
#include "Warehouse.h"
#include "Statistics.h"
#include <vector>
#include <string>
#include <optional>

// ========== 仓库文件管理 ==========
std::vector<std::string> apiScanWarehouseFiles(const std::string& dir);
bool apiCreateWarehouseFile(const std::string& path);
bool apiDeleteWarehouseFile(const std::string& path, std::string& error);
bool apiRenameWarehouseFile(const std::string& oldpath, const std::string& newpath, std::string& error);
bool apiLoadWarehouse(Warehouse& wh, const std::string& path, std::vector<std::string>& errors);
bool apiSaveWarehouse(Warehouse& wh, std::vector<std::string>& errors);

// ========== 分类管理 ==========
bool apiCreateCategory(Warehouse& wh, const std::string& name);
bool apiDeleteCategory(Warehouse& wh, const std::string& name);
bool apiRenameCategory(Warehouse& wh, const std::string& oldName, const std::string& newName);
std::vector<std::string> apiListCategories(const Warehouse& wh);

// ========== 商品管理 ==========
std::string apiAddGoods(Warehouse& wh,
    const std::string& category,
    const std::string& name,
    double price,
    const std::string& manufacturer,
    int stock,
    const std::string& arrivalDate,
    const std::string& expiryDate,
    const std::string& picture);
bool apiRemoveGoods(Warehouse& wh, const std::string& id);
bool apiUpdateGoods(Warehouse& wh, const std::string& id,
    const Goods& updated, std::string& errMsg);
std::string apiMoveGoods(Warehouse& wh, const std::string& id,
    const std::string& newCategory);

// ========== 浏览 ==========
std::vector<Warehouse::GoodsWithCategory> apiBrowseAll(const Warehouse& wh);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByCategory(const Warehouse& wh,
    const std::string& category,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByPriceRange(const Warehouse& wh,
    double minPrice,
    double maxPrice,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByStockRange(const Warehouse& wh,
    int minStock,
    int maxStock,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByArrivalDateRange(const Warehouse& wh,
    const std::string& startDate,
    const std::string& endDate,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByExpiryDateRange(const Warehouse& wh,
    const std::string& startDate,
    const std::string& endDate,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByArrivalDateRecent(const Warehouse& wh,
    int days,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiBrowseByExpiryDateSoon(const Warehouse& wh,
    int days,
    std::string& errMsg);
std::vector<Goods> apiBrowseInvalid(const Warehouse& wh);

// ========== 搜索 ==========
std::optional<Warehouse::GoodsWithCategory> apiSearchById(const Warehouse& wh,
    const std::string& id,
    std::string& errMsg);
std::vector<Warehouse::GoodsWithCategory> apiSearchByName(const Warehouse& wh,
    const std::string& name,
    bool fuzzy = false);
std::vector<Warehouse::GoodsWithCategory> apiSearchByManufacturer(const Warehouse& wh,
    const std::string& manufacturer,
    bool fuzzy = false);

// ========== 销售 ==========
bool apiSellGoods(Warehouse& wh, const std::string& id, int quantity,
    int& newStock, std::string& errMsg);

// ========== 统计 ==========
OverallStatistics apiGetOverallStatistics(const Warehouse& wh);
std::vector<CategoryStatistics> apiGetCategoryStatistics(const Warehouse& wh);
std::vector<PriceInterval> apiGetPriceHistogram(const Warehouse& wh, double step = 100.0);
StockDistribution apiGetStockDistribution(const Warehouse& wh, int lowThreshold = 10, int highThreshold = 50);
std::vector<int> apiGetAllStockValues(const Warehouse& wh);
std::vector<ManufacturerStatistics> apiGetManufacturerStatistics(const Warehouse& wh);