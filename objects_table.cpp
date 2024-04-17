#include "objects_table.h"
#include <QSqlError>
#include <QBrush>
#include <QDebug>
#include <QPainter>
#include <QIcon>
#include <QTableView>
#include <QPrintDialog>
#include "tableprinter.h"

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
    return m_queryModel->columnCount(parent); // Добавляем одну дополнительную колонку
}

void ObjectsTable::updateRowColor(const QString &ipAddress, int resultCode) {
    // Сохраняем цвет строки для данного IP-адреса
    m_rowColors[ipAddress] = getRowColor(resultCode);

    // Обновляем отображение данных
    QModelIndexList indexes;
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex ipIndex = index(row, 2);
        if (data(ipIndex, Qt::DisplayRole).toString() == ipAddress) {
            // Если нашли соответствующий IP-адрес, добавляем индексы ячеек для обновления
            indexes << index(row, 0) << index(row, columnCount() - 1);
        }
    }
    // Оповещаем вид о необходимости обновить отображение данных
    for (const QModelIndex& index : indexes) {
        emit dataChanged(index, index);
    }
}


QVariant ObjectsTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (role == Qt::BackgroundRole) {
        // Проверяем, есть ли сохраненный цвет для этой строки
        QString ipAddress = m_queryModel->data(this->index(index.row(), 2)).toString();
        if (m_rowColors.contains(ipAddress)) {
            return m_rowColors[ipAddress]; // Возвращаем сохраненный цвет
        }
    } else if (role == Qt::DisplayRole) {
        return m_queryModel->data(this->index(index.row(), index.column()));
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
    } else if (role == Qt::CheckStateRole && index.column() == 0) {
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
        // Заголовки для всех колонок берем из результирующего набора запроса
        return m_queryModel->headerData(section, orientation, role);
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
    if (index.column() == 0) {
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
    if (role == Qt::CheckStateRole && index.column() == 0) {
        // Устанавливаем состояние флажка для второй колонки
        m_checkStates[index.row()] = (value == Qt::Checked ? Qt::Checked : Qt::Unchecked);
        // Обновляем количество отмеченных элементов
        m_checkedCount = countCheckedItems();
        // Уведомляем о изменении данных
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QColor ObjectsTable::getRowColor(int resultCode) {
    QColor color;

    // Определение цвета в зависимости от кода результата
    switch(resultCode) {
    case 0:
        // Успешное обновление, зеленый цвет
        color = QColor(168, 228, 160); // Зеленый цвет
        break;
    case 1:
        // Ошибка при обновлении, красный цвет
        color = QColor(205, 92, 92); // Красный цвет
        break;
    case 2:
        // Частично успешное обновление, желтый цвет
        color = QColor(255, 219, 139); // Желтый цвет
        break;
    default:
        // По умолчанию, белый цвет
        color = QColor(255, 255, 255); // Белый цвет
        break;
    }

    return color;
}

QMap<QString, QString> ObjectsTable::getSelectedIPs() const {
    QMap<QString, QString> selectedIPsAndNamesMap;
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex ipIndex = this->index(row, 2); // Индекс колонки с айпи
        QString ipAddress = data(ipIndex, Qt::DisplayRole).toString();
        QModelIndex nameIndex = this->index(row, 0); // Индекс колонки с именем объекта
        QString objectName = data(nameIndex, Qt::DisplayRole).toString();

        // Если флаг установлен в false или чекбокс отмечен, добавляем IP-адрес и имя объекта в карту выбранных
        if (!m_markSetManualState || data(index(row, 0), Qt::CheckStateRole) == Qt::Checked) {
            selectedIPsAndNamesMap[ipAddress] = objectName;
        }
    }
    return selectedIPsAndNamesMap;
}

int ObjectsTable::countCheckedItems() const {
    int checkedCount = 0;
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex index = this->index(row, 0); // Индекс первой колонки (с чекбоксом)
        if (data(index, Qt::CheckStateRole) == Qt::Checked) {
            ++checkedCount;
        }
    }
    return checkedCount;
}

void ObjectsTable::printTable(QWidget *parentWidget)
{
    // Создаем диалог печати
    QPrintDialog printDialog(parentWidget);
    if (printDialog.exec() == QDialog::Accepted) {
        // Получаем указатель на принтер из диалога
        QPrinter *printer = printDialog.printer();

        // Создаем рисовальщика для принтера
        QPainter painter;
        if (!painter.begin(printer)) {
            qDebug() << "Ошибка: не удалось начать печать";
            return;
        }

        // Создаем экземпляр TablePrinter, передавая ему рисовальщика и принтер
        TablePrinter tablePrinter(&painter, printer);

        // Задаем растяжение столбцов. Здесь предполагается, что у нас есть информация о размере каждого столбца.
        QVector<int> columnStretch = QVector<int>() << 6 << 2 << 4 << 10; // Замени это на фактические размеры столбцов

        // Устанавливаем марджины на странице (отступы справа и слева)
        tablePrinter.setPageMargin(50, 50, 20, 20); // Например, 50 мм справа и слева, 20 мм сверху и снизу

        // Печатаем таблицу, используя модель запроса m_queryModel
        if (!tablePrinter.printTable(m_queryModel, columnStretch)) {
            qDebug() << "Ошибка при печати таблицы:" << tablePrinter.lastError();
        }

        // Завершаем рисование
        painter.end();
    }
}


