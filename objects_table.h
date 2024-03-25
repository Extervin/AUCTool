#ifndef OBJECTS_TABLE_H
#define OBJECTS_TABLE_H

#include <QAbstractTableModel>
#include <QSqlQueryModel>

class ObjectsTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ObjectsTable(QObject *parent = nullptr);

    // Метод для получения текущего запроса
    QString getQuery() const;

    // Переопределенные методы для модели таблицы
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Метод для установки SQL-запроса
    void setQuery(const QString &query);

private:
    QSqlQueryModel *m_queryModel; // Модель данных для выполнения SQL-запроса
    QString m_currentQuery;
};

#endif // OBJECTS_TABLE_H
