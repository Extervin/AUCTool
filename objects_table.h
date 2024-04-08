#ifndef OBJECTSTABLE_H
#define OBJECTSTABLE_H

#include <QAbstractTableModel>
#include <QMap>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQueryModel>
#include <QColor>

class ObjectsTable : public QAbstractTableModel
{
    Q_OBJECT

public slots:
    void updatePingFlag(const QString &ipAddress, bool pingSuccess);

public:
    explicit ObjectsTable(QObject *parent = nullptr);

    void setMarkSetManualState(bool state);

    // Переопределяем функции из QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QMap<QString, QString> getSelectedIPs() const;

    void updateRowColor(const QString &ipAddress, int finishCode);
    QColor getRowColor(int resultCode);

    // Функция для установки SQL-запроса
    QString getQuery() const;
    void setQuery(const QString &query);
    void addIPAddress(const QString &ipAddress);


    QMap<QString, bool> m_pingFlags;
    QMap<QString, QColor> m_pingColors;

private:
    QSqlQueryModel *m_queryModel;
    QString m_currentQuery;
    QString m_lastUpdatedIpAddress;
    int m_lastUpdateResult;
    QMap<int, Qt::CheckState> m_checkStates; // Переменная для хранения состояний чекбоксов
    bool m_markSetManualState = false;
    QMap<int, QColor> m_rowColors;

};

#endif // OBJECTSTABLE_H
