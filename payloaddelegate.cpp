#include "payloaddelegate.h"
#include "rulemetadatamanager.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QEvent>

PayloadDelegate::PayloadDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

PayloadDelegate::ValidationResult PayloadDelegate::validatePayload(Type type, int etype, const QString &payload)
{
    // Path/ExecutableName/Directory/StartupDir/CustomScript/Parameter/Parameters: 不限制
    switch(type) {
    case Type::DirectoryRule:
    case Type::StartupDirectoryRule:
    case Type::FullPathRule:
    case Type::ExecutableNameRule:
    case Type::CustomScriptRule:
    case Type::ParameterRule:
    case Type::ParametersRule:
        return {true, {}};
    case Type::SidRule:
        if (payload.isEmpty()) return {false, QObject::tr("SID 不能为空")};
        if (!payload.startsWith("S-1-")) return {false, QObject::tr("SID 必须以 S-1- 开头")};
        if (!QRegularExpression(R"(^S-1-\d+(-\d+)+$)").match(payload).hasMatch())
            return {false, QObject::tr("SID 格式无效")};
        return {true, {}};
    case Type::SessionRule:
    case Type::VoteRule:
        if (!QRegularExpression("^\\d+$").match(payload).hasMatch())
            return {false, QObject::tr("必须为整数")};
        return {true, {}};
    case Type::HashRule:
        if (!QRegularExpression("^[0-9a-fA-F]+$").match(payload).hasMatch())
            return {false, QObject::tr("必须为十六进制字符串")};
        return {true, {}};
    case Type::DateRule:
        if (!QRegularExpression("^\\d{4}-\\d{2}-\\d{2}$").match(payload).hasMatch())
            return {false, QObject::tr("日期格式应为 yyyy-MM-dd")};
        return {true, {}};
    case Type::TimeRule:
        if (!QRegularExpression("^\\d{2}:\\d{2}:\\d{2}$").match(payload).hasMatch())
            return {false, QObject::tr("时间格式应为 HH:mm:ss")};
        return {true, {}};
    case Type::DateTimeRule:
    case Type::FileTimeRule:
        if (!QRegularExpression("^\\d{4}-\\d{2}-\\d{2}[ T]\\d{2}:\\d{2}:\\d{2}$").match(payload).hasMatch())
            return {false, QObject::tr("日期时间格式应为 yyyy-MM-dd[ T]HH:mm:ss")};
        return {true, {}};
    default:
        return {true, {}};
    }
}
// 编辑时变色
void PayloadDelegate::updateEditorColor(QWidget *editor, bool valid) const {
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        QPalette pal = lineEdit->palette();
        pal.setColor(QPalette::Text, valid ? Qt::black : Qt::red);
        lineEdit->setPalette(pal);
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
        QPalette pal = spinBox->palette();
        pal.setColor(QPalette::Text, valid ? Qt::black : Qt::red);
        spinBox->setPalette(pal);
    }
}

// 事件过滤：输入时动态校验
bool PayloadDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
            // 取 model/index
            QModelIndex index = lineEdit->property("modelIndex").value<QModelIndex>();
            if (!index.isValid()) return false;
            auto model = const_cast<QAbstractItemModel*>(index.model());
            QModelIndex typeIndex = model->index(index.row(), 2);
            QModelIndex etypeIndex = model->index(index.row(), 3);
            int typeValue = model->data(typeIndex, Qt::UserRole).toInt();
            int etypeValue = model->data(etypeIndex, Qt::UserRole).toInt();
            auto result = validatePayload(static_cast<Type>(typeValue), etypeValue, lineEdit->text());
            updateEditorColor(lineEdit, result.valid);
        } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
            QModelIndex index = spinBox->property("modelIndex").value<QModelIndex>();
            if (!index.isValid()) return false;
            auto model = const_cast<QAbstractItemModel*>(index.model());
            QModelIndex typeIndex = model->index(index.row(), 2);
            int typeValue = model->data(typeIndex, Qt::UserRole).toInt();
            auto result = validatePayload(static_cast<Type>(typeValue), 0, QString::number(spinBox->value()));
            updateEditorColor(spinBox, result.valid);
        }
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

QWidget *PayloadDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option);

    // Get the Type from the same row (RTC_Type column is at index 2)
    // We need to access the table model to get the type of this row
    const QAbstractItemModel *model = index.model();
    QModelIndex typeIndex = model->index(index.row(), 2); // RTC_Type = 2
    int typeValue = model->data(typeIndex, Qt::UserRole).toInt();
    auto type = static_cast<Type>(typeValue);

    auto &mgr = RuleMetaDataManager::instance();
    const auto payloadConstraint = mgr.getPayloadConstraint(type);

    // Based on constraint type, create appropriate editor
    switch(payloadConstraint.type) {
    case PayloadConstraint::NoPayload: {
        // Return nullptr to disable editing
        return nullptr;
    }
    case PayloadConstraint::Integer: {
        QSpinBox *spinBox = new QSpinBox(parent);
        spinBox->setRange(0, INT_MAX);
        return spinBox;
    }
    case PayloadConstraint::String: {
        return new QLineEdit(parent);
    }
    case PayloadConstraint::Date: {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}$")), lineEdit));
        return lineEdit;
    }
    case PayloadConstraint::Time: {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{2}:\\d{2}:\\d{2}$")), lineEdit));
        return lineEdit;
    }
    case PayloadConstraint::DateTime: {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}[ T]\\d{2}:\\d{2}:\\d{2}$")), lineEdit));
        return lineEdit;
    }
    case PayloadConstraint::Hash: {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^[0-9a-fA-F]*$")), lineEdit));
        return lineEdit;
    }
    }

    return nullptr;
}

void PayloadDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!editor) return;

    QString value = index.model()->data(index, Qt::UserRole).toString();

    // Try to cast to different widget types
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        lineEdit->setText(value);
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
        spinBox->setValue(value.toInt());
    }
}

void PayloadDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    if (!editor) return;

    QString displayText;

    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        displayText = lineEdit->text();
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
        displayText = QString::number(spinBox->value());
    }

    const QString oldText = model->data(index, Qt::UserRole).toString();
    if (oldText == displayText) {
        return;
    }

    model->setData(index, displayText, Qt::UserRole);
    model->setData(index, displayText, Qt::DisplayRole);
}
