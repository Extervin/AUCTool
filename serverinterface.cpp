#include "serverinterface.h"
#include "ui_serverinterface.h"
#include <QtSql>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QDebug>
#include <QWidgetAction>
#include <QPainter>
#include <QBitmap>

ServerInterface::ServerInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerInterface)
{
    ui->setupUi(this);
    spawnTable();
    spawnMenu();

}

void ServerInterface::onSwitchToggled(bool checked) {
    Q_UNUSED(checked);
        // Ваш код для обработки изменения состояния переключателя
}

void ServerInterface::spawnMenu() {
    // Создаем выпадающее меню для кнопки
    QMenu *filterMenu = new QMenu(this);

    // Создаем переключатель как элемент меню
    SwitchButton *switchButton = new SwitchButton(this);

    // Подключаем сигнал toggled к слоту для изменения цвета фона
    connect(switchButton, &SwitchButton::toggled, this, &ServerInterface::onSwitchToggled);

    // Добавляем переключатель в меню
    QWidgetAction *switchWidgetAction = new QWidgetAction(this);
    switchWidgetAction->setDefaultWidget(switchButton);
    filterMenu->addAction(switchWidgetAction);

    // Добавляем фильтр для городов
    QLabel *titleLabel1 = new QLabel("Города:", this);
    titleLabel1->setEnabled(false);
    QWidgetAction *titleWidgetAction1 = new QWidgetAction(this);
    titleWidgetAction1->setDefaultWidget(titleLabel1);
    titleLabel1->setStyleSheet("QLabel { font-weight: 600; padding-right: 141px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction1);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для городов и добавляем их в меню
    QAction *filterCityAction1 = new QAction("Варна", this);
    QAction *filterCityAction2 = new QAction("Пловдив", this);
    QAction *filterCityAction3 = new QAction("София", this);
    filterMenu->addAction(filterCityAction1);
    filterMenu->addAction(filterCityAction2);
    filterMenu->addAction(filterCityAction3);

    // Добавляем фильтр для статуса
    QLabel *titleLabel2 = new QLabel("Статус:", this);
    titleLabel2->setEnabled(false);
    QWidgetAction *titleWidgetAction2 = new QWidgetAction(this);
    titleWidgetAction2->setDefaultWidget(titleLabel2);
    titleLabel2->setStyleSheet("QLabel { font-weight: 600; padding-right: 141px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction2);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для статуса и добавляем их в меню
    QAction *filterStatusAction1 = new QAction("Доступные", this);
    QAction *filterStatusAction2 = new QAction("Обновлённые", this);
    QAction *filterStatusAction3 = new QAction("Архивированные", this);
    QAction *filterStatusAction4 = new QAction("Ошибки", this);
    filterMenu->addAction(filterStatusAction1);
    filterMenu->addAction(filterStatusAction2);
    filterMenu->addAction(filterStatusAction3);
    filterMenu->addAction(filterStatusAction4);

    // Добавляем фильтр для тэгов
    QLabel *titleLabel3 = new QLabel("Тэги:", this);
    titleLabel3->setEnabled(false);
    QWidgetAction *titleWidgetAction3 = new QWidgetAction(this);
    titleWidgetAction3->setDefaultWidget(titleLabel3);
    titleLabel3->setStyleSheet("QLabel { font-weight: 600; padding-right: 141px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction3);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для тэгов и добавляем их в меню
    QAction *filterTagAction1 = new QAction("Клик", this);
    QAction *filterTagAction2 = new QAction("Пункт", this);
    QAction *filterTagAction3 = new QAction("Аптека", this);
    QAction *filterTagAction4 = new QAction("Новый", this);
    QAction *filterTagAction5 = new QAction("Закрытый", this);
    filterMenu->addAction(filterTagAction1);
    filterMenu->addAction(filterTagAction2);
    filterMenu->addAction(filterTagAction3);
    filterMenu->addAction(filterTagAction4);
    filterMenu->addAction(filterTagAction5);

    // Устанавливаем выпадающее меню для кнопки и устанавливаем флаг ToolButtonPopupMode
    ui->filterButton->setMenu(filterMenu);
    ui->filterButton->setPopupMode(QToolButton::InstantPopup);

    // Добавляем стили к меню
    QString styleSheet = "QMenu { border: 2px solid #959595; border-radius: 5px; background-color: white;}"
                         "QMenu::item { padding: 0; color: 959595; padding: 7px 10px; }"
                         "QMenu::item:selected { background-color: #deddde; }"
                         "QMenu::separator { height: 0px; border: 1px solid #c0c0c0; margin: 0 5px 5px 5px; }";

    filterMenu->setStyleSheet(filterMenu->styleSheet() + styleSheet);

    // Добавляем соединения для фильтров
    connect(filterCityAction1, &QAction::triggered, [=]() {
        applyCityFilter("Варна");
    });

    connect(filterCityAction2, &QAction::triggered, [=]() {
        applyCityFilter("Пловдив");
    });

    connect(filterCityAction3, &QAction::triggered, [=]() {
        applyCityFilter("София");
    });

    connect(filterStatusAction1, &QAction::triggered, [=]() {
        applyStatusFilter("Доступные");
    });

    connect(filterStatusAction2, &QAction::triggered, [=]() {
        applyStatusFilter("Обновлённые");
    });

    connect(filterStatusAction3, &QAction::triggered, [=]() {
        applyStatusFilter("Архивированные");
    });

    connect(filterStatusAction4, &QAction::triggered, [=]() {
        applyStatusFilter("Ошибки");
    });

    connect(filterTagAction1, &QAction::triggered, [=]() {
        applyTagFilter("Клик");
    });

    connect(filterTagAction2, &QAction::triggered, [=]() {
        applyTagFilter("Пункт");
    });

    connect(filterTagAction3, &QAction::triggered, [=]() {
        applyTagFilter("Аптека");
    });

    connect(filterTagAction4, &QAction::triggered, [=]() {
        applyTagFilter("Новый");
    });

    connect(filterTagAction5, &QAction::triggered, [=]() {
        applyTagFilter("Закрытый");
    });
}

void ServerInterface::cancelFilter() {
    QObject *senderObj = sender();
    if (!senderObj)
        return;

    // Получаем родительский виджет кнопки, который содержит фильтр
    QWidget *filterWidget = qobject_cast<QWidget*>(senderObj->parent());
    if (!filterWidget)
        return;

    // Удаляем фильтр и его дочерние виджеты из контейнера
    QWidget *appliedFiltersContainer = ui->appliedFilters;
    appliedFiltersContainer->layout()->removeWidget(filterWidget);
    delete filterWidget;

    // Очищаем фильтр в модели данных
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui->tableView->model());
    if (!model) {
        qDebug() << "Модель данных не найдена!";
        return;
    }

    // Получаем текущий SQL-запрос
    QString currentQuery = model->query().executedQuery();

    // Очищаем фильтр из SQL-запроса
    QString newQuery = currentQuery.section(" WHERE ", 0, 0); // Удаляем часть после "WHERE"

    // Устанавливаем новый SQL-запрос в модель
    model->setQuery(newQuery);

    // Обновляем вид
    ui->tableView->resizeColumnsToContents();
}

void ServerInterface::spawnTable() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("dev");
    db.setUserName("root");
    db.setPassword("");

    if (!db.open()) {
        qDebug() << "Ошибка при подключении к базе данных:" << db.lastError().text();
        return;
    }

    // Выполняем запрос на получение только нужных столбцов из базы данных
    QSqlQuery query;
    if (!query.exec("SELECT IP, IT, Obekt, a1, a2 FROM acc_1906")) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

    // Создаем модель данных QSqlQueryModel и устанавливаем результат запроса в нее
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(query);

    // Устанавливаем модель в QTableView
    ui->tableView->setModel(model);

    // Растягиваем столбцы по содержимому
    ui->tableView->resizeColumnsToContents();
}


ServerInterface::~ServerInterface()
{
    delete ui;
}

void ServerInterface::applyCityFilter(const QString &city) {
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui->tableView->model());
    if (!model) {
        qDebug() << "Модель данных не найдена!";
        return;
    }

    // Получаем текущий SQL-запрос
    QString currentQuery = model->query().executedQuery();

    // Добавляем фильтр по городу в SQL-запрос
    QString newQuery = currentQuery + (currentQuery.contains("WHERE") ? " AND" : " WHERE") + " IT = '" + city + "'";

    // Устанавливаем новый SQL-запрос в модель
    model->setQuery(newQuery);

    // Обновляем вид
    ui->tableView->resizeColumnsToContents();

    // Добавляем примененный фильтр в контейнер с примененными фильтрами
    QWidget *appliedFiltersContainer = ui->appliedFilters;
    if (!appliedFiltersContainer)
        return;

    // Создаем новый виджет для фильтра
    QWidget *filterWidget = new QWidget(appliedFiltersContainer);
    QString filterObjectName = "filter" + QString::number(cityFilterIndex);
    filterWidget->setObjectName(filterObjectName);

    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(5);
    filterLayout->setContentsMargins(9, 6, 9, 6);

    QLabel *filterLabel = new QLabel(city, filterWidget);
    filterLayout->addWidget(filterLabel);

    QPushButton *cancelButton = new QPushButton(filterWidget);
    QString cancelObjectName = "cancel" + filterObjectName;
    cancelButton->setObjectName(cancelObjectName);
    cancelButton->setIcon(QIcon(":/buttons/icon/x-666666-color.svg"));
    cancelButton->setIconSize(QSize(10, 10));
    cancelButton->setFixedSize(20, 20);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &ServerInterface::cancelFilter);
    filterLayout->addWidget(cancelButton);

    QHBoxLayout *appliedFiltersLayout = qobject_cast<QHBoxLayout*>(appliedFiltersContainer->layout());
    if (appliedFiltersLayout) {
        appliedFiltersLayout->addWidget(filterWidget);
        appliedFiltersLayout->setAlignment(filterWidget, Qt::AlignLeft);
    } else {
        qDebug() << "Layout for applied filters container not found!";
    }

    cityFilterIndex++;
}

