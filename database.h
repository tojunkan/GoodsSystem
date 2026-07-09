#ifndef DATABASE_H
#define DATABASE_H

#include <QtWidgets>
#include <QtSql>

bool initDatabase();
bool execSql(const QString &sql, const QVariantList &args = QVariantList(), QString *err = nullptr);
QString money(double v);
void showError(QWidget *parent, const QString &msg);
QSqlTableModel* makeModel(QObject *parent, const QString &table, const QString &filter = QString());
QString sqlLikeEscape(QString s);
bool columnExists(const QString &table, const QString &column);
void setEqualColumnWidths(QTableView *view, int width = 120);

#endif
