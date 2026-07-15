# 商品销售管理系统 - Qt前端详细设计方案 (修订版)

## 一、总体架构

- **主窗口（MainWindow）**：继承自`QMainWindow`，**本身不可见**，中央控件是一个`QStackedWidget`作为页面容器。
- **页面容器**：`QStackedWidget`管理三个主要页面（`WelcomePage`, `DashboardPage`, `ManagementPage`），通过`setCurrentIndex()`切换。
- **全局上下文（AppContext）**：单例或指针对象，在`MainWindow`中维护，包含当前`Warehouse`指针、`QSettings`指针、当前用户等（未来扩展）。所有页面通过`MainWindow`提供的接口获取上下文数据。
- **辅助对话框**：仅用于复杂表单（如`AddGoodsDialog`）和通用确认框（`QMessageBox`），简单操作（新建分类、转移分类）使用内联控件。

---

## 二、WelcomePage（欢迎页）

### 布局（VBoxLayout）

``` text
+------------------------------------------+
| [ HBoxLayout ]                           |
| Welcome!                                 |
+------------------------------------------+
| [新建仓库] [刷新列表] [重命名仓库]       |  ← 水平布局，默认等宽
| 悬停在“新建仓库”上时，该按钮权重增大   |
| 至 50%，其余按钮收缩，详见“动画设计”   |
+------------------------------------------+
| QListWidget                              |
| - warehouse1.txt                         |
| - warehouse2.txt                         |
| - ...                                    |
|                                          |
+------------------------------------------+
```

### 控件
- `QLabel *WelcomeLabel`（文本：Welcome！）
- `QPushButton *newWarehouseBtn`（文本：新建仓库）
- `QPushButton *refreshBtn`（文本：刷新列表）
- `QPushButton *renameWarehouseBtn`（文本：重命名仓库）
- `QListWidget *fileList`

### 交互
- 页面加载时扫描`./warehouses/*.txt`目录，列出所有仓库文件（仅显示文件名，不含路径）。
- 单击`fileList`项 → 高亮选中。
- 双击`fileList`项 → 发射`warehouseSelected(const QString& path)`信号。
- **新建仓库按钮**：默认与其余按钮等宽（各占 1/3）。鼠标悬停时，该按钮平滑拉伸至容器宽度的 50%，其余两个按钮相应缩小，松开后恢复等宽（动画详见“七、动画设计”）。
- 点击`newWarehouseBtn` → 弹出`QInputDialog::getText()`输入新仓库名称（不含扩展名），成功后创建文件并刷新列表。
- 点击`refreshBtn` → 重新扫描目录，更新列表。
- 点击`renameWarehouseBtn` → 如果目前有高亮项，弹出输入框，重命名后刷新列表。

**加载仓库后的错误处理**：
- 在 `MainWindow` 中调用 `warehouse.loadData(errorLines)` 后，若 `errorLines` 非空，弹出 `QMessageBox::warning`，提示“数据加载时发现 N 个问题，建议检查并修复”。
- 对话框提供两个按钮：“查看详情”（打开只读错误报告对话框，列出所有 `errorLines`）和“稍后处理”。
- 若 `loadData` 返回 `false`，弹窗“数据文件严重损坏，程序无法继续”并退出程序。
- 主窗口状态栏显示警告图标（⚠️），鼠标悬停提示“存在数据错误，点击查看”，点击后显示错误详情（后续可扩展为修复对话框）。

### 信号与槽

**信号**：
- `void warehouseSelected(const QString& path);`（双击 `fileList` 项时触发）

**槽**：
- `void refreshFileList();`（刷新列表，由新建/重命名成功后内部调用）

**内部状态管理（无需信号）**：
- 单击 `fileList` 项 → 高亮选中，并保存当前选中的文件名为 `m_selectedFile`（成员变量），供重命名操作读取。
- 重命名操作从 `m_selectedFile` 获取当前选中的文件名，无需额外信号。

### 初始化
- 程序启动时扫描目录，如果为空则创建一个`default.txt`。

---

## 三、DashboardPage（仪表盘）

### 整体布局（HBoxLayout）
- 左栏（固定宽度 20%）：垂直布局，包含快速销售、搜索、分类列表。
- 右栏（剩余 80%）：垂直布局，上方图表区，下方预警区。

### 左栏设计（从上到下）

``` text
+------------------------------------------+
| 快速销售                                 |
| 编号: [LineEdit]                         |
| 数量: [LineEdit] (仅接受数字)            |
| [确认销售]                               |
| ---分割线---                             |
| 搜索                                     |
| [LineEdit] [x] 模糊搜索                  |
| [删除分区][重命名分区]（带悬停展开动画） |
| [ + ] (带悬停展开动画，详见“动画设计”) |
| ---分割线---                             |
| 分类列表                                 |
| 📦 全部商品 (总件数)                    |  ← 虚拟分类项，仅当仓库非空时显示
| * 电子产品 (12)                          |
| * 办公用品 (8)                           |
| * 食品 (5)                               |
| ...                                      |
| (滚动区域)                               |
| ---分割线---                             |
+------------------------------------------+
```

**控件**：
- `QLineEdit *quickIdEdit`（占位文本 "商品编号"）
- `QLineEdit *quickQtyEdit`（占位文本 "数量"，使用`QIntValidator`）
- `QPushButton *quickSellBtn`（文本 "确认"）
- `QLineEdit *searchEdit`（占位文本 "搜索分类..."）
- `QCheckBox *fuzzyCheckBox`（文本 "模糊"）
- `QWidget *addCategoryContainer`（包含“+”、输入框和确认按钮，默认折叠，悬停展开，详见“七、动画设计”）
  - `QPushButton *addButton`
  - `QLineEdit *nameEdit`
- `QWidget *modifyCategoryContainer`（包含删除按钮、重命名按钮和输入框，默认折叠，悬停展开）
  - `QPushButton *deleteButton`
  - `QPushbutton *renameButton`
  - `QLineEdit *newnameEdit`
- `QListWidget *categoryList`（自定义Item，显示分类名和商品数，悬停时高亮）。**列表顶部固定插入虚拟项“📦 全部商品”**（显示总有效商品数），其余为实际分类。

**交互**：
- 打开页面时，`quickIdEdit`自动获得焦点（`setFocus()`）。
- **分类列表加载规则**：
  - 调用 `warehouse.getCategories()` 获取所有分类。
  - 若分类列表为空（仓库无任何分类），则 **不显示“📦 全部商品”虚拟项**，仅显示“+”新建分类按钮，提示用户先创建分类。
  - 若分类列表非空，则在列表顶部固定插入“📦 全部商品”虚拟项，其后显示总有效商品数，下方列出所有具体分类及其商品数。
- 快速销售：输入编号和数量，点击确认或按回车 → 调用`warehouse.sellGoods`。
  - 若 `quickIdEdit` 内容不是合法的 7 位编号（长度或校验位错误），弹出 `QMessageBox::warning` 提示“商品编号不合法”。
  - 若 `quickQtyEdit` 值 ≤ 0，弹出警告“销售数量必须为正整数”。
  - 销售成功后刷新右栏图表和预警，清空输入框并重新聚焦。
