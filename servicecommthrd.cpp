#include "servicecommthrd.h"

ServiceCommThrd::ServiceCommThrd(QObject *parent)
    : QThread(parent)
{
}

void ServiceCommThrd::requestRules()
{
    std::vector<RuleEntry> rules;

    // The two things this side can tell apart: a control channel that could not be opened, and one
    // that took the request and closed it without an answer. The second is what a refusal looks
    // like from here - the service does not send a reply it would then have to wait for a client to
    // read, because a client that never reads would hold a service thread.
    RuleClient client;
    if (!client.listRules(rules)) {
        if (client.failure() == RuleClient::Failure::NotReached) {
            emit ConnectionTimedOut();
        } else {
            emit OperationRefused();
        }
        return;
    }

    emit receivedRuleList(rules);
}

void ServiceCommThrd::run()
{

}
