#include "editwindow.h"
#include "ui_editwindow.h"
#include "rulemetadatamanager.h"

#include "payloaddelegate.h"

#include <QMap>
#include <QMessageBox>

EditWindow::EditWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditWindow)
{
    ui->setupUi(this);

    ui->lb1->setText(qtTrId("rule.field.payload"));
    ui->lb2->setText(qtTrId("rule.field.type"));
    ui->lb3->setText(qtTrId("rule.field.etype"));
    ui->lb4->setText(qtTrId("rule.field.action"));
    ui->lb5->setText(qtTrId("rule.field.allow_up_to"));

    setupGenericCb(); // Some ComboBoxes are only requied to initialize once, since they should not be affected by other values.
    setupETypeCb(); // Setup an initial EType ComboBox, it will be updated when type changed.

    connect(ui->cbType, &QComboBox::currentIndexChanged, this, &EditWindow::onTypeChanged);

    connect(ui->cbEType, &QComboBox::currentIndexChanged, this, &EditWindow::onETypeChanged);
    connect(ui->cbAction, &QComboBox::currentIndexChanged, this, &EditWindow::onActionChanged);
    connect(ui->cbPerm, &QComboBox::currentIndexChanged, this, &EditWindow::onPermChanged);

    connect(ui->pbCancel, &QPushButton::clicked, this, &EditWindow::onCancel);
    connect(ui->pbAccept, &QPushButton::clicked, this, &EditWindow::onAccept);

    // Call once to make sure UI is in the right state
    onTypeChanged();
}

EditWindow::~EditWindow()
{
    delete ui;
}

void EditWindow::showCreate()
{
    m_mode = ShowMode::Create;

    if(!isVisible()) show();

    setupEnabling(); // Initialize UI Status
    setupWindow();
}

void EditWindow::showEdit(const RuleEntry &entry)
{
    m_mode = ShowMode::Modify;

    if(!isVisible()) show();
    
    setupEnabling(); // Initialize UI Status
    setupWindow();

    switch(m_mode) {
    case ShowMode::Create: {

        break;
    }
    case ShowMode::Modify: {


        break;
    }
    case Delete:
        break;
    }
}

void EditWindow::setupGenericCb()
{
    ui->cbType->clear();
    ui->cbAction->clear();

    using AutoSudoSdk::Rule::Type;
    auto &mgr = RuleMetaDataManager::instance();

    for(auto t : mgr.getTypeMap().keys()) {
        ui->cbType->addItem(mgr.getTypeDisplayName(t), QVariant::fromValue(static_cast<uint16_t>(t)));
    }

    for(auto a : mgr.getActionMap().keys()) {
        ui->cbAction->addItem(mgr.getActionDisplayName(a), QVariant::fromValue(static_cast<uint16_t>(a)));
    }

    for(auto p : mgr.getPermissionLevelMap().keys()) {
        ui->cbPerm->addItem(mgr.getPermissionLevelDisplayName(p), QVariant::fromValue(static_cast<int>(p)));
    }
}

void EditWindow::setupETypeCb()
{
    ui->cbEType->clear();
    auto &mgr = RuleMetaDataManager::instance();

    auto e = mgr.getAvailableETypes(m_type);

    if (e.empty()) {
        ui->cbEType->setEnabled(false); // Disable when there's nothing to select.
        return;
    } else {
        ui->cbEType->setEnabled(true);
    }

    for(const auto& [str, etype] : e) {
        ui->cbEType->addItem(mgr.getETypeDisplayName(etype), QVariant::fromValue(static_cast<uint16_t>(etype)));
    }
}

void EditWindow::setupEnabling()
{
    switch(m_mode) {
    case ShowMode::Create: {
        ui->cbType->setEnabled(true);
        break;
    }
    case ShowMode::Modify: {
        ui->cbType->setEnabled(false);

        break;
    }
    case ShowMode::Delete: {
        ui->cbType->setEnabled(false);

        break;
    }
    }
}

void EditWindow::setupWindow()
{
    static const QMap<ShowMode, QString> m =
    {
        {ShowMode::Create, qtTrId("ew.title.create")},
        {ShowMode::Modify, qtTrId("ew.title.modify")},
        {ShowMode::Delete, qtTrId("ew.title.delete")}
    };

    setWindowTitle(m.value(m_mode, QString("Unknown Mode %1").arg(static_cast<uint16_t>(m_mode))));
}

void EditWindow::onTypeChanged()
{
    using AutoSudoSdk::Rule::Type;
    auto &mgr = RuleMetaDataManager::instance();

    m_type = ui->cbType->currentData().value<Type>(); // Get selected type

    setupETypeCb(); // Update EType ComboBox according to the type

    // Setup edit box state
    ui->lePayload->setEnabled(mgr.getTypeMap().value(m_type).uicfg.editEnabled);
}

void EditWindow::onETypeChanged()
{
    // Just save it for later use
    m_etype = ui->cbEType->currentData().value<Rule::EType>();
}

void EditWindow::onActionChanged()
{
    m_action = ui->cbAction->currentData().value<Rule::Action>();
}

void EditWindow::onPermChanged()
{
    m_allow = static_cast<PermissionLevel>(ui->cbPerm->currentData().toInt());

    // Post a warning for the system level permsission
    if(m_allow == PermissionLevel::System) {
        ui->lbWarning->setText(qtTrId("ew.warning.system_permission"));
        ui->lbWarning->setVisible(true);
    } else {
        ui->lbWarning->setVisible(false);
    }
}

void EditWindow::onCancel()
{

    this->hide();
}

void EditWindow::onAccept()
{
    bool shouldHide = true;

    switch(m_mode) {
    case ShowMode::Create: {
        if (onCreateAccept()) {
            emit ruleCreated(0);
        } else {
            shouldHide = false;
        }
        break;
    }
    case ShowMode::Modify: {
        onModifyAccept();
        break;
    }
    case ShowMode::Delete: {
        // onDeleteAccept();
        break;
    }
    }

    ///TODO: emit a signal to tell main window to refresh
    if (shouldHide) {
        this->hide();
    }
}

bool EditWindow::onCreateAccept()
{
    // 校验 payload
    auto payloadResult = PayloadDelegate::validatePayload(m_type, static_cast<int>(m_etype), ui->lePayload->text());
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
            return true; // 丢弃，关闭窗口
        }
        // 返回继续编辑
        return false;
    }
    auto &mgr = RuleMetaDataManager::instance();
    QString errorMessage;
    const bool ok = mgr.createRuleByType(m_type, m_etype, m_action, m_allow, ui->lePayload->text(), &errorMessage);
    if (!ok) {
        QMessageBox::critical(this,
                              qtTrId("ew.create.error.invalid_type"),
                              qtTrId("ew.create.error.invalid_type.desc") + QStringLiteral("\n") + errorMessage);
        return false;
    }
    return true;
}

void EditWindow::onModifyAccept()
{
    // 校验 payload
    auto payloadResult = PayloadDelegate::validatePayload(m_type, static_cast<int>(m_etype), ui->lePayload->text());
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
            this->hide();
        }
        // 返回继续编辑
        return;
    }
    // ...原有保存逻辑可在此补充...
}