- 搜索分类：输入文字 → 实时过滤`categoryList`（根据`fuzzyCheckBox`决定是否模糊匹配）。该过滤作用于所有分类项（含虚拟项“全部商品”）。
- 悬停分类Item → 颜色高亮。
- 点击分类Item → 颜色变深，发射`categoryClicked(const QString& name)`信号。
  - 若点击“📦 全部商品”，`name` 传**空字符串**（`QString()` 或 `""`）。
  - 若点击具体分类，`name` 传分类名。
- 新建分类：悬停在`addCategoryContainer`上时，容器内的“+”标签平滑左移，同时右侧展开输入框和确认按钮；输入新分类名后点击确认或按回车，调用`warehouse.createCategory`，刷新列表并自动折叠容器。
  - **焦点锁定**：当输入框（`m_nameEdit`）获得焦点时，鼠标离开容器**不会触发折叠动画**，防止用户正在输入时丢失内容。只有输入框失去焦点且鼠标已离开容器，才执行折叠。
  - 按 `Escape` 键立即取消输入并折叠（无动画）。

**编辑脏数据检查（`editDirty`）**：
- 在编辑模式下，任何控件值变化时设置 `editDirty = true`。
- 退出时弹出`QMessageBox`询问用户是否保存当前页面中的编辑结果。

### 右栏设计（VBoxLayout）

``` text
+------------------------------------------+
| 图表区 (GridLayout, 2x2)                 |
| +----------+ +----------+                |
| | 分类占比 | | 价格分布 |                |
| | (饼图)   | | (直方图) |                |
| +----------+ +----------+                |
| +----------+ +----------+                |
| | 库存分布 | | 生产商   |                |
| | (柱状图) | | Top10    |                |
| +----------+ +----------+                |
| 高度占 65%                               |
+------------------------------------------+
| 预警区 (HBoxLayout, 三列)                |
| +---------+ +--------+ +--------+        |
| |库存预警 | |到货提醒| |过期提醒|        |
| |(List)   | |(List)  | |(List)  |        |
| +---------+ +--------+ +--------+        |
| 高度占 35%                               |
+------------------------------------------+
```

**控件**：
- 图表区域：用`QGridLayout`放置四个`QChart`（每个加载一个QtChart组件，不启用QML）。
- 预警区域：三个`QGroupBox`，每个内部包含一个`QListWidget`显示对应预警商品列表（包含商品全部字段 + 所属分类）。

**交互**：
- 预警列表中的商品**不可点击**，仅用于展示。
- 预警阈值从`QSettings`读取（低库存阈值、到货天数阈值、过期天数阈值）。
- 每次刷新仪表盘（如快速销售成功、分类变更）时，重新计算图表数据和预警列表。

### 信号与槽：

**信号**：
- `void categoryClicked(const QString& name);`（点击分类项时触发，空字符串表示“全部商品”）
- `void switchWarehouseRequested();`（由**菜单栏“切换仓库”** 触发，DashboardPage 仅转发此信号）

**公共槽**：
- `void refreshDashboard();`（重新读取所有数据，更新图表和预警，并刷新分类列表的显示条件）
- `void onQuickSell();`（执行快速销售）
- `void onCreateCategory(const QString& name);`（执行新建分类）

## 四、ManagementPage（管理页）

### 整体布局（HBoxLayout）
- 左栏（15%）：分类信息、返回、搜索、浏览条件。
- 中栏（60%）：表格视图（主区域）。
- 右栏（25%）：详情面板。

### 左栏设计（VBoxLayout）

``` text
+------------------------------------------+
| 当前分类: 全部商品 (或 电子产品)         |
| [← 返回仪表盘]                          |
| ---分割线---                             |
| 搜索                                     |
| [x] 通过ID: [LineEdit]                   |
| [x] 通过商品名：[LineEdit] [x] 模糊搜索  |
| [x] 通过供货商：[LineEdit] [x] 模糊搜索  |
| ---分割线---                             |
| 筛选条件                                 |
| [x] 价格: [min] ~ [max]                  |
| [x] 库存: [min] ~ [max]                  |
| [x] 到货: [起始] ~ [结束]                |
| [x] 保质期: [起始] ~ [结束]              |
| [应用浏览] [重置浏览]                    |
+------------------------------------------+
```

**控件**：
- `QLabel *currentCategoryLabel`
- `QPushButton *backBtn`（返回仪表盘）
- `QCheckBox *idCheckBox`（是否通过id搜索）
- `QLineEdit *idEdit`（占位文本 "通过编号搜索..."）
- `QCheckBox *nameCheckBox`（是否通过名称搜索）
- `QLineEdit *nameEdit`（占位文本 "通过名称搜索..."）
- `QCheckBox *nameFuzzyCheckBox`（名称模糊搜索开关）
- `QCheckBox *manufacturerCheckBox`（是否通过供货商搜索）
- `QLineEdit *manufacturerEdit`（占位文本 "通过供货商搜索..."）
- `QCheckBox *manufacturerFuzzyCheckBox`（供货商模糊搜索开关）
- `QCheckBox *priceCheckBox`（是否通过价格区间筛选）
- `QDoubleSpinBox *minPriceSpin`, `maxPriceSpin`（范围0~9999，步长10）
- `QCheckBox *stockCheckBox`（是否通过库存区间筛选）
- `QSpinBox *minStockSpin`, `maxStockSpin`
- `QCheckBox *arrivaldateCheckBox`（是否通过最近进货日期筛选）
- `QDateEdit *startArrivalEdit`, `endArrivalEdit`（默认今天和一年后）
- `QCheckBox *expirydateCheckBox`（是否通过最早过期日期筛选）
- `QDateEdit *startExpiryEdit`, `endExpiryEdit`
- `QCheckBox *categoryCheckBox`（是否通过分区筛选）
- `QListWidget *categoryMenu`（仅限全局模式下可见）
  - `QCheckbox *categorySelector`（复选框实现）
- `QPushButton *applyFilterBtn`, `resetFilterBtn`

**交互**：
- **分类列表点击**：
  - 点击“ 全部商品” → 设置 `AppContext.currentCategory = std::nullopt`，清空搜索框和所有筛选条件（与重置等效），调用 `refreshManagement()` 加载 `browseAll()` 的数据。
  - 点击具体分类 → 设置 `AppContext.currentCategory = 分类名`，**同样清空搜索框和筛选条件**，调用 `refreshManagement()` 加载 `browseByCategory(分类名)` 的数据。
  - 点击分类时，清除上一个选中项的高亮，高亮当前项。
- **搜索输入**实时触发（`textChanged`），根据 `fuzzyCheckBox` 决定是否模糊匹配，在现有 `m_displayList` 基础上进行二次过滤。
- **浏览条件**点击“应用”后，前端自动生成S表达式，并调用后端`Query::queryBySExpr`，重新计算 `m_displayList` 并刷新表格。同时在`AppContext`里加载当前的筛选条件，以实现期有效。
- **重置按钮**：恢复所有条件为默认值（价格0~无穷，库存0~无穷，日期全范围），**同时清空 `searchEdit` 和取消 `fuzzyCheckBox` 的勾选**。

