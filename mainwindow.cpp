#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "comboboxdelegate.h"
#include "rulemetadatamanager.h"
#include "payloaddelegate.h"

#include <QEvent>
#include <QPalette>
#include <QSignalBlocker>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , commThread(new ServiceCommThrd(this))
    , ew(new EditWindow())
{
    ui->setupUi(this);

    auto refreshPriorityHeader = [this]() {
        for (int row = 0; row < ui->tbRules->rowCount(); ++row) {
            ui->tbRules->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        }
    };

    // Set the columns
    ui->tbRules->setColumnCount(7);
    ui->tbRules->setHorizontalHeaderLabels({
        qtTrId("rule.field.order"), qtTrId("rule.field.uid"), qtTrId("rule.field.type"),
        qtTrId("rule.field.etype"), qtTrId("rule.field.action"), qtTrId("rule.field.payload"), qtTrId("rule.field.allow_up_to")
    });
    // ui->tbRules->horizontalHeader()->setStretchLastSection(true);
    ui->tbRules->horizontalHeader()->setSectionResizeMode(RTC_Payload, QHeaderView::Stretch);

    // Set some column width
    ui->tbRules->setColumnWidth(RTC_Uid, 40);
    ui->tbRules->setColumnWidth(RTC_Type, 140);

    ui->tbRules->setColumnWidth(RTC_AllowUpTo, 60);

    // Keep order as internal sort key, but hide it in UI
    ui->tbRules->setColumnHidden(RTC_Order, true);

    // Use row header as visible priority and make rows denser
    ui->tbRules->verticalHeader()->setDefaultSectionSize(22);
    ui->tbRules->verticalHeader()->setMinimumSectionSize(18);
    ui->tbRules->verticalHeader()->setVisible(true);

    // Setup delegates for inline editing
    ui->tbRules->setItemDelegateForColumn(RTC_Type, new ComboBoxDelegate(ComboBoxDelegate::RuleType, this));
    ui->tbRules->setItemDelegateForColumn(RTC_EType, new ComboBoxDelegate(ComboBoxDelegate::RuleEType, this));
    ui->tbRules->setItemDelegateForColumn(RTC_Action, new ComboBoxDelegate(ComboBoxDelegate::RuleAction, this));
    ui->tbRules->setItemDelegateForColumn(RTC_Payload, new PayloadDelegate(this));
    ui->tbRules->setItemDelegateForColumn(RTC_AllowUpTo, new ComboBoxDelegate(ComboBoxDelegate::RulePermission, this));

    // Enable drag-and-drop reordering
    ui->tbRules->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tbRules->setDefaultDropAction(Qt::MoveAction);
    ui->tbRules->setDragDropOverwriteMode(false);
    ui->tbRules->setDragEnabled(true);
    ui->tbRules->setAcceptDrops(true);
    ui->tbRules->setDropIndicatorShown(true);
    ui->tbRules->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbRules->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tbRules->viewport()->installEventFilter(this);

    // Connect drag-drop signal
    connect(ui->tbRules->model(), &QAbstractItemModel::rowsMoved, this, &MainWindow::onTableRowsMoved);
    connect(ui->tbRules, &QTableWidget::itemChanged, this, &MainWindow::onRuleCellChanged);

    // Connect signals and slots
    connect(commThread, &ServiceCommThrd::receivedRuleList, this, &MainWindow::onRulesListResponse);
    connect(commThread, &ServiceCommThrd::ConnectionTimedOut, this, [this]() {
        statusLabel->setText(qtTrId("mw.status.service_unavailable"));
    });
    connect(ew, &EditWindow::ruleCreated, this, [this](uint16_t) {
        loadRules();
    });

    // Enable sorting
    ui->tbRules->setSortingEnabled(true);
    ui->tbRules->sortByColumn(RTC_Order, Qt::AscendingOrder); // smaller the order, higher the priority
    ui->tbRules->setSortingEnabled(false);
    refreshPriorityHeader();

    // Menu
    connect(ui->acExit, &QAction::triggered, this, [&]() { this->close(); });

    connect(ui->acRefreshRule, &QAction::triggered, this, &MainWindow::loadRules);
    connect(ui->acNewRule, &QAction::triggered, this, [&]() { ew->showCreate(); });
    connect(ui->acDeleteRule, &QAction::triggered, this, &MainWindow::onDeleteRule);

    connect(ui->acAboutQt, &QAction::triggered, this, []{ QApplication::aboutQt(); });

    // Status Bar
    statusLabel = new QLabel();
    ui->statusBar->addWidget(statusLabel);

    QTimer::singleShot(0, this, &MainWindow::loadRules); // Load rules after the event loop starts
}

