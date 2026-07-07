#include "Goods.h"
#include <iomanip> // 用于格式化输出
#include <ctime>
//
//// 默认构造
//Goods::Goods() : price(0), stock(0) {}

std::string Goods::CURRENT_DATE = Goods::getCurrentDate();
const std::string Goods::DEFAULT_EXPIRY_DATE = "9999-12-31";//对于无保质期的商品应该填入这个值，不要置空，以示和文件读取错误的区分。
// 参数构造
Goods::Goods(const std::string& id, const std::string& name, double price, const std::string& manufacturer, int stock, const std::string& arrivalDate, const std::string& expiryDate, const std::string& picture)
: id(id), name(name), price(price), manufacturer(manufacturer), stock(stock), arrivalDate(arrivalDate), expiryDate(expiryDate), picture(picture) {
}

std::string Goods::getId() const {
    return id;
}
std::string Goods::getName() const {
    return name;
}
double Goods::getPrice() const {
    return price;
}
std::string Goods::getManufacturer() const {
    return manufacturer;
}
int Goods::getStock() const {
    return stock;
}
std::string Goods::getArrivalDate() const {
    return arrivalDate;
}
std::string Goods::getExpiryDate() const {
    return expiryDate;
}
std::string Goods::getPicture() const {
    return picture;
}

void Goods::setName(const std::string& name) {
    this->name = name;
}

void Goods::setPrice(double price) {
    this->price = price;
}

void Goods::setManufacturer(const std::string& manufacturer) {
    this->manufacturer = manufacturer;
}

void Goods::setStock(int stock) {
    this->stock = stock;
}

void Goods::setArrivalDate(const std::string& arrivalDate) {
    this->arrivalDate = arrivalDate;
}

void Goods::setExpiryDate(const std::string& expiryDate) {
    this->expiryDate = expiryDate;
}

void Goods::setPicture(const std::string& picture) {
    this->picture = picture;
}

// 显示信息
void Goods::display(std::ostream& os) const {
    os << "--------------------------------------------------\n";
    os << "编号: " << id << "\n";
    os << "名称: " << name << "\n";
    os << "单价: " << price << " 元\n";
    os << "厂商: " << manufacturer << "\n";
    os << "库存: " << stock << "\n";
	os << "到货日期: " << arrivalDate << "\n";
	if(expiryDate != DEFAULT_EXPIRY_DATE) os << "保质期: " << expiryDate << "\n";
	else os << "保质期: 长期\n";
    os << "--------------------------------------------------\n";
}

char Goods::generateCheckDigit(const std::string& raw6)
{
    if (raw6.length() != 6) {
		return '\0'; // 返回空字符表示无效
    }
	int weights[6] = { 7, 9, 10, 5, 8, 4 };
    int sum = 0;
    for (int i = 0;i < 6;i++) {
        if (!std::isdigit(raw6[i])) return '\0'; // 返回空字符表示无效
		sum += (raw6[i] - '0') * weights[i];
    }
    std::string table = "10X98765432";
	return table[sum % 11];
}

std::string Goods::generateFullId(const std::string& raw6) {
    char checkDigit = Goods::generateCheckDigit(raw6);
    return raw6 + checkDigit;
}

std::string Goods::getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm localtime;
#ifdef _WIN32 || _WIN64
	localtime_s(&localtime, &now);
 #else
	localtime_r(&now, &localtime);
#endif
    char buf[11];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d", &localtime);
	return std::string(buf);
}

namespace {
    // 格式化回 "YYYY-MM-DD"
    std::string format(int y, unsigned m, unsigned d) {
        char buf[11]; // "YYYY-MM-DD" 正好 10 字符 + '\0'
        snprintf(buf, sizeof(buf), "%04d-%02u-%02u", y, m, d);
        return std::string(buf);
    }

    // 天数转年月日（Hinnant 算法）
    std::string civil_from_days(int64_t z) {
        z += 719468;
        const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        const int y = static_cast<int>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const unsigned mp = (5 * doy + 2) / 153;
        const unsigned d = doy - (153 * mp + 2) / 5 + 1;
        const unsigned m = mp + (mp < 10 ? 3 : -9);
        return format( y + (m <= 2), static_cast<int>(m), static_cast<int>(d) );
    }

    // 年月日转天数（Hinnant 算法）
    int64_t days_from_civil(int y, unsigned m, unsigned d) {
        y -= m <= 2;
        const int64_t era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<int64_t>(doe) - 719468;
    }

}

std::string Goods::DaysOffset(const std::string& date, int offset) {
    if (!isValidDate(date)) {
		return ""; // 无效日期格式，返回空字符串
    }
    
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

	int64_t days = days_from_civil(year, month, day) + offset;
    
	return civil_from_days(days);
}

bool Goods::isValidId(const std::string& id) {
    if (id.length() != 7) return false;
    std::string raw6 = id.substr(0, 6);
    char expectedCheckDigit = Goods::generateCheckDigit(raw6);
    return id[6] == expectedCheckDigit;
}

bool Goods::isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(date[i])) return false;
    }
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
	int daysInMonth[] = {0, 31, (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (year < 1900 || (year > 2100 && year != 9999)) return false;
	if (month < 1 || month > 12) return false;
	if (day < 1 || day > daysInMonth[month]) return false;
    return true;
}

std::string Goods::isValidFields(const std::string& id, const std::string& name, double price, const std::string& manufacturer, int stock, const std::string& arrivalDate, const std::string& expiryDate) {
    if (name.empty()) return "商品名称不能为空";
    if (price < 0) return "单价不能为负数";
    if (manufacturer.empty()) return "生产商不能为空";
    if (stock < 0) return "库存量不能为负数";
	if (!isValidId(id)) return "商品编号不合法";
	if (!isValidDate(arrivalDate)) return "到货日期不合法";
    if (arrivalDate > CURRENT_DATE) return "到货日期不能晚于今天";
	if (!isValidDate(expiryDate)) return "保质期不合法";
    return ""; // 合法
}