#include "objects_table.h"
#include <QSqlError>
#include <QBrush>
#include <QDebug>
#include <QPainter>
#include <QIcon>

ObjectsTable::ObjectsTable(QObject *parent) : QAbstractTableModel(parent), m_queryModel(new QSqlQueryModel(this))
{
    m_currentQuery = "";
}

void ObjectsTable::setMarkSetManualState(bool state)
{
    if (m_markSetManualState != state) {
        m_markSetManualState = state;
        beginResetModel();
        endResetModel();
    }
}


void ObjectsTable::addIPAddress(const QString &ipAddress) {
    m_pingFlags[ipAddress] = false;
}

void ObjectsTable::updatePingFlag(const QString &ipAddress, bool pingSuccess)
{
    qDebug() << "Зашли в функцию обновления кружочков";
    // Проверяем, существует ли запись для данного IP-адреса в QMap
    if (!m_pingFlags.contains(ipAddress)) {
        qDebug() << "IP address" << ipAddress << "not found in the table.";
        return;
    }

    // Обновляем флаг пинга для соответствующего IP-адреса
    m_pingFlags[ipAddress] = pingSuccess;

    qDebug() << "Обновили флаг";

    // Находим индекс строки, соответствующей IP-адресу
    int row = 0;
    for (auto it = m_pingFlags.constBegin(); it != m_pingFlags.constEnd(); ++it) {
        if (it.key() == ipAddress) {
            break;
        }
        ++row;
    }
    qDebug() << "Нашли индекс строки";
    // Получаем индексы ячейки для обновления данных
    QModelIndex topLeft = index(row, 0); // Пинг хранится в первой колонке
    QModelIndex bottomRight = index(row, 0);

    QColor color = pingSuccess ? QColor(76, 175, 80) : QColor(221, 0, 0);
    m_pingColors[ipAddress] = color;

    // Оповещаем вид об изменении данных
    emit dataChanged(topLeft, bottomRight);
}


QString ObjectsTable::getQuery() const {
    return m_currentQuery;
}

int ObjectsTable::rowCount(const QModelIndex &parent) const
{
    return m_queryModel->rowCount(parent);
}

int ObjectsTable::columnCount(const QModelIndex &parent) const
{
    return m_queryModel->columnCount(parent) + 1; // Добавляем одну дополнительную колонку
}

QVariant ObjectsTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        // Если роль - отображение данных
        if (index.column() == 0) {
            // Для первой колонки возвращаем пустые значения
            return QVariant();
        } else {
            // Возвращаем данные из результирующего набора запроса
            return m_queryModel->data(this->index(index.row(), index.column() - 1));
        }
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        // Если роль - декорация и это первая колонка
        QString ipAddress = m_queryModel->data(this->index(index.row(), 2)).toString(); // Получаем IP-адрес объекта
        QColor color = m_pingColors.value(ipAddress, QColor(221, 0, 0)); // Получаем цвет кружочка для данного IP-адреса

        int iconSize = 20; // Размер кружочка
        int radius = 8; // Радиус кружка

        QPixmap pixmap(iconSize, iconSize);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(color);
        painter.setPen(Qt::NoPen); // Убираем обводку

        // Нарисовать кружок
        painter.drawEllipse((iconSize - 2 * radius) / 2, (iconSize - 2 * radius) / 2, 2 * radius, 2 * radius);

        return QIcon(pixmap);
    } else if (role == Qt::CheckStateRole && index.column() == 1) {
        // Если роль - состояние флажка и это вторая колонка
        // Проверяем значение переменной m_markSetManualState
        if (m_markSetManualState) {
            // Возвращаем состояние флажка, если режим установлен вручную
            return m_checkStates.value(index.row(), Qt::Unchecked);
        } else {
            // Возвращаем пустое значение, чтобы скрыть чекбоксы, если режим не установлен вручную
            return QVariant();
        }
    }
    return QVariant();
}


QVariant ObjectsTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section == 0) {
            // Заголовок для первой колонки
            return QVariant();
        } else {
            // Заголовки для остальных колонок берем из результирующего набора запроса
            return m_queryModel->headerData(section - 1, orientation, role);
        }
    } else {
        // Возвращаем номер строки для вертикальных заголовков (если необходимо)
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

Qt::ItemFlags ObjectsTable::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == 1) {
        if (m_markSetManualState == true) {
            flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
        } else {
            flags &= ~(Qt::ItemIsUserCheckable);
        }
    }
    return flags;
}

bool ObjectsTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 1) {
        // Устанавливаем состояние флажка для второй колонки
        m_checkStates[index.row()] = (value == Qt::Checked ? Qt::Checked : Qt::Unchecked);
        // Уведомляем о изменении данных
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

