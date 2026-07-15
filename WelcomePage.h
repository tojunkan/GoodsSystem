// WelcomePage.h
#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget* parent = nullptr);
    ~WelcomePage();

    // 公有槽：刷新文件列表（会被按钮调用，也会在初始化时调用）
    void refreshFileList(bool init=false);

signals:
    // 设计文档要求的信号：双击列表项时发射，携带完整路径
    void warehouseSelected(const QString& path);

private slots:
    void onNewWarehouseClicked();
    void onRefreshClicked();
    void onDeleteClicked();
    void onRenameClicked();
    void onFileItemClicked(QListWidgetItem* item);
    void onFileItemDoubleClicked(QListWidgetItem* item);
    void onInputReturnPressed();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum Mode { DEFAULT, NEW, RENAME };

    void setMode(Mode mode);
    void cancelInput();
    void confirmInput();
    void expandInput();

    // UI 控件
    QLabel* m_welcomeLabel;
    QListWidget* m_fileList;
    QPushButton* m_newBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_renameBtn;
    QLineEdit* m_inputEdit;

    // 存储当前单击选中的文件名（不含路径但有文件扩展名，用于重命名）
    QString m_selectedFile;
    Mode m_currentMode = DEFAULT;
    QPushButton* m_actionBtn = nullptr;

    QPropertyAnimation* m_widthAnim = nullptr;

    // 布局（直接在构造函数里 new 并设置）
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
};