MainWindow::~MainWindow()
{
    if (commThread != nullptr && commThread->isRunning()) {
        commThread->quit();
        commThread->wait();
    }
    
    // ui will manage statusLabel's lifecycle, no need to delete it separately
    delete ui;

    if(ew != nullptr) delete ew;
}

void MainWindow::loadRules()
{
    statusLabel->setText(qtTrId("mw.status.sending"));
    commThread->requestRules();
}

void MainWindow::onRulesListResponse(std::vector<RuleEntry> rl)
{
    m_loadingRules = true;
    m_reorderingRows = false;
    QSignalBlocker blockTableSignals(ui->tbRules);

    auto refreshPriorityHeader = [this]() {
        for (int row = 0; row < ui->tbRules->rowCount(); ++row) {
            ui->tbRules->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        }
    };

    // Save the current uid to restore selection after refresh. If no rules, set to 0 to indicate no selection.
    // (Note: 0 is not a valid UID for rules, since it was reserved.)
    uint16_t oldSeluid = 0;
    if (ui->tbRules->currentRow() >= 0) {
        QTableWidgetItem* uidItem = ui->tbRules->item(ui->tbRules->currentRow(), RTC_Uid);
        if (uidItem != nullptr) {
            oldSeluid = static_cast<uint16_t>(uidItem->data(Qt::DisplayRole).toInt());
        }
    }

    ui->tbRules->clearContents();

    ui->tbRules->setRowCount(0);

    // Temporary disable sorting to avoid incorrect insert
    ui->tbRules->setSortingEnabled(false);

    auto &mgr = RuleMetaDataManager::instance();

    for(const RuleEntry& re : rl) {
        int row = ui->tbRules->rowCount();
        ui->tbRules->insertRow(row);

        // Order column
        QTableWidgetItem* orderItem = new QTableWidgetItem();
        orderItem->setData(Qt::DisplayRole, static_cast<int>(re.order));
        orderItem->setData(Qt::UserRole, static_cast<int>(re.order)); // For sorting
        orderItem->setFlags(orderItem->flags() & ~Qt::ItemIsEditable);
        ui->tbRules->setItem(row, RTC_Order, orderItem);

        // UID column
        QTableWidgetItem* uidItem = new QTableWidgetItem();
        uidItem->setData(Qt::DisplayRole, static_cast<int>(re.uid));
        uidItem->setFlags(uidItem->flags() & ~Qt::ItemIsEditable);
        ui->tbRules->setItem(row, RTC_Uid, uidItem);

        // Type column - display localized name, store raw value
        QTableWidgetItem* typeItem = new QTableWidgetItem();
        auto type = static_cast<Type>(re.type);
        typeItem->setData(Qt::DisplayRole, mgr.getTypeDisplayName(type));
        typeItem->setData(Qt::UserRole, static_cast<int>(re.type)); // For sorting and editing
        ui->tbRules->setItem(row, RTC_Type, typeItem);

        // EType column - display localized name, store raw value
        QTableWidgetItem* etypeItem = new QTableWidgetItem();
        auto etype = static_cast<EType>(re.etype);
        etypeItem->setData(Qt::DisplayRole, mgr.getETypeDisplayName(etype));
        etypeItem->setData(Qt::UserRole, static_cast<int>(re.etype)); // For sorting and editing
        ui->tbRules->setItem(row, RTC_EType, etypeItem);

        // Action column - display localized action name, store raw value
        QTableWidgetItem* actionItem = new QTableWidgetItem();
        auto action = static_cast<Action>(re.action);
        actionItem->setData(Qt::DisplayRole, mgr.getActionDisplayName(action));
        actionItem->setData(Qt::UserRole, static_cast<int>(re.action));
        ui->tbRules->setItem(row, RTC_Action, actionItem);

        // Payload column - display converted payload text according to rule type
        QTableWidgetItem* payloadItem = new QTableWidgetItem();
        const auto payloadConstraint = mgr.getPayloadConstraint(type);
        const QString payloadText = mgr.payloadToDisplayText(re);
        payloadItem->setData(Qt::UserRole, payloadText);
        if (payloadConstraint.type == PayloadConstraint::NoPayload) {
            payloadItem->setData(Qt::DisplayRole, payloadText.isEmpty() ? qtTrId("rule.payload.none") : payloadText);
            payloadItem->setFlags(payloadItem->flags() & ~Qt::ItemIsEditable);
            payloadItem->setForeground(ui->tbRules->palette().brush(QPalette::Disabled, QPalette::Text));
        } else {
            payloadItem->setData(Qt::DisplayRole, payloadText);
            payloadItem->setForeground(QBrush());
        }
        ui->tbRules->setItem(row, RTC_Payload, payloadItem);

        // AllowUpTo column - display localized permission name, store raw value
        QTableWidgetItem* allowUpToItem = new QTableWidgetItem();
        auto perm = static_cast<PermissionLevel>(re.allowUpTo);
        allowUpToItem->setData(Qt::DisplayRole, mgr.getPermissionLevelDisplayName(perm));
        allowUpToItem->setData(Qt::UserRole, static_cast<int>(re.allowUpTo)); // For sorting and editing
        ui->tbRules->setItem(row, RTC_AllowUpTo, allowUpToItem);
    }

    // Re-enable Sorting
    ui->tbRules->setSortingEnabled(true);
    ui->tbRules->sortByColumn(RTC_Order, Qt::AscendingOrder);
    ui->tbRules->setSortingEnabled(false);
    refreshPriorityHeader();

    // Restore selection
    if(oldSeluid != 0) {
        for (int i = 0; i < ui->tbRules->rowCount(); ++i) {
            if (ui->tbRules->item(i, RTC_Uid)->data(Qt::DisplayRole).toInt() == oldSeluid) {
                ui->tbRules->setCurrentCell(i, RTC_Uid);
                break;
            }
        }
    }

    m_loadingRules = false;
    m_lastSubmitted.clear();
    statusLabel->setText(qtTrId("mw.status.ready"));
}