**多分类聚焦（仅全局模式下可见）**：
- 当 `AppContext.currentCategory` 为空时，在搜索框下方显示 `QListWidget` 的“分类筛选”控件，列出所有分类，默认全选。
- 用户取消勾选某些分类后，`refreshManagement()` 会在内存中过滤掉这些分类的商品（基于 `GoodsWithCategory` 结构体中的分类名），再应用其他筛选条件。
- 状态栏显示“已选 N 个分类，共 X 件商品”。

**查询机制**:
- UI控件动态生成S表达式（`buildSExpr()`），调用`apiQueryBySEpxr`获取具体数据。如果表达式为空，则默认返回全部商品。如果中间出现错误，则应当通过`errorLines`反馈（弹出`QMessageBox`）
- 特殊值约定：QDate(1900, 1, 1)---setSpecialValueText-->“不限” QDate(9999, 12, 31)--->“长期”“今天”的特殊值只在初始化时给出。

### 中栏设计（VBoxLayout）

``` text
+------------------------------------------+
| QStackedWidget                           |
| 页面0: QTableView (列表视图)             |
| 页面1: QListView (图标视图，预留)        |
| 页面0作为默认                            |
+------------------------------------------+
| 状态栏: "共 X 件商品"                    |
+------------------------------------------+
```

**控件**：
- `QTableView *tableView`（使用`QStandardItemModel`）
- `QListView *iconView`（预留）
- `QLabel *statusLabel`

**交互**：
- 点击`tableView`行 → 填充右侧详情面板（只读模式）。如果当前处于编辑模式（`editDirty == true`），弹出提示“请先提交或取消当前编辑”，不允许切换行。
- 双击行（可选） → 快速进入编辑模式（但按照设计方案，编辑通过右侧按钮触发）。
- 表格支持点击表头排序，排序仅作用于当前内存中的 `m_displayList`，不触发后端查询。

### 右栏设计（VBoxLayout）

``` text
+------------------------------------------+
| 商品详情                                 |
| 编号: [Label]                            |
| 名称: [Label / LineEdit] (编辑时切换)    |
| 单价: [Label / LineEdit]                 |
| 厂商: [Label / LineEdit]                 |
| 库存: [Label / LineEdit]                 |
| 到货: [Label / DateEdit]                 |
| 保质期: [Label / DateEdit]               |
| [✏️编辑] [✅提交] [❌取消]              |  ← 辅助操作纯图标
| ---分割线---                             |
| 销售                                     |
| 数量: [SpinBox] [🛒销售]                |
| ---分割线---                             |
| [🗑️下架] [📂转移至...][ComboBox]        |
+------------------------------------------+
```

**控件**：
- 只读模式：全部`QLabel`。
- 编辑模式：对应的`QLineEdit` / `QDateEdit`（通过`QStackedWidget`或`setVisible`切换）。单价使用 `QDoubleValidator` 限制非负数，库存使用 `QIntValidator` 限制非负数。
- `QPushButton *editBtn`（纯图标 ✏️），`submitBtn`（✅），`cancelBtn`（❌）（`submitBtn`和`cancelBtn`仅在编辑模式显示）。
- `QSpinBox *sellQtySpin`（最小值1）
- `QPushButton *sellBtn`（纯图标 🛒）
- `QPushButton *removeBtn`（🗑️），`moveBtn`（📂）
- `QComboBox *selectiveCategory`

**交互**：
- 选中行 → 填充右栏，只读模式。
- 点击“编辑” → 切换到编辑模式，禁用`sellBtn`、`removeBtn`、`moveBtn`，`editBtn`隐藏，`submitBtn`和`cancelBtn`显示。缓存当前商品数据快照（用于取消时还原）。
- 点击“提交” → 验证数据（非空、数值合法），调用`warehouse.updateGoods`，若成功则设置 `editDirty = false`，切回只读模式，调用 `refreshManagement()` 刷新表格并重新填充当前行详情；若失败则弹窗显示错误信息，保持编辑模式。
- 点击“取消” → 放弃修改，从快照恢复数据，`editDirty = false`，切回只读模式。
- 点击“销售” → 读取数量，调用`warehouse.sellGoods`，成功后调用 `refreshManagement()` 更新表格和库存数据，重新填充当前行详情；失败则弹窗报错。
- 点击“下架” → 弹出`QMessageBox::question`确认，确认后调用`warehouse.removeGoods`，刷新列表并清空详情面板。
- 点击“转移” → 弹出一个内联选择器（或简单对话框），显示目标分类下拉列表。**该下拉列表排除当前商品所属分类**；若仓库中仅有一个分类（即无可转移目标），则“转移”按钮置灰（`setEnabled(false)`）。选择目标分类后调用 `warehouse.moveGoodsToCategory`，成功后刷新列表并清空详情面板（因商品ID已变）。

**编辑脏数据检查（`editDirty`）**：
- 在编辑模式下，任何控件值变化时设置 `editDirty = true`。
- 退出时弹出`QMessageBox`询问用户是否保存当前页面中的编辑结果。

**数据流**：
- 成员变量：
  - `std::vector<Warehouse::GoodsWithCategory> m_displayList`（当前显示的商品列表）
  - `editDirty = false`当前页面是否有未保存的编辑
- 刷新逻辑：
  - `refreshManagement()`：
    1. 根据 `m_currentCategory` 调用 `warehouse.browseAll()` 或 自动通过`buildSExpr()`生成S表达式并通过`apiQueryBySExpr`获取基础数据。
    2. 若为全局模式且多分类筛选有选择，则在内存中过滤掉未选中分类的商品，实现方式同上文存在`m_currentCategory`的情况。
    3. 应用价格/库存/日期筛选条件。
    4. 应用搜索关键词（名称或编号，根据模糊开关）。
    5. 更新 `QStandardItemModel`。
    6. 清空右侧详情面板。
    7. 更新状态栏（显示总数，若全局模式且有多分类筛选，显示“已选 N 个分类”）。

### 信号与槽：

**信号**：
- `void backToDashboardRequested();`（点击返回按钮且通过脏检查后触发）
- `void statusMessageChanged(const QString& message, int timeout);`（需要更新状态栏时触发）

**公共槽**：
- `void loadCategory(const QString& category);`（加载分类，空字符串表示全部商品，同时重置搜索和筛选）
- `void refreshManagement();`（刷新表格和状态栏）
- `void setViewMode(int mode);`（切换列表/图标视图，由菜单栏调用）

**内部槽（由页面内部控件信号触发）**：
- `void onSearchTextChanged(const QString& text);`（搜索框输入实时过滤）
- `void onFuzzyToggled(bool checked);`（模糊搜索开关切换时重新过滤）
- `void onApplyFilter();`（应用浏览条件）
- `void onResetFilter();`（重置所有筛选条件，清空搜索框）
- `void onTableRowSelected(const QModelIndex& index);`（表格行选中，填充详情面板）
- `void onEditClicked();`（进入编辑模式）
- `void onSubmitClicked();`（提交编辑）
- `void onCancelClicked();`（取消编辑）
- `void onSellClicked();`（执行销售）
- `void onRemoveClicked();`（执行下架，含确认对话框）
- `void onMoveClicked();`（执行转移，含目标分类选择）
- `bool checkAndSaveEdit();`（检查未保存编辑并提示，供MainWindow调用，返回用户是否取消操作）
- `void onCategoryFilterChanged();`（多分类聚焦的复选框变化时重新过滤）

