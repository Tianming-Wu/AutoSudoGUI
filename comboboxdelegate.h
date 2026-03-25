#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>

#include <AutoSudoSdk/sdk.hpp>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum DelegateType {
        RuleType,
        RuleEType,
        RuleAction,
        RulePermission
    };

    explicit ComboBoxDelegate(DelegateType type, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;

private:
    DelegateType m_type;
};

#endif // COMBOBOXDELEGATE_H