void MainWindow::onTableRowsMoved()
{
    if (m_loadingRules || m_reorderingRows) {
        return;
    }

    m_reorderingRows = true;
    QSignalBlocker blockTableSignals(ui->tbRules);

    // Extract UIDs in current table order
    std::vector<uint16_t> orderedUids;
    orderedUids.reserve(static_cast<size_t>(ui->tbRules->rowCount()));
    for (int row = 0; row < ui->tbRules->rowCount(); ++row) {
        QTableWidgetItem* uidItem = ui->tbRules->item(row, RTC_Uid);
        if (!uidItem) {
            statusLabel->setText(qtTrId("mw.status.modify_failed").arg(QStringLiteral("invalid uid row")));
            m_reorderingRows = false;
            QTimer::singleShot(0, this, &MainWindow::loadRules);
            return;
        }

        bool ok = false;
        const int uidInt = uidItem->data(Qt::DisplayRole).toInt(&ok);
        if (!ok || uidInt <= 0) {
            statusLabel->setText(qtTrId("mw.status.modify_failed").arg(QStringLiteral("invalid uid value")));
            m_reorderingRows = false;
            QTimer::singleShot(0, this, &MainWindow::loadRules);
            return;
        }

        orderedUids.push_back(static_cast<uint16_t>(uidInt));
    }

    // Sync the new order with the service
    if (!orderedUids.empty() && static_cast<int>(orderedUids.size()) == ui->tbRules->rowCount()) {
        AutoSudoSdk::ReorderRulesByUidOrder(orderedUids);
        statusLabel->setText(qtTrId("mw.status.reordered"));
        m_reorderingRows = false;
        QTimer::singleShot(0, this, &MainWindow::loadRules);
        return;
    }

    m_reorderingRows = false;
}

