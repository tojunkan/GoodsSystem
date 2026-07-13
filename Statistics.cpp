#include<vector>
#include<algorithm>
#include"Statistics.h"
#include"Warehouse.h"

// ========== 统计函数 ==========

OverallStatistics Statistics::getOverallStatistics() const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    OverallStatistics stats{};
    stats.totalCategories = static_cast<int>(categories.size());

    for (const auto& cat : categories) {
        for (const auto& pair : cat.goodsList) {
            if (!pair.second) continue; // 跳过无效商品
            const Goods& g = pair.first;
            stats.totalGoods++;
            stats.totalStock += g.getStock();
            stats.totalValue += g.getPrice() * g.getStock();
        }
    }
    stats.averagePrice = (stats.totalGoods > 0) ? (stats.totalValue / stats.totalGoods) : 0.0;
    return stats;
}

std::vector<CategoryStatistics> Statistics::getCategoryStatistics() const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    std::vector<CategoryStatistics> result;
    result.reserve(categories.size());

    for (const auto& cat : categories) {
        CategoryStatistics cs{ cat.name, 0, 0, 0.0 };
        for (const auto& pair : cat.goodsList) {
            if (!pair.second) continue;
            const Goods& g = pair.first;
            cs.goodsCount++;
            cs.stockSum += g.getStock();
            cs.valueSum += g.getPrice() * g.getStock();
        }
        result.push_back(cs);
    }
    return result;
}

std::vector<PriceInterval> Statistics::getPriceHistogram(double step) const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    if (step <= 0) step = 100.0; // 防御

    // 先收集所有有效商品的价格
    std::vector<double> prices;
    for (const auto& cat : categories) {
        for (const auto& pair : cat.goodsList) {
            if (pair.second) {
                prices.push_back(pair.first.getPrice());
            }
        }
    }
    if (prices.empty()) return {};

    // 找最大价格，确定区间数量
    double maxPrice = *std::max_element(prices.begin(), prices.end());
    int intervals = static_cast<int>(std::ceil(maxPrice / step)) + 1; // 多一个无穷区间

    std::vector<PriceInterval> result;
    result.reserve(intervals);

    for (int i = 0; i < intervals; ++i) {
        double lower = i * step;
        double upper = (i == intervals - 1) ? 0x7ff0000000000000 : (i + 1) * step; // -1 表示无穷
        int count = 0;
        for (double p : prices) {
            if (p >= lower && (upper == 0x7ff0000000000000 || p < upper)) {
                count++;
            }
        }
        result.push_back({ lower, upper, count });
    }
    return result;
}

StockDistribution Statistics::getStockDistribution(int lowThreshold, int highThreshold) const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    StockDistribution dist{ 0, 0, 0 };
    for (const auto& cat : categories) {
        for (const auto& pair : cat.goodsList) {
            if (!pair.second) continue;
            int stock = pair.first.getStock();
            if (stock <= lowThreshold) {
                dist.lowCount++;
            }
            else if (stock <= highThreshold) {
                dist.mediumCount++;
            }
            else {
                dist.highCount++;
            }
        }
    }
    return dist;
}

std::vector<int> Statistics::getAllStockValues() const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    std::vector<int> values;
    for (const auto& cat : categories) {
        for (const auto& pair : cat.goodsList) {
            if (pair.second) {
                values.push_back(pair.first.getStock());
            }
        }
    }
    return values;
}

std::vector<ManufacturerStatistics> Statistics::getManufacturerStatistics() const {
    const std::vector<Warehouse::Category>& categories = warehouse.getCategories();
    std::unordered_map<std::string, ManufacturerStatistics> map;
    for (const auto& cat : categories) {
        for (const auto& pair : cat.goodsList) {
            if (!pair.second) continue;
            const Goods& g = pair.first;
            const std::string& mfr = g.getManufacturer();
            auto it = map.find(mfr);
            if (it == map.end()) {
                map.emplace(mfr, ManufacturerStatistics{ mfr, 1, g.getPrice() * g.getStock() });
            }
            else {
                it->second.goodsCount++;
                it->second.totalValue += g.getPrice() * g.getStock();
            }
        }
    }
    std::vector<ManufacturerStatistics> result;
    result.reserve(map.size());
    for (auto& kv : map) {
        result.push_back(std::move(kv.second));
    }
    // 按商品数降序排序（可选）
    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) { return a.goodsCount > b.goodsCount; });
    return result;
}