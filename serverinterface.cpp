#include <confirmation.h>
#include <fileoperations.h>
#include <QDebug>
#include <QAction>
#include <QBitmap>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSql>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>
#include <QWidgetAction>
#include <QToolButton>
#include <QTcpSocket>
#include <QTimer>
#include "serverinterface.h"
#include "progressdialog.h"
#include "ui_serverinterface.h"

ServerInterface::ServerInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerInterface)
{
    ui->setupUi(this);

    spawnTable();
    spawnMenu();
    ui->searchLine->installEventFilter(this);


    model = qobject_cast<ObjectsTable*>(ui->tableView->model());
    if (model) {
        currentQuery = model->getQuery(); // Используем метод getQuery() из класса ObjectsTable
    } else {
        qDebug() << "Модель данных не найдена!";
    }
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint |
                              Qt::WindowMinimizeButtonHint |
                              Qt::WindowMaximizeButtonHint |
                              Qt::WindowCloseButtonHint);
}

void ServerInterface::recieveError(const QString& ipAddress, const QString& errorType, const QString& errorMessage) {
    QMap<QString, QString> ipMap = model->getSelectedIPs();
    // Проверяем, существует ли уже элемент для данного IP-адреса
    QTreeWidgetItem* existingItem = nullptr;
    for (int i = 0; i < ui->errorTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = ui->errorTreeWidget->topLevelItem(i);
        if (item->text(1) == ipAddress) {
            existingItem = item;
            break;
        }
    }

    // Если элемент для данного IP-адреса не найден, создаем новый элемент
    if (!existingItem) {
        existingItem = new QTreeWidgetItem(ui->errorTreeWidget);
        existingItem->setText(0, ipMap[ipAddress]);
        existingItem->setText(1, ipAddress);
    }

    // Создаем новый элемент для отображения описания ошибки и добавляем его в качестве дочернего элемента
    QTreeWidgetItem* errorItem = new QTreeWidgetItem(existingItem);
    errorItem->setText(0, errorType); // Тип ошибки
    errorItem->setText(1, errorMessage); // Сообщение об ошибке

    // Разворачиваем родительский элемент, чтобы увидеть описание ошибки
    existingItem->setExpanded(true);
    ui->errorTreeWidget->resizeColumnToContents(0);
    ui->errorTreeWidget->resizeColumnToContents(1);

}

void ServerInterface::openProgressWindow() {
    ProgressDialog *progress = new ProgressDialog(this); // установка текущего объекта в качестве родителя
    connect(this, &ServerInterface::sendDebugInfoInProgress, progress, &ProgressDialog::updateDebugInfo);
    connect(this, &ServerInterface::sendListSize, progress, &ProgressDialog::receiveListSize);
    connect(this, &ServerInterface::sendUpdateProgressBar, progress, &ProgressDialog::receiveUpdateProgressBar);
    progress->setAttribute(Qt::WA_DeleteOnClose); // для автоматического удаления окна после закрытия
    // Установка флага Qt::Window, чтобы сделать диалог не модальным
    progress->setWindowFlags(progress->windowFlags() | Qt::Window);
    progress->show();
}

void ServerInterface::receiveDebugInfo(const QString& info) {
    emit sendDebugInfoInProgress(info);
}

void ServerInterface::receiveData(const QString& newLogin, const QString& newPassword, const bool newFlag, const QString source) {
    qDebug() << newLogin << " " << newPassword << " " << newFlag << " " << source;
    updateServerFiles(newFlag, newLogin, newPassword, source);
}


void ServerInterface::on_serverUpdateButton_clicked()
{
    QMap<QString, QString> ipMap = model->getSelectedIPs();
    Confirmation dialog(this, ipMap);
    dialog.exec();
}