void MainWindow::onRuleCellChanged(QTableWidgetItem* item)
{
    if (item == nullptr || m_loadingRules || m_reorderingRows) {
        return;
    }

    if (item->column() != RTC_Type && item->column() != RTC_EType &&
        item->column() != RTC_Action && item->column() != RTC_Payload &&
        item->column() != RTC_AllowUpTo) {
        return;
    }

    const int row = item->row();
    if (row < 0) {
        return;
    }

    QTableWidgetItem* uidItem = ui->tbRules->item(row, RTC_Uid);
    QTableWidgetItem* typeItem = ui->tbRules->item(row, RTC_Type);
    QTableWidgetItem* etypeItem = ui->tbRules->item(row, RTC_EType);
    QTableWidgetItem* actionItem = ui->tbRules->item(row, RTC_Action);
    QTableWidgetItem* payloadItem = ui->tbRules->item(row, RTC_Payload);
    QTableWidgetItem* allowItem = ui->tbRules->item(row, RTC_AllowUpTo);

    if (!uidItem || !typeItem || !etypeItem || !actionItem || !payloadItem || !allowItem) {
        return;
    }

    if (!typeItem->data(Qt::UserRole).isValid() ||
        !etypeItem->data(Qt::UserRole).isValid() ||
        !actionItem->data(Qt::UserRole).isValid() ||
        !allowItem->data(Qt::UserRole).isValid()) {
        statusLabel->setText(qtTrId("mw.status.modify_failed").arg(QStringLiteral("invalid row state")));
        QTimer::singleShot(0, this, &MainWindow::loadRules);
        return;
    }

    const uint16_t uid = static_cast<uint16_t>(uidItem->data(Qt::DisplayRole).toInt());
    const auto type = static_cast<Type>(typeItem->data(Qt::UserRole).toInt());

    auto &mgr = RuleMetaDataManager::instance();

    if (item->column() == RTC_Type) {
        const auto oldEType = static_cast<EType>(etypeItem->data(Qt::UserRole).toInt());
        const auto availableETypes = mgr.getAvailableETypes(type);

        bool oldETypeStillAvailable = false;
        for (const auto &pair : availableETypes) {
            if (pair.second == oldEType) {
                oldETypeStillAvailable = true;
                break;
            }
        }

        if (!oldETypeStillAvailable && !availableETypes.empty()) {
            const auto fallbackEType = availableETypes.front().second;
            QSignalBlocker blockTableSignals(ui->tbRules);
            etypeItem->setData(Qt::UserRole, static_cast<int>(fallbackEType));
            etypeItem->setData(Qt::DisplayRole, mgr.getETypeDisplayName(fallbackEType));
        }

        const auto payloadConstraint = mgr.getPayloadConstraint(type);
        const QString rawPayload = payloadItem->data(Qt::UserRole).toString();
        Qt::ItemFlags payloadFlags = payloadItem->flags();
        if (payloadConstraint.type == PayloadConstraint::NoPayload) {
            payloadFlags &= ~Qt::ItemIsEditable;
            payloadItem->setFlags(payloadFlags);
            payloadItem->setData(Qt::DisplayRole, rawPayload.isEmpty() ? qtTrId("rule.payload.none") : rawPayload);
            payloadItem->setForeground(ui->tbRules->palette().brush(QPalette::Disabled, QPalette::Text));
        } else {
            payloadFlags |= Qt::ItemIsEditable;
            payloadItem->setFlags(payloadFlags);
            payloadItem->setData(Qt::DisplayRole, rawPayload);
            payloadItem->setForeground(QBrush());
        }
    }

    const auto etype = static_cast<EType>(etypeItem->data(Qt::UserRole).toInt());
    const auto action = static_cast<Action>(actionItem->data(Qt::UserRole).toInt());
    const auto allow = static_cast<PermissionLevel>(allowItem->data(Qt::UserRole).toInt());
    const QString payloadText = payloadItem->data(Qt::UserRole).toString();

    // 校验 payload
    auto payloadResult = PayloadDelegate::validatePayload(type, static_cast<int>(etype), payloadText);
    if (!payloadResult.valid) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("无效输入"));
        msgBox.setText(tr("Payload 不符合要求：%1\n是否丢弃本次修改？").arg(payloadResult.error));
        QPushButton *discardBtn = msgBox.addButton(tr("丢弃修改"), QMessageBox::RejectRole);
        QPushButton *editBtn = msgBox.addButton(tr("返回继续编辑"), QMessageBox::AcceptRole);
        msgBox.setDefaultButton(editBtn);
        msgBox.exec();
        if (msgBox.clickedButton() == discardBtn) {
            QSignalBlocker blockTableSignals(ui->tbRules);
            loadRules();
        }
        // 返回继续编辑则什么都不做
        return;
    }

    // The delegates write the user role and the display role separately, so one edit
    // arrives here twice; only the first one needs to reach the service.
    const QString submitKey = QStringLiteral("%1|%2|%3|%4|%5|%6")
                                  .arg(uid)
                                  .arg(static_cast<int>(type))
                                  .arg(static_cast<int>(etype))
                                  .arg(static_cast<int>(action))
                                  .arg(static_cast<int>(allow))
                                  .arg(payloadText);
    if (submitKey == m_lastSubmitted) {
        return;
    }

    QString errorMessage;
    const bool ok = mgr.modifyRuleByType(uid, type, etype, action, allow, payloadText, &errorMessage);

    if (!ok) {
        statusLabel->setText(QString(qtTrId("mw.status.modify_failed")).arg(errorMessage));
        loadRules();
        return;
    }

    m_lastSubmitted = submitKey;
    statusLabel->setText(qtTrId("mw.status.modified"));
    QTimer::singleShot(0, this, &MainWindow::loadRules);
}

