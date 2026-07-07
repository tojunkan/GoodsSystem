#ifndef GOODS_H
#define GOODS_H
#include <string>
using namespace std;

class Goods
{
private:
    string id;      // 商品编号
    string name;    // 商品名称
    double price;   // 单价
    string factory; // 生产商
    int stock;      // 库存
public:
    void setId(string i);
    string getId();

    void setName(string n);
    string getName();

    void setPrice(double p);
    double getPrice();

    void setFactory(string f);
    string getFactory();

    void setStock(int s);
    int getStock();
};
#endif