void ServerInterface::updateServerFiles(const bool closeFlag, const QString& login, const QString& password, const QString source) {

    QMap<QString, QString> ipMap = model->getSelectedIPs();
    emit sendListSize(ipMap.size());

    QList<QFuture<void>> futures;
    for (auto it = ipMap.begin(); it != ipMap.end(); ++it) {
        QString IPAddress = it.key();

        futures.append(QtConcurrent::run([=]() {
            FileOperations server = new FileOperations();

            connect(&server, &FileOperations::debugInfo, this, &ServerInterface::receiveDebugInfo);
            connect(&server, &FileOperations::copyFinished, this, &ServerInterface::handleCopyFinished);
            connect(&server, &FileOperations::recieveError, this, &ServerInterface::recieveError);
            server.connectToNetworkShare(IPAddress, "d$", login, password, closeFlag, source);
        }));
    }
}


void ServerInterface::handleCopyFinished(const QString &ipAddress, const int &finishCode) {
    emit sendUpdateProgressBar();
    // Выполните здесь необходимые действия после завершения копирования файлов, например, отключение от сетевой шары
    qDebug() << "Copying files finished for IP:" << ipAddress << "and code:" << finishCode;
    // ...
    model->updateRowColor(ipAddress, finishCode);
}

void ServerInterface::on_markSetManual_stateChanged(int arg1)
{
    model->setMarkSetManualState(arg1 == Qt::Checked);
    ui->tableView->resizeColumnsToContents();
}

bool ServerInterface::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->searchLine && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            on_searchButton_clicked();
            return true; // Поглощаем событие
        }
    }
    return false; // Передаем событие дальше для обработки другим обработчикам
}

void ServerInterface::on_searchButton_clicked()
{
    QString searchText = ui->searchLine->text(); // Получаем текст из QLineEdit для поиска
    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();

    if (selectionModel) {
        QModelIndexList indexes;
        for (int i = 0; i < model->rowCount(); ++i) {
            for (int j = 0; j < model->columnCount(); ++j) {
                QModelIndex index = model->index(i, j);
                QString cellText = index.data().toString();
                if (cellText.contains(searchText, Qt::CaseInsensitive)) {
                    indexes.append(index);
                }
            }
        }

        if (!indexes.isEmpty()) {
            ui->tableView->selectionModel()->clearSelection();
            QModelIndex firstMatchIndex = indexes.first(); // Получаем индекс первого совпадения
            ui->tableView->setCurrentIndex(firstMatchIndex); // Устанавливаем его как текущий индекс
            ui->tableView->scrollTo(firstMatchIndex, QAbstractItemView::PositionAtTop); // Прокручиваем к найденному объекту
        } else {
            QMessageBox::information(this, "Search", "No matches found.");
        }
    }
}

void ServerInterface::checkPingInMainThread(const QString &ipAddress) {
    // Создаем сокет без указания родителя
    QTcpSocket *socket = new QTcpSocket();

    // Подключаем сигналы и слоты для обработки результатов проверки пинга
    connect(socket, &QTcpSocket::connected, this, [=]() {
        qDebug() << ipAddress << "is reachable";
        socket->disconnectFromHost();
        emit pingResult(ipAddress, true);
        socket->deleteLater(); // Удаляем сокет после использования
    });

    connect(socket, &QTcpSocket::errorOccurred, this, [=](QAbstractSocket::SocketError socketError) {
        qDebug() << ipAddress << "is unreachable (error: " << socketError << ")";
        emit pingResult(ipAddress, false);
        socket->deleteLater(); // Удаляем сокет после использования
    });

    // Устанавливаем соединение
    socket->connectToHost(ipAddress, 3389);

    // Проверка соединения происходит асинхронно, результаты будут обработаны в сигналах и слотах
}


