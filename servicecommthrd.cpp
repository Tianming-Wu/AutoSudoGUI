#include "servicecommthrd.h"

ServiceCommThrd::ServiceCommThrd(QObject *parent)
    : QThread(parent)
{
}

void ServiceCommThrd::requestRules()
{
    std::vector<RuleEntry> rules;

    if (!AutoSudoSdk::TryListRules(rules)) {
        emit ConnectionTimedOut();
        return;
    }

    emit receivedRuleList(rules);
}

void ServiceCommThrd::run()
{

}