void ServerInterface::applyStatusFilter(const QString &status) {
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui->tableView->model());
    if (!model) {
        qDebug() << "Модель данных не найдена!";
        return;
    }

    // Получаем текущий SQL-запрос
    QString currentQuery = model->query().executedQuery();

    // Добавляем фильтр по статусу в SQL-запрос
    QString newQuery = currentQuery;
    if (status == "Доступные") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" a2 = ''");
    } else if (status == "Обновлённые") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" a1 = 'yes'");
    } else if (status == "Архивированные") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" a2 = 'yes'");
    } else if (status == "Ошибки") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" mode = 'lg'");
    }

    // Устанавливаем новый SQL-запрос в модель
    model->setQuery(newQuery);

    // Обновляем вид
    ui->tableView->resizeColumnsToContents();

    // Добавляем примененный фильтр в контейнер с примененными фильтрами
    QWidget *appliedFiltersContainer = ui->appliedFilters;
    if (!appliedFiltersContainer)
        return;

    // Создаем новый виджет для фильтра
    QWidget *filterWidget = new QWidget(appliedFiltersContainer);
    QString filterObjectName = "filter" + QString::number(statusFilterIndex);
    filterWidget->setObjectName(filterObjectName);

    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(5);
    filterLayout->setContentsMargins(9, 6, 9, 6);

    QLabel *filterLabel = new QLabel(status, filterWidget);
    filterLayout->addWidget(filterLabel);

    QPushButton *cancelButton = new QPushButton(filterWidget);
    QString cancelObjectName = "cancel" + filterObjectName;
    cancelButton->setObjectName(cancelObjectName);
    cancelButton->setIcon(QIcon(":/buttons/icon/x-666666-color.svg"));
    cancelButton->setIconSize(QSize(10, 10));
    cancelButton->setFixedSize(20, 20);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &ServerInterface::cancelFilter);
    filterLayout->addWidget(cancelButton);

    QHBoxLayout *appliedFiltersLayout = qobject_cast<QHBoxLayout*>(appliedFiltersContainer->layout());
    if (appliedFiltersLayout) {
        appliedFiltersLayout->addWidget(filterWidget);
        appliedFiltersLayout->setAlignment(filterWidget, Qt::AlignLeft);
    } else {
        qDebug() << "Layout for applied filters container not found!";
    }

    statusFilterIndex++;
}


