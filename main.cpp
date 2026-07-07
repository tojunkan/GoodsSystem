#include <iostream>
#include "Warehouse.h"

using namespace std;

int main() {
    Warehouse myWarehouse; // 创建仓库对象，自动加载数据
    int choice;

    while (true) {
        cout << "\n========== 商品销售管理系统 ==========\n";
        cout << "1. 商品上架 (添加)\n";
        cout << "2. 商品浏览 (列表)\n";
        cout << "3. 商品查询 (搜索)\n";
        cout << "4. 商品销售 (购买)\n";
        cout << "0. 退出系统\n";
        cout << "====================================\n";
        cout << "请选择操作: ";

        if (!(cin >> choice)) {
            // 防止输入非数字导致死循环
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
        case 1: myWarehouse.addGoods(); break;
        case 2: myWarehouse.browseGoods(); break;
        case 3: myWarehouse.searchGoods(); break;
        case 4: myWarehouse.sellGoods(); break;
        case 0:
            cout << "感谢使用，再见！\n";
            return 0;
        default:
            cout << "无效输入，请重试。\n";
        }
    }
    return 0;
}