## 五、对话框（QDialog）

### 1. AddGoodsDialog
- **用途**：上架新商品。
- **布局**：`QFormLayout`包含所有字段（分类、名称、单价、厂商、库存、到货日期、保质期、图片路径）。
- **分类字段逻辑**：
  - 如果 `AppContext.currentCategory` 有值（非空），则分类下拉框**自动填充为该分类**，并置为只读，用户无需选择。
  - 如果 `AppContext.currentCategory` 为空（全局模式），则显示分类下拉框，列出仓库所有分类，**必须选择一个分类才能确认**。
- **确认**：调用`warehouse.addGoods`，成功则`accept()`，失败则显示错误（`QMessageBox::critical`）。
- **其他字段校验**：名称、厂商非空；单价、库存非负；到货日期不晚于今天；保质期格式合法。

### 2. SettingsDialog
- **用途**：全局参数设置。
- **布局**：第一个是`QFormLayout`包含：
  - 低库存阈值（`QSpinBox`）
  - 高库存阈值（`QSpinBox`）
  - 到货提醒天数（`QSpinBox`）
  - 过期提醒天数（`QSpinBox`）
  - 默认视图模式（`QComboBox`：列表/图标）
- 第二个是`QVBoxLayout`，包含：
  - 筛选条件的操作，同ManagementPage页面中的设置。此处存储所有的控件值的快照，和ManagementPage对齐。
- **确认**：保存到`QSettings`，发射`settingsChanged()`信号（可选）。

### 其他对话框
- 使用`QMessageBox`的标准方法（信息、警告、提问、错误）。

---

## 六、全局设置与菜单栏

- **菜单栏**（位于MainWindow）：
  - **文件**：保存仓库（Ctrl+S）、切换仓库、退出。
  - **视图**：切换视图模式（列表/图标，调用ManagementPage的切换槽）。
  - **设置**：打开`SettingsDialog`。
  - **帮助**：关于。
- **状态栏**：显示当前仓库文件名、商品总数、总库存等（由MainWindow统一更新）。

**菜单项与信号/槽连接（MainWindow 负责）**：
- “保存仓库” → 调用 `AppContext.currentWarehouse->saveData()`，并刷新状态栏。
- “切换仓库” → 调用 `dashboardPage->switchWarehouseRequested()` 信号（由 DashboardPage 转发给 MainWindow），检查脏状态后返回 WelcomePage 并刷新列表。
- “退出” → 检查脏状态，保存后退出。

---

## 七、动画设计

**动画触发机制说明**：
- 本项目的所有动画（悬停展开、按钮拉伸）均由 **Qt 事件**（`enterEvent`/`leaveEvent`）直接触发，**不经过信号槽机制**。
- 这样做的好处是：动画是纯UI行为，不涉及业务逻辑，使用事件处理更加直接高效。
- 信号槽仅用于跨控件/跨页面的业务通信（如点击按钮触发表单提交、分类切换等）。

本项目包含两类动画交互，均基于 `QPropertyAnimation` 实现，不引入 QML。

### 1. DashboardPage 的“新建分类”悬停展开动画

### 交互行为
- **默认状态**：一个居中的“+”标签（`QLabel`），无输入框，无按钮。
- **鼠标悬停**：当鼠标进入该控件所在的容器（`QWidget`）时，触发平滑动画：
  1. “+”标签以缓动曲线向左移动（至容器左侧约 10%~20% 位置）。
  2. 同时在右侧淡入一个输入框（`QLineEdit`）和一个确认按钮（`QPushButton`）。
  3. 输入框自动获得焦点，用户可直接键入新分类名称。
- **鼠标离开**：如果用户未点击确认且移出容器，则反向动画，恢复默认状态。
- **确认或取消**：
  - 按回车或点击确认按钮 → 调用 `warehouse.createCategory`，成功后刷新分类列表并自动折叠。
  - 按 `Escape` 键 → 放弃输入，立即折叠（无动画）。
  - 点击容器外部（但仍在窗口内）→ 不处理，仅鼠标离开容器时触发折叠。
- **交互行为补充**：
  - **焦点锁定**：当输入框（`m_nameEdit`）获得焦点时，鼠标离开容器**不触发折叠动画**。只有输入框失去焦点（`focusOutEvent`）且鼠标已离开容器，才执行折叠。
  - 按 `Escape` 键立即取消输入并折叠（无动画）。
  - 按回车或点击确认按钮 → 执行创建，成功后折叠。

### 技术实现要点
- **触发区域**：监听整个容器的 `enterEvent` / `leaveEvent`，而非仅监听“+”标签本身，避免鼠标移出标签导致动画循环。
- **动画引擎**：使用 `QPropertyAnimation` 对 `QLabel` 的 `geometry` 或 `pos` 进行动画，配合 `QEasingCurve::InOutQuad` 缓动曲线，时长 200ms。
- **显隐控制**：输入框和按钮的 `setVisible` 在动画开始/结束时调用，可配合透明度动画（`QGraphicsOpacityEffect`）实现淡入淡出。
- **防抖动**：
  - 在 `enterEvent` 中，若当前有动画正在播放，立即停止并反向。
  - 使用一个标志位 `m_isExpanded` 记录当前展开状态，避免重复触发。
  - 在 `leaveEvent` 中，检查 `m_nameEdit->hasFocus()`，若为 true 则不做任何操作，直接返回。
  - 为输入框安装事件过滤器，监听 `FocusOut` 事件，在失焦时调用 `collapse()`。
- **布局管理**：容器内部使用 `QHBoxLayout`，通过调整 `setStretchFactor` 或直接 `setGeometry` 实现“+”标签的位移。推荐使用 `QSpacerItem` 的 `changeSize` 配合 `invalidate()` 触发布局重算，但更稳定的做法是直接操作子控件的 `move()`，并设置 `setAutoFillBackground(true)` 避免重绘闪烁。

### 控件结构
``` text
+--------------------------------------------------+
|  AddCategoryContainer (QWidget)                  |
|  HBoxLayout                                      |
|  [“+”标签 (QLabel)]  [输入框]  [确认按钮]      |
|  默认：标签居中，输入框和按钮隐藏                |
|  悬停：标签左移，输入框和按钮显示                |
+--------------------------------------------------+
```

### 代码骨架（简述）
```cpp
class AddCategoryContainer : public QWidget {
    Q_OBJECT
public:
    explicit AddCategoryContainer(QWidget *parent = nullptr);
signals:
    void categoryCreated(const QString& name);
protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void onConfirm();
    void onCancel();
private:
    QLabel *m_plusLabel;
    QLineEdit *m_nameEdit;
    QPushButton *m_confirmBtn;
    QPropertyAnimation *m_animation;
    bool m_isExpanded;
    void expand();
    void collapse(bool animate = true);
};

```