void ServerInterface::spawnTable() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("192.168.0.115");
    db.setDatabaseName("dev");
    db.setUserName("global");
    db.setPassword("");

    if (!db.open()) {
        qDebug() << "Ошибка при подключении к базе данных:" << db.lastError().text();

        // Создаем элемент в errorTreeWidget с текстом ошибки и добавляем его
        QTreeWidgetItem *errorItem = new QTreeWidgetItem(ui->errorTreeWidget);
        errorItem->setText(0, "Ошибка при подключении к базе данных");
        errorItem->setText(1, db.lastError().text());

        return;
    }

    // Выполняем запрос на получение только нужных столбцов из базы данных
    QSqlQuery query;
    if (!query.exec("SELECT Obekt, IT, IP, a1, a2 FROM acc_blank")) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();

        // Создаем элемент в errorTreeWidget с текстом ошибки и добавляем его
        QTreeWidgetItem *errorItem = new QTreeWidgetItem(ui->errorTreeWidget);
        errorItem->setText(0, "Ошибка выполнения запроса");
        errorItem->setText(1, query.lastError().text());

        return;
    }

    // Устанавливаем SQL-запрос для модели
    model->setQuery(query.lastQuery());

    // Устанавливаем модель в QTableView
    ui->tableView->setModel(model);

    // Растягиваем столбцы по содержимому
    ui->tableView->resizeColumnsToContents();

    connect(this, &ServerInterface::pingResult, model, &ObjectsTable::updatePingFlag);

    while (query.next()) {
        QString ipAddress = query.value("IP").toString();
        model->addIPAddress(ipAddress);
        // Вызываем метод для выполнения проверки пинга в основном потоке
        QMetaObject::invokeMethod(this, "checkPingInMainThread", Qt::QueuedConnection, Q_ARG(QString, ipAddress));
    }
}


