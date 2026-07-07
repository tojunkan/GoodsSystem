#pragma once
#include <string>
#include <iostream>

class Goods {
public:
    std::string id;          // 商品编号
    std::string name;        // 商品名称
    double price;            // 单价
    std::string manufacturer;// 生产商
    int stock;               // 库存量

    // 默认构造函数
    Goods();

    // 带参数的构造函数，方便初始化
    Goods(std::string id, std::string name, double price, std::string manufacturer, int stock);

    // 显示商品信息
    void display() const;
};