### 2. WelcomePage 的“新建仓库”按钮拉伸动画、以及DashboardPage中的“重命名分区”按钮拉伸动画
- **交互行为**：
  - 默认状态：三个按钮（新建仓库、刷新列表、重命名仓库）在水平布局中**等宽**（各占约 33.3%）。
  - 鼠标悬停到“新建仓库”按钮时，该按钮的**宽度**平滑增加至容器总宽度的 **50%**，其余两个按钮等分剩余空间（各占 25%）。
  - 鼠标离开“新建仓库”按钮后，三个按钮平滑恢复等宽。
  - **不会发生死循环**：因为按钮尺寸扩大，鼠标仍在按钮区域内，离开事件只在鼠标完全移出按钮边界时触发。
- **技术实现**：
  - 使用 `QHBoxLayout` 管理三个按钮，通过 `setStretchFactor` 控制每个按钮的拉伸权重。
  - 在 `enterEvent` 中启动动画，将 `newWarehouseBtn` 的拉伸因子从 1 平滑过渡到 **3**（因为总权重为 1+1+1=3，目标 50% 意味着权重为 3，而另两个权重保持 1，总权重变为 5，则占比为 3/5=60%，为更接近 50% 可微调权重为 2.5+1+1=4.5，或者直接使用 `setStretchFactor` 设置具体像素宽度，推荐使用 `setStretchFactor` 配合 `QWidget::setFixedWidth` 或 `minimumWidth`，但最简单是操作布局的 `setStretchFactor`）。
  - 更稳定的做法：不使用拉伸因子，而是使用 `QPropertyAnimation` 直接修改 `newWarehouseBtn` 的 `minimumWidth`（或 `fixedWidth`），并让其他按钮自动适应剩余空间（设置 `sizePolicy` 为 `Expanding`）。
  - 动画时长 150ms，缓动曲线为 `QEasingCurve::InOutQuad`。
- **防抖与状态管理**：
  - 在进入事件中，若动画正在播放，先停止并反向，再开始新动画。
  - 使用 `bool m_isHovered` 标记当前是否悬停，避免重复触发。
- **控件结构**：
  ``` text
  +--------------------------------------------------+
  |  HBoxLayout (按钮容器)                           |
  |  [新建仓库] [刷新列表] [重命名仓库]              |
  |  默认：等宽 (1:1:1)                              |
  |  悬停：新建仓库占比 50%，其余均分剩余            |
  +--------------------------------------------------+
  ```
- **代码骨架（简述）**：
  ```cpp
  class WelcomePage : public QWidget {
      // ...
  protected:
      bool eventFilter(QObject *watched, QEvent *event) override;
  private slots:
      void onNewHoverEnter();
      void onNewHoverLeave();
  private:
      QPushButton *m_newBtn, *m_refreshBtn, *m_renameBtn;
      QPropertyAnimation *m_stretchAnim;
      bool m_isNewHovered;
      void updateButtonStretch(int newWeight);
  };
  ```
 - 为“新建仓库”按钮安装事件过滤器，在 `enterEvent`/`leaveEvent` 中触发动画。

 - 动画更新一个自定义权重值，再调用 `layout()->setStretchFactor` 或直接设置按钮的 `sizePolicy` 实现平滑变化。

**注意**：此动画与 Dashboard 的动画逻辑独立，但均使用相同的 `QPropertyAnimation` 技术栈，风格统一

---
## 八、交互细节规范

### 1. Tooltip（悬停提示）

- **所有纯图标按钮**必须设置 `setToolTip()`，悬停时显示功能说明。
- **常用业务按钮的 Tooltip 文案**：

| 按钮 | Tooltip 文案 |
|------|-------------|
| 编辑 | "编辑商品信息" |
| 提交 | "提交修改" |
| 取消 | "取消修改" |
| 销售 | "销售该商品" |
| 下架 | "下架该商品（不可恢复）" |
| 转移 | "转移该商品到其他分类" |
| 刷新列表 | "刷新仓库列表" |
| 重命名仓库 | "重命名选中的仓库文件" |
| 新建仓库 | "创建新的仓库文件" |
| 返回仪表盘 | "返回仪表盘" |
| 应用筛选 | "应用当前筛选条件" |
| 重置筛选 | "重置所有筛选条件" |

### 2. 悬停状态（Hover）

- **分类列表项（Dashboard / Management）**：
  - 鼠标悬停时，背景色变为浅灰色（`#F0F0F0`），圆角边框。
  - 点击选中后，背景色变为深色（`#D0D0D0`），表示当前激活状态。
  - 悬停与选中状态应共存（选中项在悬停时颜色略深）。

- **表格行（ManagementPage）**：
  - 鼠标悬停时，行背景色变为浅蓝色（`#E8F0FE`）。
  - 选中行背景色为蓝色（`#D0E4FF`），悬停时保持蓝色但略深。

- **按钮悬停（所有页面）**：
  - 纯图标按钮：背景色出现浅灰色圆形/圆角矩形（`#E8ECF1`）。
  - 危险操作按钮（下架、删除）：悬停时图标变为红色（`#E74C3C`）。

### 3. 焦点管理（Focus）

- **DashboardPage**：
  - 页面打开时，`quickIdEdit` 自动获取焦点（`setFocus()`）。
  - 快速销售成功后，`quickIdEdit` 重新获取焦点。
  - 搜索分类的 `searchEdit` 不自动抢焦点，用户点击后才激活。

- **ManagementPage**：
  - 页面打开时，焦点不自动定位到任何控件，等待用户点击。
  - 进入编辑模式后，第一个可编辑字段（名称）自动获取焦点。

### 4. 键盘快捷键（Keyboard Shortcuts）

| 页面 | 快捷键 | 操作 |
|------|--------|------|
| DashboardPage | `Enter`（在编号/数量输入框） | 触发快速销售 |
| ManagementPage | `Ctrl+S` | 保存仓库（由MainWindow菜单栏统一处理） |
| 全局 | `Ctrl+Q` | 退出程序 |
| AddGoodsDialog | `Enter` | 确认上架 |
| AddGoodsDialog | `Escape` | 取消关闭对话框 |

### 5. 动画反馈（Animation Feedback）

- **新建分类悬停展开**：见“七、动画设计”第一节，此处不重复。
- **新建仓库按钮拉伸**：见“七、动画设计”第二节，此处不重复。
- **表格行选中切换**：无动画，直接切换背景色。
- **页面切换**：使用 `QStackedWidget` 默认切换效果（无过渡动画），保持简洁。

### 6. 状态栏反馈（Status Bar）

- 所有操作的成功/失败信息应通过 `statusMessageChanged` 信号发送给 MainWindow，由 MainWindow 在状态栏显示。
- 状态栏消息支持自动超时消失：成功消息默认 2 秒，错误消息默认 5 秒。
- 状态栏右侧固定显示：当前仓库文件名、商品总数、总库存量，由 MainWindow 在页面切换时统一更新。

---

## 九、特殊日期处理（“今天”和“长期”、以及最小日期）

