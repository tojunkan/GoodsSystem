#pragma once
#include<vector>
#include"Warehouse.h"
// 整体统计
struct OverallStatistics {
    int totalGoods;      // 有效商品总数
    int totalCategories;
    int totalStock;
    double totalValue;
    double averagePrice;
};

// 分类统计
struct CategoryStatistics {
    std::string categoryName;
    int goodsCount;
    int stockSum;
    double valueSum;
};

// 价格直方图
struct PriceInterval {
    double lowerBound;
    double upperBound;  // 若为 infinity 表示“以上”
    int count;
};

// 库存分布
struct StockDistribution {
    int lowCount;    // stock <= lowThreshold
    int mediumCount; // lowThreshold < stock <= highThreshold
    int highCount;   // stock > highThreshold
};

// 生产商统计
struct ManufacturerStatistics {
    std::string manufacturer;
    int goodsCount;
    double totalValue;
};

class Warehouse;

class Statistics{
public:
    explicit Statistics(const Warehouse& wh) :warehouse(wh) {}

    OverallStatistics getOverallStatistics() const;
    std::vector<CategoryStatistics> getCategoryStatistics() const;
    std::vector<PriceInterval> getPriceHistogram(double step = 100.0) const;
    StockDistribution getStockDistribution(int lowThreshold = 10, int highThreshold = 50) const;
    std::vector<int> getAllStockValues() const; // 供前端自定义分布
    std::vector<ManufacturerStatistics> getManufacturerStatistics() const;
private:
    const Warehouse& warehouse;
};