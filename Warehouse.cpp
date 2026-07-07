#include "Warehouse.h"
#include <fstream>
#include <iostream>

using namespace std;

Warehouse::Warehouse(string fname) : filename(fname) {
    loadData(); // 程序启动时自动加载数据
}

// 从文件读取
void Warehouse::loadData() {
    ifstream inFile(filename);
    if (!inFile) return; // 文件不存在则跳过

    Goods g;
    // 假设文件格式为：编号 名称 单价 厂商 库存 (用空格或换行分隔)
    while (inFile >> g.id >> g.name >> g.price >> g.manufacturer >> g.stock) {
        goodsList.push_back(g);
    }
    inFile.close();
    cout << "[系统] 成功加载 " << goodsList.size() << " 条商品数据。\n";
}

// 保存到文件
void Warehouse::saveData() {
    ofstream outFile(filename);
    if (!outFile) {
        cout << "[错误] 无法保存文件！\n";
        return;
    }
    for (const auto& g : goodsList) {
        outFile << g.id << " " << g.name << " " << g.price << " " << g.manufacturer << " " << g.stock << "\n";
    }
    outFile.close();
    cout << "[系统] 数据已保存。\n";
}

// 添加商品
void Warehouse::addGoods() {
    Goods newGoods;
    cout << "请输入商品编号: "; cin >> newGoods.id;

    // 简单查重
    for (const auto& g : goodsList) {
        if (g.id == newGoods.id) {
            cout << "错误：该编号已存在！\n";
            return;
        }
    }

    cout << "请输入商品名称: "; cin >> newGoods.name;
    cout << "请输入单价: "; cin >> newGoods.price;
    cout << "请输入生产商: "; cin >> newGoods.manufacturer;
    cout << "请输入初始库存: "; cin >> newGoods.stock;

    goodsList.push_back(newGoods);
    saveData(); // 添加后自动保存
    cout << "上架成功！\n";
}

// 浏览商品
void Warehouse::browseGoods() {
    if (goodsList.empty()) {
        cout << "仓库为空。\n";
        return;
    }
    cout << "=== 商品列表 ===\n";
    for (const auto& g : goodsList) {
        g.display();
    }
}

// 查询商品
void Warehouse::searchGoods() {
    string targetId;
    cout << "请输入要查询的商品编号: ";
    cin >> targetId;

    for (const auto& g : goodsList) {
        if (g.id == targetId) {
            g.display();
            return;
        }
    }
    cout << "未找到该商品。\n";
}

// 销售商品
void Warehouse::sellGoods() {
    string targetId;
    int quantity;
    cout << "请输入要购买的商品编号: ";
    cin >> targetId;
    cout << "请输入购买数量: ";
    cin >> quantity;

    for (auto& g : goodsList) { // 注意这里要用引用 auto& 才能修改库存
        if (g.id == targetId) {
            if (g.stock >= quantity) {
                g.stock -= quantity;
                double total = g.price * quantity;
                cout << "购买成功！应付金额: " << total << " 元\n";
                saveData(); // 销售后库存变动，需保存
            }
            else {
                cout << "库存不足！当前库存: " << g.stock << "\n";
            }
            return;
        }
    }
    cout << "未找到该商品。\n";
}