在日期相关的输入场景中（如筛选条件、商品录入），需要便捷地选择“今天”或“长期（永不过期）”。不同场景采用不同策略：

### 1. 作为筛选条件（ManagementPage 左侧的日期区间）
- **“长期”**：虽然后端实现的时候加了一个特殊值，但是前端如果想要设置“长期”可以直接置空。后端的查询方式支持开区间。
- **“今天”**：一次性在初始化的时候填入。具体来说到货日期默认结束在今天，保质期默认起始在今天。
- **控件布局示例**：
  ``` text
  到货日期: [起始: 2024-01-01]   ~  [结束: 长期] 
  保质期:   [起始: 2024-01-01]   ~  [结束: 长期] 
  ```

### 2. 作为商品属性（AddGoodsDialog 中的保质期录入）
- 在保质期字段旁增加一个 `QCheckBox`，文字为“长期/永不过期”。
- 勾选后，`QDateEdit` 被禁用（`setEnabled(false)`），并显示为“长期”（可通过设置 `setSpecialValueText` 或直接显示文本）。
- 取消勾选则恢复日期编辑，并默认留空。
- 保存时，若勾选则存入 `Goods::DEFAULT_EXPIRY_DATE`（即 `"9999-12-31"`）。

### 3. 注意点
- “长期”的特殊值文本仅在日期达到最大值时显示，因此需确保用户无法通过手动输入超出范围。
- 所有日期控件均使用 `QDateEdit` 并设置日历弹出（`setCalendarPopup(true)`），方便用户点选。

### 4. 与后端对接
- 筛选时，若日期为特殊值（如 `9999-12-31`），在构建查询条件时将其视为无限制或特定逻辑（例如 `9999-12-31` 表示永不截止）。
- 存储时，保质期字段若为 `DEFAULT_EXPIRY_DATE`，则视为永不过期。
---

## 十、图标与样式

**图标策略**：
- 优先使用`QStyle::SP_*`标准图表，特殊图标下载SVG后放入`resources/icons/`目录并使用`.qrc`资源文件管理。
- **主要行动按钮**（如“确认销售”、“确认上架”）使用 **图标+文字** 形式，确保一目了然。
- **辅助操作按钮**（如编辑、提交、取消、下架、转移、刷新、返回等）使用 **纯图标**，节省空间，界面更干净。
- 所有纯图标按钮必须设置 `setToolTip()`，悬停时显示功能说明。

**图标映射表**：

| 按钮 | 场景 | 图标 | 类型 |
|------|------|------|------|
| 新建仓库 | WelcomePage | ➕ `fa-plus` | 图标+文字 |
| 刷新列表 | WelcomePage / Dashboard | 🔄 `fa-rotate-right` | 纯图标 |
| 重命名仓库 | WelcomePage | ✏️ `fa-pencil` | 纯图标 |
| 确认销售 | Dashboard（快速销售） | 💰 `fa-cart-shopping` + “销售” | 图标+文字 |
| 新建分类 | Dashboard（悬停展开） | ➕ `fa-plus` | 纯图标 |
| 返回仪表盘 | ManagementPage | ⬅️ `fa-arrow-left` | 纯图标 |
| 编辑 | ManagementPage 详情 | ✏️ `fa-pen` | 纯图标 |
| 提交 | ManagementPage 编辑模式 | ✅ `fa-check` | 纯图标（绿色） |
| 取消 | ManagementPage 编辑模式 | ❌ `fa-xmark` | 纯图标（红色） |
| 销售 | ManagementPage 详情 | 🛒 `fa-bag-shopping` | 纯图标 |
| 下架 | ManagementPage 详情 | 🗑️ `fa-trash-can` | 纯图标（红色悬停） |
| 转移 | ManagementPage 详情 | 📂 `fa-folder-arrow-right` | 纯图标 |
| 应用筛选 | ManagementPage 左栏 | 🔍 `fa-magnifying-glass` | 纯图标或图标+“应用” |
| 重置筛选 | ManagementPage 左栏 | ↩️ `fa-undo` | 纯图标 |
| 切换视图 | 菜单栏 | 📋 `fa-table` / 🖼️ `fa-grid-2` | 纯图标 |
| 设置 | 菜单栏 | ⚙️ `fa-gear` | 纯图标 |
| 关于 | 菜单栏 | ℹ️ `fa-circle-info` | 纯图标 |

**视觉风格**：
- 所有图标按钮尺寸统一为 `32x32` 像素，图标缩放至 `20x20`。
- 危险操作（删除、下架）图标默认灰色，悬停时变为 `#E74C3C`（红色）。
- 使用全局 QSS 文件，给 `QToolButton` 设置 `border: none; border-radius: 6px; padding: 4px;`，悬停时 `background-color: #E8ECF1;`。
- 颜色主题：柔和蓝灰为主，强调色为 `#2E86AB`（信息）和 `#E74C3C`（危险）。

---

## 十一、AppContext 设计（MainWindow持有）

``` cpp
class AppContext {
public:
    Warehouse* currentWarehouse;   // 当前仓库
    QSettings* settings;           // 全局配置
    std::optional<std::string> currentCategory; // 当前管理页聚焦的分类（nullopt 表示“全部商品”）

    struct DashboardCache {
        OverallStatistics overall;
        std::vector<CategoryStatistics> categories;
        std::vector<PriceInterval> priceHistogram;
        StockDistribution stockDist;
        std::vector<ManufacturerStatistics> manufacturerStats;
        bool valid = false;  // 标记缓存是否有效
    };
    // 未来扩展：用户、多仓库管理器等
};
```

- `currentCategory` 在以下时机被更新：
  - 从 `WelcomePage` 双击进入仓库时，置为 `std::nullopt`（默认进入“全部商品”视图）。
  - 从 `Dashboard` 点击具体分类进入管理页时，置为该分类名。
  - 在 `ManagementPage` 内部点击“全部商品”虚拟项时，置为 `std::nullopt`。
  - 点击左侧具体分类时，置为该分类名。
- MainWindow在切换页面时，将`AppContext`的指针传递给每个页面（或通过信号传递必要数据）。

---

## 十二、页面切换与保存检查流程

- **页面激活刷新策略（核心规则）**：
  1. 整个项目一共有三层数据：磁盘文件中的数据（磁盘层）、内存中数据（内存层）、前端显示的数据（显示层），其中内存永远比其他两者快。但`isDirty`只协调内存层和磁盘层，显示层与内存层之间的区别由`DashboardCache.valid`维护。
  2. 显示层的数据区分还有以下分层：同一个页面内部的、跨页面的。其中同一个页面内部应该在数据更新的时候立刻更新本页面的前端显示，而跨页面的显示层差异需要通过在`AppContext`中设置标志来完成。
  3. 此外，还有用户在正编辑某个值的时候退出当前界面的情况，这种情况应该提醒用户保存自己的编辑，用editDirty保存即可，每个页面自己保存一份。
  4. 每次切换到 `DashboardPage` 时（通过 `QStackedWidget` 的 `currentChanged` 信号或重写 `showEvent`），**无条件调用 `refreshDashboard()`**，确保仪表盘为最新汇总数据。
  5. 每次切换到 `ManagementPage` 时，检查 `AppContext.currentCategory` 的值（空则说明全局），并调用 `refreshManagement()` 加载对应数据。
  6. 从 `ManagementPage` 返回 `DashboardPage` 时，先检查 `m_hasUnsavedEdit` ，处理后由规则3自动刷新。
  7. 从 `ManagementPage` 返回 `DashboardPage` 时也是同理，比如要加载高亮行等。
  8. 刷新粒度：
     - `refreshDashboard()`：全量刷新（图表、预警、分类列表）。
     - `refreshManagement()`：全量刷新表格，清空右侧详情面板，取消焦点。
