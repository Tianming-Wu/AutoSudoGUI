#include "payloaddelegate.h"
#include "rulemetadatamanager.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

PayloadDelegate::PayloadDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
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

    QString value = index.model()->data(index, Qt::DisplayRole).toString();

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

    const QString oldText = model->data(index, Qt::DisplayRole).toString();
    if (oldText == displayText) {
        return;
    }

    model->setData(index, displayText, Qt::DisplayRole);
}
