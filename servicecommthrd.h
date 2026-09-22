#ifndef SERVICECOMMTHRD_H
#define SERVICECOMMTHRD_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include <AutoSudoSdk/sdk.hpp>

class ServiceCommThrd : public QThread
{
    Q_OBJECT
public:
    explicit ServiceCommThrd(QObject *parent = nullptr);

    void run() override;

public slots:
    void requestRules(); // request rules from service


signals:
    void receivedRuleList(std::vector<RuleEntry> response);
    void ConnectionTimedOut();
    void OperationRefused();
};

#endif // SERVICECOMMTHRD_H
