#ifndef SERVICECOMMTHRD_H
#define SERVICECOMMTHRD_H

#include <QObject>
#include <QThread>
#include <QTimer>

// #include <libpipe.hpp>

#include <AutoSudoSdk/sdk.hpp>

class ServiceCommThrd : public QThread
{
    Q_OBJECT
public:
    ServiceCommThrd();

    void run() override;

public slots:
    void requestRules(); // request rules from service


signals:
    void receivedRuleList(std::vector<RuleEntry> response);
    void ConnectionTimedOut();

private:
    RuleClient client;
};

#endif // SERVICECOMMTHRD_H
