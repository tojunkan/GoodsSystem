#ifndef TOGGLETABLEVIEW_H
#define TOGGLETABLEVIEW_H

#include <QTableView>
#include <QMouseEvent>

class ToggleTableView : public QTableView
{
public:
    explicit ToggleTableView(QWidget *parent = nullptr) : QTableView(parent) {}

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        QModelIndex idx = indexAt(event->pos());

        if (!idx.isValid()) {
            clearSelection();
            setCurrentIndex(QModelIndex());
            QTableView::mousePressEvent(event);
            return;
        }

        if (selectionModel()) {
            const QModelIndexList rows = selectionModel()->selectedRows();
            for (const QModelIndex &r : rows) {
                if (r.row() == idx.row()) {
                    clearSelection();
                    setCurrentIndex(QModelIndex());
                    event->accept();
                    return;
                }
            }
        }

        QTableView::mousePressEvent(event);
    }
};

#endif
