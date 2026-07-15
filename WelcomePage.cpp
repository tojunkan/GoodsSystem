// WelcomePage.cpp
#include "WelcomePage.h"
#include "BackendAPI.h"
#include <QMessageBox>
#include <QInputDialog>
#include <filesystem>
#include <QDebug>
#include <vector>
#include <string>

const std::string WAREHOUSE_DIR = "./warehouses";

WelcomePage::WelcomePage(QWidget* parent)
    : QWidget(parent), m_selectedFile("")
{
    // ---------- 1. 创建控件 ----------
    m_welcomeLabel = new QLabel("Welcome!", this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);

    m_fileList = new QListWidget(this);
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);

    m_newBtn = new QPushButton("新建仓库", this);
    m_refreshBtn = new QPushButton("刷新列表", this);
    m_deleteBtn = new QPushButton("删除仓库", this);
    m_renameBtn = new QPushButton("重命名仓库", this);
    m_newBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_refreshBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_deleteBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_renameBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    qDebug()<<"Button Width before loading:"<<m_newBtn->width()<<"\n";

    // **创建输入框（之前漏掉了）**
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText("输入仓库名称...");
    m_inputEdit->setVisible(false);
    m_inputEdit->setFixedWidth(0);
    m_inputEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // ---------- 2. 创建布局 ----------
    m_buttonLayout = new QHBoxLayout;
    // **把输入框放在最前面**
    m_buttonLayout->insertWidget(0, m_inputEdit);
    m_buttonLayout->addWidget(m_newBtn);
    m_buttonLayout->addWidget(m_refreshBtn);
    m_buttonLayout->addWidget(m_deleteBtn);
    m_buttonLayout->addWidget(m_renameBtn);
    qDebug() << "Button Width after added to the layout:" << m_newBtn->width() << "\n";
    qDebug() << "layout Width before loading:" << m_buttonLayout->geometry() << "\n";

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_welcomeLabel);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_fileList);
    setLayout(m_mainLayout);

    // ---------- 3. 连接信号与槽 ----------
    // 每个按钮只连接一次
    connect(m_newBtn, &QPushButton::clicked, this, &WelcomePage::onNewWarehouseClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &WelcomePage::onRefreshClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &WelcomePage::onDeleteClicked);
    connect(m_renameBtn, &QPushButton::clicked, this, &WelcomePage::onRenameClicked);

    connect(m_fileList, &QListWidget::itemClicked, this, &WelcomePage::onFileItemClicked);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &WelcomePage::onFileItemDoubleClicked);

    // 输入框回车确认
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &WelcomePage::onInputReturnPressed);

    // ---------- 4. 启动时刷新文件列表 ----------
    refreshFileList(true);    
}

WelcomePage::~WelcomePage()
{
    // 无需手动 delete
}

// ---------- 槽函数 ----------
void WelcomePage::onNewWarehouseClicked()
{
    if (m_currentMode == NEW) {
        confirmInput();
    }
    else {
        setMode(NEW);
    }
}

void WelcomePage::onRefreshClicked()
{
    refreshFileList();
}

void WelcomePage::onDeleteClicked()
{
    if (m_selectedFile.isEmpty()) return;

    QMessageBox::StandardButton rb = QMessageBox::question(
        this,
        "提示",
        "确认要删除 " + m_selectedFile + " 吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );
    if (rb == QMessageBox::Yes) {
        std::string path = WAREHOUSE_DIR + "/" + m_selectedFile.toStdString();
        std::string err;
        if (!apiDeleteWarehouseFile(path, err)) {
            QMessageBox::critical(this, "错误", QString::fromStdString(err));
        }
        else {
            m_selectedFile.clear();
        }
    }
    refreshFileList();
}

void WelcomePage::onRenameClicked()
{
    if (m_currentMode == RENAME) {
        confirmInput();
    }
    else {
        setMode(RENAME);
    }
}

void WelcomePage::onFileItemClicked(QListWidgetItem* item)
{
    if (item) {
        if (m_selectedFile != item->text()) {
            m_selectedFile = item->text();
        }
        else {
            m_selectedFile.clear();
            m_fileList->clearSelection();
        }
    }
    else {
        m_selectedFile.clear();
    }
}

void WelcomePage::onFileItemDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        QString path = QString::fromStdString(WAREHOUSE_DIR)+ item->text();
        emit warehouseSelected(path);
    }
}

void WelcomePage::refreshFileList(bool init)
{
    m_fileList->clear();

    std::vector<std::string> files = apiScanWarehouseFiles(WAREHOUSE_DIR);

    if (files.empty() && init) {
        QMessageBox::StandardButton rb = QMessageBox::question(
            this,
            "警告",
            "目录下没有仓库文件，要为您创建默认的default文件吗？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        if (rb == QMessageBox::Yes) {
            if (!apiCreateWarehouseFile(WAREHOUSE_DIR + "/default.txt")) {
                QMessageBox::critical(this, "错误", "创建失败，请检查权限或其他问题！");
            }
            files = apiScanWarehouseFiles(WAREHOUSE_DIR);
        }
    }

    QStringList items;
    for (const auto& f : files) {
        items << QString::fromStdString(f);
    }
    m_fileList->addItems(items);

    // 恢复之前选中的高亮（如果有）
    if (!m_selectedFile.isEmpty()) {
        QList<QListWidgetItem*> found = m_fileList->findItems(m_selectedFile, Qt::MatchExactly);
        if (!found.isEmpty()) {
            m_fileList->setCurrentItem(found.first());
        }
        else {
            m_selectedFile.clear();
        }
    }
}


