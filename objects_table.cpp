#include "objects_table.h"
#include <QSqlError>
#include <QDebug>

ObjectsTable::ObjectsTable(QObject *parent) : QAbstractTableModel(parent), m_queryModel(new QSqlQueryModel(this))
{
    // Инициализация модели данных
    m_currentQuery = "";
}

QString ObjectsTable::getQuery() const {
    // Возвращаем текущий запрос
    return m_currentQuery;
}

int ObjectsTable::rowCount(const QModelIndex &parent) const
{
    // Возвращаем количество строк
    return m_queryModel->rowCount(parent);
}

int ObjectsTable::columnCount(const QModelIndex &parent) const
{
    // Возвращает количество столбцов из результирующего набора запроса
    return m_queryModel->columnCount(parent);
}

QVariant ObjectsTable::data(const QModelIndex &index, int role) const
{
    // Возвращает данные для конкретной ячейки
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return m_queryModel->data(index);
    }

    return QVariant();
}

QVariant ObjectsTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    // Возвращает данные для заголовка
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return m_queryModel->headerData(section, orientation, role);
    } else {
        // Возвращает номер строки для вертикальных заголовков (если необходимо)
        return section + 1;
    }
}

void ObjectsTable::setQuery(const QString &query)
{
    m_currentQuery = query;
    // Установка SQL-запроса для модели данных
    m_queryModel->setQuery(query);

    if (m_queryModel->lastError().isValid()) {
        qDebug() << "Error executing query:" << m_queryModel->lastError().text();
        return;
    }
    // Обновление количества строк
    beginResetModel();
    endResetModel();
}
