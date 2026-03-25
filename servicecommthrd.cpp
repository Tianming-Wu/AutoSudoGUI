#include "servicecommthrd.h"

ServiceCommThrd::ServiceCommThrd()
{

    // Connections are established on-demand for each operation,
    // so this is just a placeholder.
    client.connect();
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