void MainWindow::onDeleteRule()
{
    const int row = ui->tbRules->currentRow();
    if (row < 0) {
        statusLabel->setText(qtTrId("mw.status.no_selection"));
        return;
    }

    QTableWidgetItem* uidItem = ui->tbRules->item(row, RTC_Uid);
    bool ok = false;
    const int uid = uidItem ? uidItem->data(Qt::DisplayRole).toInt(&ok) : 0;
    if (!ok || uid <= 0) {
        statusLabel->setText(QString(qtTrId("mw.status.modify_failed")).arg(QStringLiteral("invalid uid value")));
        return;
    }

    const auto answer = QMessageBox::question(this, qtTrId("mw.delete.title"),
                                              qtTrId("mw.delete.question").arg(uid),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!AutoSudoSdk::DeleteRule(static_cast<uint16_t>(uid))) {
        statusLabel->setText(QString(qtTrId("mw.status.delete_failed")).arg(uid));
        return;
    }

    statusLabel->setText(qtTrId("mw.status.deleted"));
    QTimer::singleShot(0, this, &MainWindow::loadRules);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tbRules->viewport() && event->type() == QEvent::Drop) {
        QTimer::singleShot(0, this, &MainWindow::onTableRowsMoved);
    }

    return QMainWindow::eventFilter(watched, event);
}