// ---------- 模式管理 ----------
void WelcomePage::setMode(Mode mode)
{
    // 如果当前已经在该模式下，不做任何事（比如连续点击新建）
    if (m_currentMode == mode && mode != DEFAULT) {
        return;
    }

    // 如果当前不是默认模式，先取消（恢复默认布局）
    if (m_currentMode != DEFAULT) {
        // 注意：cancelInput 会置 m_currentMode = DEFAULT，所以之后要重新设置
        cancelInput();
    }

    if (mode == DEFAULT) {
        // 已经由 cancelInput 恢复了，直接返回
        return;
    }

    m_currentMode = mode;

    // 隐藏其他按钮
    qDebug() << "Button Geometry before changing:" << m_newBtn->geometry() << "\n";
    qDebug() << "layout Geometry before changing:" << m_buttonLayout->geometry() << "\n";
    m_newBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_refreshBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_deleteBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_renameBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_refreshBtn->setVisible(false);
    m_deleteBtn->setVisible(false);
    qDebug() << "Button Geometry before changing:" << m_newBtn->geometry() << "\n";
    qDebug() << "layout Geometry before changing:" << m_buttonLayout->geometry() << "\n";

    if (mode == NEW) {
        m_actionBtn = m_newBtn;
        m_newBtn->setText("确认");
        m_renameBtn->setVisible(false);
        m_inputEdit->setPlaceholderText("输入新仓库名称 (不含 .txt)");
        m_inputEdit->clear();
    }
    else { // RENAME
        if (m_selectedFile.isEmpty()) {
            QMessageBox::warning(this, "提示", "请先选中一个仓库文件");
            setMode(DEFAULT);
            return;
        }
        m_actionBtn = m_renameBtn;
        m_renameBtn->setText("确认");
        m_newBtn->setVisible(false);
        QString baseName = m_selectedFile;
        if (baseName.endsWith(".txt", Qt::CaseInsensitive))
            baseName.chop(4);
        m_inputEdit->setText(baseName);
        m_inputEdit->setPlaceholderText("输入新名称");
    }

    // 显示输入框并聚焦
    m_inputEdit->setVisible(true);
    m_inputEdit->setFocus();
    m_inputEdit->selectAll();

    expandInput();
}

void WelcomePage::cancelInput()
{
    // 恢复默认布局
    m_newBtn->setText("新建仓库");
    m_renameBtn->setText("重命名仓库");
    m_refreshBtn->setVisible(true);
    m_deleteBtn->setVisible(true);
    m_newBtn->setVisible(true);
    m_renameBtn->setVisible(true);
    m_newBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_refreshBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_deleteBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_renameBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_inputEdit->setVisible(false);
    m_inputEdit->setFixedWidth(0);
    m_inputEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_actionBtn = nullptr;
    m_currentMode = DEFAULT;
    if (m_widthAnim) m_widthAnim->stop();
}

void WelcomePage::expandInput()
{
    if (!m_widthAnim) {
        m_widthAnim = new QPropertyAnimation(m_inputEdit, "minimumWidth", this);
        m_widthAnim->setDuration(200);
        m_widthAnim->setEasingCurve(QEasingCurve::InOutQuad);
    }

    // 计算目标宽度
    int totalWidth = m_buttonLayout->geometry().width();
    if (totalWidth <= 0) {
        totalWidth = this->width() - 50; // 估算
    }
    int btnWidth = m_actionBtn ? m_actionBtn->sizeHint().width() + 20 : 80;
    int spacing = m_buttonLayout->spacing() * 2;
    int targetWidth = qMax(0, totalWidth - btnWidth - spacing);

    m_inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_widthAnim->stop();
    m_widthAnim->setStartValue(0);
    m_widthAnim->setEndValue(targetWidth);
    m_widthAnim->start();
}

void WelcomePage::confirmInput()
{
    if (!m_actionBtn) return;

    QString name = m_inputEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "名称不能为空");
        return;
    }

    if (m_currentMode == NEW) {
        std::string path = WAREHOUSE_DIR + "/" + name.toStdString() + ".txt";
        if (!apiCreateWarehouseFile(path)) {
            QMessageBox::critical(this, "错误", "创建失败，可能已存在同名文件");
            return;
        }
        refreshFileList();
        m_selectedFile = name + ".txt";
        QList<QListWidgetItem*> found = m_fileList->findItems(m_selectedFile, Qt::MatchExactly);
        if (!found.isEmpty()) m_fileList->setCurrentItem(found.first());
    }
    else { // RENAME
        if (m_selectedFile.isEmpty()) {
            setMode(DEFAULT);
            return;
        }
        std::string oldPath = WAREHOUSE_DIR + "/" + m_selectedFile.toStdString();
        std::string newPath = WAREHOUSE_DIR + "/" + name.toStdString() + ".txt";
        std::string error;
        if (!apiRenameWarehouseFile(oldPath, newPath, error)) {
            QMessageBox::critical(this, "错误", QString::fromStdString(error));
            return;
        }
        refreshFileList();
        m_selectedFile = name + ".txt";
        QList<QListWidgetItem*> found = m_fileList->findItems(m_selectedFile, Qt::MatchExactly);
        if (!found.isEmpty()) m_fileList->setCurrentItem(found.first());
    }

    // 完成，回到默认模式
    setMode(DEFAULT);
}

void WelcomePage::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_currentMode != DEFAULT) {
        cancelInput();
        return;
    }
    QWidget::keyPressEvent(event);
}

void WelcomePage::onInputReturnPressed()
{
    confirmInput();
}