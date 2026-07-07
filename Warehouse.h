#pragma once
#include <vector>
#include <string>
#include "Goods.h"

class Warehouse {
private:
    std::vector<Goods> goodsList; // 存放所有商品的容器
    std::string filename;         // 数据文件名

public:
    Warehouse(std::string fname = "data.txt");

    // 1. 从文件加载数据
    void loadData();
    // 2. 保存数据到文件
    void saveData();

    // 3. 添加商品 (上架)
    void addGoods();
    // 4. 浏览所有商品
    void browseGoods();
    // 5. 查询商品 (按编号)
    void searchGoods();
    // 6. 销售商品
    void sellGoods();
};
