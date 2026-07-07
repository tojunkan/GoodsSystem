#include "Goods.h"
#include <iomanip> // 用于格式化输出

// 默认构造
Goods::Goods() : price(0), stock(0) {}

// 参数构造
Goods::Goods(std::string id, std::string name, double price, std::string manufacturer, int stock)
    : id(id), name(name), price(price), manufacturer(manufacturer), stock(stock) {
}

// 显示信息
void Goods::display() const {
    std::cout << "--------------------------------------------------\n";
    std::cout << "编号: " << id << "\n";
    std::cout << "名称: " << name << "\n";
    std::cout << "单价: " << price << " 元\n";
    std::cout << "厂商: " << manufacturer << "\n";
    std::cout << "库存: " << stock << "\n";
    std::cout << "--------------------------------------------------\n";
}