#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include "Goods.h"

class Warehouse {

public:
    // 浏览价格区间的默认值
    static constexpr double DEFAULT_MIN_PRICE = 0.0;
    static constexpr double DEFAULT_MAX_PRICE = 0x7ff0000000000000;

    // 浏览库存区间的默认值
    static constexpr int DEFAULT_MIN_STOCK = 0;
	static constexpr int DEFAULT_MAX_STOCK = 0x7fffffff;
    struct Category {
        std::string name; // 分类名称
        std::vector<std::pair<Goods, bool>> goodsList; // 该分类下的商品列表，bool如果是false意味着文件读取不完整
		int maxSeq; // 当前最大序号，用于生成新商品编号
        Category(const std::string& name) : name(name), maxSeq(0) {}
		Category(const std::string& name, int maxSeq, int totalnumber) : name(name), maxSeq(maxSeq) {
			goodsList.reserve(totalnumber);
        }
    };

    struct GoodsWithCategory {
        std::string CategoryName;
        Goods goods;

        void display(std::ostream& os = std::cout) const;
    };

    explicit Warehouse(const std::string& fname = "warehouse.txt");
	~Warehouse() = default;

    bool getDirtiness();

    // 1. 从文件加载数据
    bool loadData(std::vector<std::string>& errorLines);

    // 2. 保存数据到文件
    bool saveData(std::vector<std::string>& errorLines);

	//这里检测的是内存中标记为true的数据是否符合要求，返回不符合要求的商品编号列表
    std::vector<std::string> validateData() const;

	//如果已存在该分类，返回 false；否则创建新分类并返回 true
	bool createCategory(const std::string& categoryName);

	bool deleteCategory(const std::string& categoryName); // 删除分类，必须保证该分类下没有商品，否则拒绝删除。

	bool renameCategory(const std::string& oldName, const std::string& newName); // 重命名分类，必须保证新名称不重复。

	const std::vector<Category>& getCategories() const; // 获取所有分类信息

	const Category* getCategoryByName(const std::string& categoryName) const; // 根据分类名称获取分类指针，如果不存在返回 nullptr

	std::vector<const Category*> getCategoryByNameFuzzy(const std::string& categoryName) const; // 根据分类名称模糊搜索，返回所有匹配的分类指针列表

    // 3. 添加商品 (上架)
    std::string addGoods(const std::string& categoryName,
                         const std::string& name,
                         double price,
                         const std::string& manufacturer,
                         int stock,
                		 const std::string& arrivalDate = "",
                         const std::string& expiryDate = Goods::DEFAULT_EXPIRY_DATE,
                         const std::string& picture = "");

	bool removeGoods(const std::string& id); // 删除商品 (下架)

    bool updateGoods(const std::string& id,
		             const Goods& updatedGoods, std::string* err); // 更新商品信息 (除编号外)，如果编号不一致拒绝修改。

	std::string moveGoodsToCategory(const std::string& id, const std::string& newCategoryName); // 移动商品到新分类，如果新分类不存在则拒绝移动，返回空字符串
    // 4. 浏览商品
    std::vector<GoodsWithCategory> browseByCategory(const std::string& categoryName, std::string* err) const;

	std::vector<GoodsWithCategory> browseAll() const;

	std::vector<Goods> browseInvalid() const; // 浏览所有信息不完整的商品

	std::vector<GoodsWithCategory> browseByPriceRange(double minPrice = DEFAULT_MIN_PRICE, double maxPrice = DEFAULT_MAX_PRICE, std::string* err = nullptr) const;

	std::vector<GoodsWithCategory> browseByStockRange(int minStock = DEFAULT_MIN_STOCK, int maxStock = DEFAULT_MAX_STOCK, std::string* err = nullptr) const;

	std::vector<GoodsWithCategory> browseByArrivalDateRange(const std::string& startDate, const std::string& endDate = Goods::CURRENT_DATE, std::string* err = nullptr) const;

	std::vector<GoodsWithCategory> browseByExpiryDateRange(const std::string& startDate, const std::string& endDate, std::string* err = nullptr) const;

	std::vector<GoodsWithCategory> browseByArrivalDateRecent(int days, std::string* err = nullptr) const; // 浏览最近到货的商品，days为天数

	std::vector<GoodsWithCategory> browseByExpiryDateSoon(int days, std::string* err = nullptr) const; // 浏览即将过期的商品，days为天数

    // 5. 查询商品 (按编号)
    std::optional<GoodsWithCategory>  searchGoodsById(const std::string& id, std::string* err) const;
    //5. 查询商品 (按名称)
    std::vector<GoodsWithCategory> searchGoodsByName(const std::string& name) const;

    std::vector<GoodsWithCategory> searchGoodsByNameFuzzy(const std::string& name) const;

	std::vector<GoodsWithCategory> searchGoodsByManufacturer(const std::string& manufacturer) const;

    std::vector<GoodsWithCategory> searchGoodsByManufacturerFuzzy(const std::string& manufacturer) const;
	// 6. 销售商品，返回最新库存量，如果商品不存在或库存不足，返回错误码
    bool sellGoods(const std::string& id, 
                         int quantity, 
                         int& newStock, std::string* err);

private:
    std::vector<Category> categories;// 所有分区
    std::string filename;         // 数据文件名
	std::unordered_map<std::string, int> categoryPrefixMap; // 分类名称 -> 前缀编号
	mutable bool isDirty;          // 数据是否已修改，若为 true 则需要保存

	bool Validation(std::string id); // 检查商品编号是否唯一且正确

	Goods* findAuxiliary(const std::string& id, int& categoryIndex, int& goodsIndex, bool invalid_filter = true); // 查找商品，返回指针，如果不存在返回 nullptr
};
