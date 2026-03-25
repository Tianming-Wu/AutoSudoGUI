#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QWidget>

#include <AutoSudoSdk/sdk.hpp>

namespace Rule = AutoSudoSdk::Rule;

namespace Ui {
class EditWindow;
}

class RuleMetaDataManager;

class EditWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EditWindow(QWidget *parent = nullptr);
    ~EditWindow();

    enum ShowMode {
        Create, Modify, Delete
    };


public slots:
    void showCreate();
    void showEdit(const RuleEntry& entry);
    // void showDelete();


signals:
    void ruleCreated(uint16_t uid);

private:
    void setupGenericCb();
    void setupETypeCb();
    void setupEnabling();
    void setupWindow(); // Initialize Window Appearance

    // We do need to change the entire window if the type changed.
    void onTypeChanged();

    void onETypeChanged();
    void onActionChanged();
    void onPermChanged();

    void onCancel();
    void onAccept();

    bool onCreateAccept();
    void onModifyAccept();



private:
    Ui::EditWindow *ui;

    ShowMode m_mode;
    Rule::Type m_type = Rule::Type::Constant;
    Rule::EType m_etype = Rule::EType::Equal;
    Rule::Action m_action = Rule::Action::Approve;
    PermissionLevel m_allow = PermissionLevel::User;

    RuleEntry m_temp_entry;
};

#endif // EDITWINDOW_H
