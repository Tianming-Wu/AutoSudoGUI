#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QTableWidgetItem>

#include "servicecommthrd.h"
#include "editwindow.h"

enum RuleTableColumns {
    RTC_Order, RTC_Uid, RTC_Type, RTC_EType, RTC_Action, RTC_Payload, RTC_AllowUpTo,
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadRules(); // Request rules from the AutoSudo Service
    void onRulesListResponse(std::vector<RuleEntry> rl);
    void onTableRowsMoved(); // Handle table drag reorder
    void onRuleCellChanged(QTableWidgetItem* item);
    void onDeleteRule();
    void onExportRules();
    void onImportRules();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    ServiceCommThrd *commThread;
    QTimer* loadTimer = nullptr;

    QLabel* statusLabel = nullptr;
    EditWindow* ew = nullptr;
    bool m_loadingRules = false;
    bool m_reorderingRows = false;
    QString m_lastSubmitted;
    
};
#endif // MAINWINDOW_H
