#pragma once
#include <string>
#include <iostream>

class Goods {
private:
    std::string id;          // 商品编号
    std::string name;        // 商品名称
    double price;            // 单价
    std::string manufacturer;// 生产商
    int stock;               // 库存量
    std::string arrivalDate; // 到货日期（格式: YYYY-MM-DD）
	std::string expiryDate;  // 保质期（格式: YYYY-MM-DD）
	std::string picture;     // 图片路径（仅保存路径字符串）
	
    static std::string CURRENT_DATE; // 当前日期（格式: YYYY-MM-DD），用于判断商品是否过期

    static std::string generateFullId(const std::string& raw6); // 生成完整的商品编号（含校验位）
	static char generateCheckDigit(const std::string& raw6); // 生成校验位的私有方法
public:

	static const std::string DEFAULT_EXPIRY_DATE; // 永不过期的特殊值
    // 默认构造函数
    Goods() = default;

    // 带参数的构造函数，方便初始化
    Goods(const std::string& id, 
          const std::string& name, 
          double price, 
          const std::string& manufacturer, 
          int stock,
		  const std::string& arrivalDate,
		  const std::string& expiryDate = "9999-12-31", //用这个特殊值表示 永不过期
          const std::string& picture = "");

    // ---------- Getter（只读属性） ----------
    std::string getId() const;
    std::string getName() const;
    double getPrice() const;
    std::string getManufacturer() const;
    int getStock() const;
	std::string getArrivalDate() const;
	std::string getExpiryDate() const;
    std::string getPicture() const;   // 图片路径后门

    // ---------- Setter（仅允许修改业务可变字段，严禁修改 ID） ----------
    void setName(const std::string& name);
    void setPrice(double price);
    void setManufacturer(const std::string& manufacturer);
    void setStock(int stock);
	void setArrivalDate(const std::string& arrivalDate);
	void setExpiryDate(const std::string& expiryDate);
    void setPicture(const std::string& picture); // 仅保存路径字符串


    static std::string getCurrentDate();

	static std::string DaysOffset(const std::string& date, int offset); // 计算日期偏移后的新日期

	static bool isValidId(const std::string& id); // 检查商品编号是否合法

	static bool isValidDate(const std::string& date); // 检查日期格式是否合法（YYYY-MM-DD）

    static bool isValidStock(const int n);

    static bool isValidPrice(const double p);

	static std::string isValidFields(const std::string& id, const std::string& name, double price, const std::string& manufacturer, int stock, const std::string& arrivalDate, const std::string& expiryDate); // 检查商品信息是否合法

    // 显示商品信息
    void display(std::ostream& os = std::cout) const;

	friend class Warehouse; // 允许 Warehouse 类访问私有成员
};