- **切换仓库**（从任何页面点击菜单"切换仓库"）：
 - 检查 `editDirty`（编辑状态）和 `isDirty`，若有未保存则分别弹出 `QMessageBox`（保存/不保存/取消）。
 - 若取消则中断，否则执行切换（回到 `WelcomePage`）。
- **返回仪表盘**（从`ManagementPage`点击返回）：
 - 由于数据并无丢失，只是前端界面区别，因此可以直接通过`refreshDashboard()`函数对齐，刷新后再重置`editDirty`为`false`即可。
- **关闭窗口**：重写 `closeEvent`，检查`isDirty`，协调内存层和磁盘层，依此提示，因为不再需要考虑前端问题了。

---

## 十三、信号与槽连接规范

### 概述

- **MainWindow** 负责建立所有**跨页面**的信号-槽连接。
- **各页面** 负责建立自己内部的信号-槽连接（构造函数中完成）。
- **页面之间不直接通信**，统一经过 MainWindow 中转。

---

### 1. 跨页面连接（由 MainWindow 建立）

| 源对象 | 源信号 | 目标对象 | 目标槽 | 说明 |
|--------|--------|----------|--------|------|
| `WelcomePage` | `warehouseSelected(path)` | `MainWindow` | `onWarehouseSelected(path)` | 加载仓库，设置 `currentCategory = nullopt`，切换到 `ManagementPage` |
| `DashboardPage` | `categoryClicked(name)` | `MainWindow` | `onCategoryClicked(name)` | 更新 `currentCategory`，切换到 `ManagementPage` 并调用 `loadCategory(name)` |
| `DashboardPage` | `switchWarehouseRequested()` | `MainWindow` | `onSwitchWarehouse()` | 检查 `isDirty()`，回到 `WelcomePage` |
| `ManagementPage` | `backToDashboardRequested()` | `MainWindow` | `switchToDashboard()` | 检查 `editDirty` 和 `isDirty()`，切回 `DashboardPage` |
| `ManagementPage` | `statusMessageChanged(msg, timeout)` | `MainWindow` | `updateStatusBar(msg, timeout)` | 在状态栏显示消息 |
| `SettingsDialog` | `settingsChanged()` | `MainWindow` | `onSettingsChanged()` | 刷新所有阈值相关 UI（Dashboard 预警等） |

### 2. WelcomePage 内部连接（由 WelcomePage 构造函数建立）

| 源对象 | 源信号 | 目标槽 | 说明 |
|--------|--------|--------|------|
| `newWarehouseBtn` | `clicked()` | `onNewWarehouseClicked()` | 弹出输入框 → 创建文件 → 调用 `refreshFileList()` |
| `refreshBtn` | `clicked()` | `refreshFileList()` | 扫描 `./warehouses/` 目录，刷新列表 |
| `renameWarehouseBtn` | `clicked()` | `onRenameWarehouseClicked()` | 检查 `m_selectedFile` → 弹出输入框 → 重命名文件 → 调用 `refreshFileList()` |
| `fileList` | `itemDoubleClicked(item)` | `onFileDoubleClicked(item)` | 发射 `warehouseSelected(path)` |

**内部状态（无需信号）**：
- 单击 `fileList` 项 → 高亮选中，保存文件名至 `m_selectedFile`，供重命名操作使用。

### 3. DashboardPage 内部连接（由 DashboardPage 构造函数建立）

| 源对象 | 源信号 | 目标槽 | 说明 |
|--------|--------|--------|------|
| `quickSellBtn` | `clicked()` | `onQuickSell()` | 校验编号/数量 → 调用 `warehouse.sellGoods()` → 调用 `refreshDashboard()` → 发射 `statusMessageChanged` |
| `quickIdEdit` | `returnPressed()` | `onQuickSell()` | 同上（回车触发） |
| `quickQtyEdit` | `returnPressed()` | `onQuickSell()` | 同上（回车触发） |
| `searchEdit` | `textChanged(text)` | `onSearchTextChanged(text)` | 实时过滤 `categoryList` |
| `fuzzyCheckBox` | `toggled(checked)` | `onFuzzyToggled(checked)` | 切换模糊匹配，重新过滤分类列表 |
| `categoryList` | `itemClicked(item)` | `onCategoryItemClicked(item)` | 发射 `categoryClicked(name)`（空字符串表示“全部商品”） |
| `addCategoryContainer` | `categoryCreated(name)` | `onCategoryCreated(name)` | 调用 `warehouse.createCategory()` → 调用 `refreshDashboard()` → 发射 `statusMessageChanged` |

**公共槽**（供 MainWindow 调用）：
- `refreshDashboard()`：从仓库重新读取所有数据，更新图表、预警、分类列表。

### 4. ManagementPage 内部连接（由 ManagementPage 构造函数建立）

| 源对象 | 源信号 | 目标槽 | 说明 |
|--------|--------|--------|------|
| `backBtn` | `clicked()` | `onBackClicked()` | 检查 `editDirty`，发射 `backToDashboardRequested()` |
| `categoryList` | `itemClicked(item)` | `onCategoryItemClicked(item)` | 切换分类/全部商品，重置搜索筛选，调用 `refreshManagement()` |
| `searchEdit` | `textChanged(text)` | `onSearchTextChanged(text)` | 在 `m_displayList` 基础上实时过滤 |
| `fuzzyCheckBox` | `toggled(checked)` | `onFuzzyToggled(checked)` | 切换模糊匹配，重新应用搜索 |
| `applyFilterBtn` | `clicked()` | `onApplyFilter()` | 应用价格/库存/日期筛选，刷新表格 |
| `resetFilterBtn` | `clicked()` | `onResetFilter()` | 重置所有条件，清空搜索框，刷新表格 |
| `tableView` | `clicked(index)` | `onTableRowSelected(index)` | 若 `editDirty` 为 true 则提示；否则填充右侧详情面板 |
| `editBtn` | `clicked()` | `onEditClicked()` | 切换编辑模式，缓存快照，禁用销售/下架/转移按钮 |
| `submitBtn` | `clicked()` | `onSubmitClicked()` | 验证数据 → 调用 `warehouse.updateGoods()` → 成功则调用 `refreshManagement()` 并发射 `statusMessageChanged` |
| `cancelBtn` | `clicked()` | `onCancelClicked()` | 从快照恢复数据，切回只读模式 |
| `sellBtn` | `clicked()` | `onSellClicked()` | 读取数量 → 调用 `warehouse.sellGoods()` → 成功则调用 `refreshManagement()` 并发射 `statusMessageChanged` |
| `sellQtySpin` | `returnPressed()` | `onSellClicked()` | 同上（回车触发） |
| `removeBtn` | `clicked()` | `onRemoveClicked()` | 弹出确认框 → 调用 `warehouse.removeGoods()` → 成功则调用 `refreshManagement()` 并发射 `statusMessageChanged` |
| `moveBtn` | `clicked()` | `onMoveClicked()` | 弹出目标分类选择器（排除当前分类）→ 调用 `warehouse.moveGoodsToCategory()` → 成功则调用 `refreshManagement()` 并发射 `statusMessageChanged` |

