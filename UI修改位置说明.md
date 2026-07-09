# UI 修改位置说明

这个项目目前没有使用 Qt Designer 的 `.ui` 文件，所有界面都是用 C++ 代码创建的。这样做的好处是代码少依赖、复制后容易直接运行；缺点是不能直接双击 `.ui` 拖拽修改。

你要改界面时，看下面这些文件：

## 主菜单
- 文件：`mainmenu.cpp`
- 位置：`MainMenu::MainMenu(...)`

## 登录界面
- 文件：`logindialog.cpp`
- 位置：`LoginDialog::LoginDialog(...)`

## 销售员注册界面
- 文件：`registerdialog.cpp`
- 位置：`RegisterDialog::RegisterDialog(...)`

## 商品添加 / 编辑界面
- 文件：`productdialog.cpp`
- 位置：`ProductDialog::ProductDialog(...)`
- 价格输入框用的是 `DigitStepDoubleSpinBox`。
- 日期选择：
  - `purchaseDateEdit`：进货日期
  - `expiryDateEdit`：到期日期
- 自动补全编号：
  - 函数：`ProductDialog::completeCodeFromCategory()`

## 经理商品管理界面
- 文件：`productmanagerpage.cpp`
- 位置：`ProductManagerPage::ProductManagerPage(...)`
- 查询依据下拉框：
  - 变量：`searchTypeBox`
- 查询逻辑：
  - 函数：`ProductManagerPage::searchProduct()`
- 商品表列隐藏、排序、宽度：
  - 函数：`ProductManagerPage::applyProductViewSettings(...)`

## 经理员工管理界面
- 文件：`employeemanagerpage.cpp`
- 位置：`EmployeeManagerPage::EmployeeManagerPage(...)`
- 删除员工：
  - 函数：`EmployeeManagerPage::deleteEmployee()`
- 修改员工信息：
  - 函数：`EmployeeManagerPage::editEmployee()`

## 经理销售记录界面
- 文件：`salesrecordpage.cpp`
- 位置：`SalesRecordPage::SalesRecordPage(...)`
- 年份和月份选择：
  - `yearSpin`
  - `monthSpin`

## 销售员销售界面
- 文件：`employeesaleswindow.cpp`
- 位置：`EmployeeSalesWindow::EmployeeSalesWindow(...)`

## 表格点一行再次取消选中
- 文件：`toggletableview.h`
- 类：`ToggleTableView`

## 所有表格等宽列
- 文件：`database.cpp`
- 函数：`setEqualColumnWidths(QTableView *view, int width)`
