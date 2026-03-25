#include "comboboxdelegate.h"
#include "rulemetadatamanager.h"

#include <QComboBox>

using AutoSudoSdk::Rule::Type;
using AutoSudoSdk::Rule::EType;
using AutoSudoSdk::Rule::Action;

ComboBoxDelegate::ComboBoxDelegate(DelegateType type, QObject *parent)
    : QStyledItemDelegate(parent), m_type(type)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QComboBox *editor = new QComboBox(parent);
    auto &mgr = RuleMetaDataManager::instance();

    switch(m_type) {
    case RuleType: {
        for(auto t : mgr.getTypeMap().keys()) {
            editor->addItem(mgr.getTypeDisplayName(t), QVariant::fromValue(static_cast<uint16_t>(t)));
        }
        break;
    }
    case RuleEType: {
        for(auto e : mgr.getETypeMap().keys()) {
            editor->addItem(mgr.getETypeDisplayName(e), QVariant::fromValue(static_cast<uint16_t>(e)));
        }
        break;
    }
    case RuleAction: {
        for(auto a : mgr.getActionMap().keys()) {
            editor->addItem(mgr.getActionDisplayName(a), QVariant::fromValue(static_cast<uint16_t>(a)));
        }
        break;
    }
    case RulePermission: {
        for(auto p : mgr.getPermissionLevelMap().keys()) {
            editor->addItem(mgr.getPermissionLevelDisplayName(p), QVariant::fromValue(static_cast<int>(p)));
        }
        break;
    }
    }

    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Read the original value from UserRole (not the display text)
    int value = index.model()->data(index, Qt::UserRole).toInt();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    for (int i = 0; i < comboBox->count(); ++i) {
        if (comboBox->itemData(i).toInt() == value) {
            comboBox->setCurrentIndex(i);
            return;
        }
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentData().toInt();

    const int oldValue = model->data(index, Qt::UserRole).toInt();
    if (oldValue == value) {
        return;
    }
    
    // Store raw value in UserRole for internal use
    model->setData(index, value, Qt::UserRole);
    
    // Update the display text via RuleMetaDataManager
    auto &mgr = RuleMetaDataManager::instance();
    QString displayText;
    
    // Determine which type this column is and get display name
    // We infer from the column but this could also be passed as a parameter
    switch(m_type) {
    case RuleType: {
        auto type = static_cast<Type>(value);
        displayText = mgr.getTypeDisplayName(type);
        break;
    }
    case RuleEType: {
        auto etype = static_cast<EType>(value);
        displayText = mgr.getETypeDisplayName(etype);
        break;
    }
    case RuleAction: {
        auto action = static_cast<Action>(value);
        displayText = mgr.getActionDisplayName(action);
        break;
    }
    case RulePermission: {
        auto perm = static_cast<PermissionLevel>(value);
        displayText = mgr.getPermissionLevelDisplayName(perm);
        break;
    }
    }
    
    model->setData(index, displayText, Qt::DisplayRole);
}