**公共槽**（供 MainWindow 调用）：
- `loadCategory(category)`：设置 `m_currentCategory`，清空搜索和筛选，调用 `refreshManagement()`
- `refreshManagement()`：从仓库获取数据 → 应用多分类过滤 → 应用价格/库存/日期筛选 → 应用搜索词 → 更新表格模型 → 清空详情面板 → 更新状态栏
- `setViewMode(mode)`：切换 `QStackedWidget` 的当前索引（列表/图标视图）
- `checkAndSaveEdit()`：若 `editDirty` 为 true，弹出保存确认框，返回用户选择（用于阻断页面切换）

### 5. 菜单栏连接（由 MainWindow 建立）

| 源对象 | 源信号 | 目标槽 | 说明 |
|--------|--------|--------|------|
| 菜单“保存仓库” | `triggered()` | `onSaveWarehouse()` | 调用 `warehouse.saveData()`，刷新状态栏 |
| 菜单“切换仓库” | `triggered()` | `onSwitchWarehouse()` | 调用 `dashboardPage->switchWarehouseRequested()`（由 DashboardPage 转发） |
| 菜单“退出” | `triggered()` | `onExit()` | 检查脏状态，保存后退出 |

### 6. 状态栏更新规范

- 所有页面在执行完业务操作后，若需要向用户反馈结果，应发射 `statusMessageChanged(const QString& message, int timeout)` 信号。
- MainWindow 接收到该信号后，在状态栏显示消息，超时后自动消失。
- **超时建议**：成功消息 2000ms，错误消息 5000ms。
- 状态栏右侧固定显示：当前仓库文件名、商品总数、总库存量，由 MainWindow 在页面切换时统一更新。

### 7. MainWindow 私有槽实现要点

| 槽 | 实现逻辑 |
|----|----------|
| `onWarehouseSelected(path)` | `warehouse.loadData()` → 更新 `AppContext.currentWarehouse` → `currentCategory = nullopt` → 切换到 `ManagementPage` → `managementPage->loadCategory("")` |
| `onCategoryClicked(name)` | 更新 `AppContext.currentCategory = name` → 切换到 `ManagementPage` → `managementPage->loadCategory(name)` |
| `onSwitchWarehouse()` | 若 `warehouse.isDirty()` 为 true，弹出保存确认框 → 用户取消则返回 → 切换到 `WelcomePage` |
| `switchToDashboard()` | 若 `managementPage->checkAndSaveEdit()` 返回“取消”则返回 → 切换到 `DashboardPage` |
| `updateStatusBar(msg, timeout)` | 在状态栏显示消息，超时后自动清除 |
| `onSettingsChanged()` | 调用 `dashboardPage->refreshDashboard()`（预警阈值变化） |
| `onSaveWarehouse()` | 调用 `warehouse.saveData()`，更新状态栏 |
| `onExit()` | 检查 `warehouse.isDirty()` → 若有未保存则弹出确认框 → 保存后退出 |

---

## 十四、草图（ASCII）
**WelcomePage**

``` text
+------------------------------------------+
| [ HBoxLayout ]                           |
| Welcome!                                 |
+------------------------------------------+
| [新建仓库] [刷新列表] [重命名仓库]       |
+------------------------------------------+
| QListWidget                              |
| - warehouse1.txt                         |
| - warehouse2.txt                         |
| - ...                                    |
|                                          |
+------------------------------------------+
```

**DashboardPage**
``` text
+--------+----------------------------------------------------+
|快速销售  |  图表区 (GridLayout)                             |
| 编号: _  |  +------------+  +------------+                  |
| 数量: _  |  | 分类占比   |  | 价格分布   |                  |
| [确认]   |  | (饼图)     |  | (直方图)   |                  |
|--------  |  +------------+  +------------+                  |
|搜索      |  +------------+  +------------+                  |
| [___] [x]|  | 库存分布   |  | 生产商     |                  |
|[D]    [R]|  | (柱状图)   |  | Top10      |                  |
|   [+]    |  +------------+  +------------+                  |
|--------  |--------------------------------------------------|
|分类列表  |  预警区 (HBox)                                   |
| *电子(12)|  +----------+ +----------+ +----------+          |
| *办公(8) |  |库存预警  | |到货提醒  | |过期提醒  |          |
| *食品(5) |  |(List)    | | (List)   | | (List)   |          |
| ...      |  +----------+ +----------+ +----------+          |
+----------+--------------------------------------------------+
```

**ManagementPage**
``` text
+-----------------+----------------------------------------+---------+
|当前分类         |  QTableView / QListView                | 详情    |
| 电子            |  编号 | 名称 | 单价 | 厂商 | 库存 | ...| 编号:   |
|[返回]           |----------------------------------------| 名称:   |
|-----------------|  01001X| 苹果 | 5.00 | 果农 | 100 |    | 单价:   |
|搜索             |  010026| 香蕉 | 3.50 | 果农 | 80  |    | 厂商:   |
|[x]id  ：[___]   |  ...                                   | 库存:   |
|[x]名称：[___][x]|                                        | 到货:   |
|[x]来源：[___][x]|                                        | 保质期: |
|---------------- |                                        | [编辑]  |
|浏览条件         |                                        |---------|
|价格：           |                                        | 销售:   |
|[x][min]~[max]   |                                        | [5] [√]|
|[x]库存:         |                                        |---------|
|[x][min]~[max]   |                                        | [下架]  |
|[x]到货:         |                                        | [转移]  |
|[起始]~[结束]    |                                        |         |
|[x]保质期:       |                                        |         |
|[起始]~[结束]    |                                        |         |
|[应用] [重置]    |                                        |         |
+-----------------+----------------------------------------+---------+
```

---

  ## 十五、未来扩展

- **无效商品修复对话框**：当前版本仅显示加载时的错误警告，暂不提供修复功能。后续可设计一个独立的 `FixInvalidDialog`，通过 `browseInvalid()` 获取所有无效商品，复用 `updateGoods` 逻辑进行修复。此功能待后续迭代实现。
- **高级搜索对话框**：当前版本后端可以通过S表达式进行复杂搜索，但尚未在前端进行对齐。
- **预警区商品跳转**：当前版本仅供浏览，但后期可以加入跳转代码。
- **线程安全**：当前版本全部工作都是单一线程内完成的，后期需要考虑加载数据等问题的多线程安全问题。不过此部分后端还未完善。