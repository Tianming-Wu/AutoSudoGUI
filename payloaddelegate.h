#ifndef PAYLOADDELEGATE_H
#define PAYLOADDELEGATE_H

#include <QStyledItemDelegate>
#include <AutoSudoSdk/sdk.hpp>

using AutoSudoSdk::Rule::Type;

class PayloadDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PayloadDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
};

#endif // PAYLOADDELEGATE_H