void ServerInterface::onSwitchToggled(bool checked) {
    Q_UNUSED(checked);
    filterInclude = !filterInclude;
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
    QLabel *titleLabel1 = new QLabel("Градове:", this);
    titleLabel1->setEnabled(false);
    QWidgetAction *titleWidgetAction1 = new QWidgetAction(this);
    titleWidgetAction1->setDefaultWidget(titleLabel1);
    titleLabel1->setStyleSheet("QLabel { font-weight: 600; padding-right: 133px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction1);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для городов и добавляем их в меню
    QAction *filterCityAction1 = new QAction("Варна", this);
    QAction *filterCityAction2 = new QAction("Пловдив", this);
    QAction *filterCityAction3 = new QAction("София", this);
    QAction *filterCityAction4 = new QAction("Търново", this);
    filterMenu->addAction(filterCityAction1);
    filterMenu->addAction(filterCityAction2);
    filterMenu->addAction(filterCityAction3);
    filterMenu->addAction(filterCityAction4);

    // Добавляем фильтр для статуса
    QLabel *titleLabel2 = new QLabel("Статус:", this);
    titleLabel2->setEnabled(false);
    QWidgetAction *titleWidgetAction2 = new QWidgetAction(this);
    titleWidgetAction2->setDefaultWidget(titleLabel2);
    titleLabel2->setStyleSheet("QLabel { font-weight: 600; padding-right: 100px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction2);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для статуса и добавляем их в меню
    QAction *filterStatusAction1 = new QAction("Достъпни", this);
    QAction *filterStatusAction2 = new QAction("Актуализирани", this);
    QAction *filterStatusAction3 = new QAction("Архивирани", this);
    QAction *filterStatusAction4 = new QAction("Грешки", this);
    filterMenu->addAction(filterStatusAction1);
    filterMenu->addAction(filterStatusAction2);
    filterMenu->addAction(filterStatusAction3);
    filterMenu->addAction(filterStatusAction4);
    filterStatusAction1->setEnabled(false);
    filterStatusAction2->setEnabled(false);
    filterStatusAction3->setEnabled(false);
    filterStatusAction4->setEnabled(false);

    // Добавляем фильтр для тэгов
    QLabel *titleLabel3 = new QLabel("Тагове:", this);
    titleLabel3->setEnabled(false);
    QWidgetAction *titleWidgetAction3 = new QWidgetAction(this);
    titleWidgetAction3->setDefaultWidget(titleLabel3);
    titleLabel3->setStyleSheet("QLabel { font-weight: 600; padding-right: 100px; margin: 0 7px; background-color: white;}");
    filterMenu->addAction(titleWidgetAction3);

    // Добавляем разделитель
    filterMenu->addSeparator();

    // Создаем фильтры для тэгов и добавляем их в меню
    QAction *filterTagAction1 = new QAction("Клик", this);
    QAction *filterTagAction2 = new QAction("Пункт", this);
    QAction *filterTagAction3 = new QAction("Аптека", this);

    filterMenu->addAction(filterTagAction1);
    filterMenu->addAction(filterTagAction2);
    filterMenu->addAction(filterTagAction3);


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

    connect(filterCityAction4, &QAction::triggered, [=]() {
        applyCityFilter("Търново");
    });

    connect(filterStatusAction1, &QAction::triggered, [=]() {
        applyStatusFilter("Достъпни");
    });

    connect(filterStatusAction2, &QAction::triggered, [=]() {
        applyStatusFilter("Актуализирани");
    });

    connect(filterStatusAction3, &QAction::triggered, [=]() {
        applyStatusFilter("Архивирани");
    });

    connect(filterStatusAction4, &QAction::triggered, [=]() {
        applyStatusFilter("Грешки");
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

}

void ServerInterface::cancelFilter(QWidget *filterWidget) {
    if (filterWidget) {
        // Удаляем виджет из контейнера
        filterWidget->deleteLater();
        // Удаляем запись из структуры appliedFilters по ключу (указателю на виджет)
        appliedFilters.remove(filterWidget);
        // Обновляем фильтр для отображения изменений
        updateFilter();
    }
}

void ServerInterface::updateFilter() {
    ObjectsTable *model = qobject_cast<ObjectsTable*>(ui->tableView->model());
    if (!model) {
        qDebug() << "Модель данных не найдена!";
        return;
    }

    // Создаем строку запроса с начальным значением, не содержащим WHERE
    QString newQuery = currentQuery;
    // Проверяем, есть ли какие-либо фильтры
    if (!appliedFilters.isEmpty()) {
        newQuery += " WHERE";
        QStringList filters; // Создаем список для хранения обработанных фильтров
        for (const auto& query : appliedFilters) {
            filters.append(query); // Добавляем фильтр в список
        }
        // Объединяем фильтры с помощью логического оператора AND
        newQuery += filters.join(" AND ");
    }

    qDebug() << "Новый запрос:" << newQuery;

    // Устанавливаем новый SQL-запрос в модель
    model->setQuery(newQuery);

    model->rowCount();

    // Обновляем вид
    ui->tableView->resizeColumnsToContents();
}

void ServerInterface::applyFilter(const QString &columnName, const QString &value, const QString &displayText, const bool chosenOperator) {
    // Добавляем примененный фильтр в контейнер с примененными фильтрами
    QWidget *appliedFiltersContainer = ui->appliedFilters;
    if (!appliedFiltersContainer)
        return;

    // Создаем новый виджет для фильтра
    QWidget *filterWidget = new QWidget(appliedFiltersContainer);
    QString filterObjectName = "filter" + QString::number(++filterIndex);
    filterWidget->setObjectName(filterObjectName);

    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(5);
    filterLayout->setContentsMargins(9, 6, 9, 6);

    QLabel *filterLabel = new QLabel(displayText, filterWidget);
    filterLayout->addWidget(filterLabel);

    QPushButton *cancelButton = new QPushButton(filterWidget);
    QString cancelObjectName = "cancel" + filterObjectName;
    cancelButton->setObjectName(cancelObjectName);


    if (chosenOperator) {
        cancelButton->setIcon(QIcon(":/buttons/icon/x-008631-color.svg"));
    } else {
        cancelButton->setIcon(QIcon(":/buttons/icon/x-BD2C36-color.svg"));
    }

    cancelButton->setIconSize(QSize(10, 10));
    cancelButton->setFixedSize(20, 20);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        cancelFilter(filterWidget);
    });
    filterLayout->addWidget(cancelButton);

    QHBoxLayout *appliedFiltersLayout = qobject_cast<QHBoxLayout*>(appliedFiltersContainer->layout());
    if (appliedFiltersLayout) {
        appliedFiltersLayout->addWidget(filterWidget);
        appliedFiltersLayout->setAlignment(filterWidget, Qt::AlignLeft);
    } else {
        qDebug() << "Layout for applied filters container not found!";
    }

    QString query = "";
    if (chosenOperator) {
        query = " " + columnName + " LIKE '" + value + "'";
    } else {
        query = " " + columnName + " NOT LIKE '" + value + "'";

        // Устанавливаем стиль границы, фона и цвета текста в красный
        QString styleSheet = QString("#%1 { border: 2px solid #D02C37; border-radius: 10px; background-color: #FFD6D6; }"
                                     "#%1 QLabel { color: #BD2C36; }"
                                     "#%1 QPushButton { background-color: transparent; }"
                                     "#%1 QPushButton:hover { background-color: #FFC7C7; border: 2px solid #D02C37; }").arg(filterObjectName);
        filterWidget->setStyleSheet(styleSheet);
    }

    // Добавляем информацию о фильтре в QMap
    appliedFilters.insert(filterWidget, query);
    updateFilter();
}



void ServerInterface::applyCityFilter(const QString &city) {
    applyFilter("IT", city, city, filterInclude);
}

void ServerInterface::applyStatusFilter(const QString &status) {
    QString filterValue;
    QString displayText;
    if (status == "Достъпни") {
        filterValue = "";
        displayText = "Достъпни";
    } else if (status == "Актуализирани") {
        filterValue = "yes";
        displayText = "Актуализирани";
    } else if (status == "Архивирани") {
        filterValue = "yes";
        displayText = "Архивирани";
    } else if (status == "Грешки") {
        filterValue = "lg";
        displayText = "Грешки";
    }
    applyFilter("a1", filterValue, displayText, filterInclude);
}

void ServerInterface::applyTagFilter(const QString &tag) {
    QString filterValue;
    QString displayText;
    if (tag == "Клик") {
        filterValue = "%c%";
        displayText = "Клик";
        applyFilter("mode", filterValue, displayText, filterInclude);
    } else if (tag == "Пункт") {
        filterValue = "%b%";
        displayText = "Пункт";
        applyFilter("Nkod", filterValue, displayText, filterInclude);
    } else if (tag == "Аптека") {
        filterValue = "%a%";
        displayText = "Аптека";
        applyFilter("Nkod", filterValue, displayText, filterInclude);
    } else if (tag == "Нов") {
        filterValue = "%n%";
        displayText = "Нов";
        applyFilter("mode", filterValue, displayText, filterInclude);
    } else if (tag == "Затворен") {
        filterValue = "%s%";
        displayText = "Затворен";
        applyFilter("mode", filterValue, displayText, filterInclude);
    }
}

ServerInterface::~ServerInterface()
{
    delete ui;
}



/*
 *
 * это код для чека - анчека всех чекбоксов, потом въебашу куда то.
    if (!model)
        return;

    // Если галочка установлена
    if (arg1 == Qt::Checked) {
        // Обновляем данные в таблице
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex index = model->index(row, 1); // Индекс второй колонки (с чекбоксами)
            model->setData(index, Qt::Checked, Qt::CheckStateRole); // Устанавливаем состояние чекбокса
        }
    } else {
        // Если галочка не установлена
        // Обновляем данные в таблице
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex index = model->index(row, 1); // Индекс второй колонки (с чекбоксами)
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole); // Снимаем галочку с чекбокса
        }
    }
*/