void ServerInterface::applyTagFilter(const QString &tag) {
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui->tableView->model());
    if (!model) {
        qDebug() << "Модель данных не найдена!";
        return;
    }

    // Получаем текущий SQL-запрос
    QString currentQuery = model->query().executedQuery();

    // Добавляем фильтр по тегу в SQL-запрос
    QString newQuery = currentQuery;
    if (tag == "Клик") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" mode LIKE '%c%'");
    } else if (tag == "Пункт") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" mode LIKE '%p%'");
    } else if (tag == "Аптека") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" Nkod LIKE 'a%'");
    } else if (tag == "Новый") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" a1 = 'yes'");
    } else if (tag == "Закрытый") {
        newQuery += (currentQuery.contains("WHERE") ? " AND" : " WHERE") + QString(" a2 = 'yes'");
    }

    // Устанавливаем новый SQL-запрос в модель
    model->setQuery(newQuery);

    // Обновляем вид
    ui->tableView->resizeColumnsToContents();

    // Добавляем примененный фильтр в контейнер с примененными фильтрами
    QWidget *appliedFiltersContainer = ui->appliedFilters;
    if (!appliedFiltersContainer)
        return;

    // Создаем новый виджет для фильтра
    QWidget *filterWidget = new QWidget(appliedFiltersContainer);
    QString filterObjectName = "filter" + QString::number(tagFilterIndex);
    filterWidget->setObjectName(filterObjectName);

    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(5);
    filterLayout->setContentsMargins(9, 6, 9, 6);

    QLabel *filterLabel = new QLabel(tag, filterWidget);
    filterLayout->addWidget(filterLabel);

    QPushButton *cancelButton = new QPushButton(filterWidget);
    QString cancelObjectName = "cancel" + filterObjectName;
    cancelButton->setObjectName(cancelObjectName);
    cancelButton->setIcon(QIcon(":/buttons/icon/x-666666-color.svg"));
    cancelButton->setIconSize(QSize(10, 10));
    cancelButton->setFixedSize(20, 20);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &ServerInterface::cancelFilter);
    filterLayout->addWidget(cancelButton);

    QHBoxLayout *appliedFiltersLayout = qobject_cast<QHBoxLayout*>(appliedFiltersContainer->layout());
    if (appliedFiltersLayout) {
        appliedFiltersLayout->addWidget(filterWidget);
        appliedFiltersLayout->setAlignment(filterWidget, Qt::AlignLeft);
    } else {
        qDebug() << "Layout for applied filters container not found!";
    }

    tagFilterIndex++;
}


