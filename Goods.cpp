#include "Goods.h"

void Goods::setId(string i) { id = i; }
string Goods::getId() { return id; }

void Goods::setName(string n) { name = n; }
string Goods::getName() { return name; }

void Goods::setPrice(double p) { price = p; }
double Goods::getPrice() { return price; }

void Goods::setFactory(string f) { factory = f; }
string Goods::getFactory() { return factory; }

void Goods::setStock(int s) { stock = s; }
int Goods::getStock() { return stock; }