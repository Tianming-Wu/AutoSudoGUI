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

    // 校验结果结构体
    struct ValidationResult {
        bool valid;
        QString error;
    };

    // 静态校验函数，支持 Type/EType
    static ValidationResult validatePayload(Type type, int etype, const QString& payload);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;

protected:
    bool eventFilter(QObject *editor, QEvent *event) override;

private:
    void updateEditorColor(QWidget *editor, bool valid) const;
};

#endif // PAYLOADDELEGATE_H
