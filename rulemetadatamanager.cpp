#include "rulemetadatamanager.h"

RuleMetaDataManager& RuleMetaDataManager::instance()
{
    static RuleMetaDataManager mgr;
    return mgr;
}

RuleMetaDataManager::RuleMetaDataManager()
{
    // Initialize maps on demand in getter methods
}

RuleMetaDataManager::~RuleMetaDataManager()
{
}

const QMap<Type, TypeMetaData>& RuleMetaDataManager::getTypeMap(bool forceReload)
{
    if(m_typeMap.empty() || forceReload) {
        m_typeMap =
        {
            {Type::Constant,             {qtTrId("rule.type.constant"),          {false}, {PayloadConstraint::NoPayload, qtTrId("rule.payload.none")}}},
            {Type::DirectoryRule,        {qtTrId("rule.type.directory"),         {true },  {PayloadConstraint::String,    qtTrId("rule.payload.directory")}}},
            {Type::FullPathRule,         {qtTrId("rule.type.fullpath"),          {true },  {PayloadConstraint::String,    qtTrId("rule.payload.path")}}},
            {Type::ExecutableNameRule,   {qtTrId("rule.type.executable_name"),   {true },  {PayloadConstraint::String,    qtTrId("rule.payload.executable")}}},
            {Type::StartupDirectoryRule, {qtTrId("rule.type.startup_directory"), {true },  {PayloadConstraint::String,    qtTrId("rule.payload.directory")}}},
            {Type::SidRule,              {qtTrId("rule.type.sid"),               {true },  {PayloadConstraint::String,    qtTrId("rule.payload.sid")}}},
            {Type::SessionRule,          {qtTrId("rule.type.session"),           {true },  {PayloadConstraint::Integer,   qtTrId("rule.payload.session_id")}}},
            {Type::ParameterRule,        {qtTrId("rule.type.parameter"),         {true },  {PayloadConstraint::String,    qtTrId("rule.payload.regex")}}},
            {Type::ParametersRule,       {qtTrId("rule.type.parameters"),        {true },  {PayloadConstraint::String,    qtTrId("rule.payload.regex")}}},
            {Type::CustomScriptRule,     {qtTrId("rule.type.custom_script"),     {true },  {PayloadConstraint::String,    qtTrId("rule.payload.script_path")}}},
            {Type::DateRule,             {qtTrId("rule.type.date"),              {true },  {PayloadConstraint::Date,      qtTrId("rule.payload.date")}}},
            {Type::TimeRule,             {qtTrId("rule.type.time"),              {true },  {PayloadConstraint::Time,      qtTrId("rule.payload.time")}}},
            {Type::DateTimeRule,         {qtTrId("rule.type.datetime"),          {true },  {PayloadConstraint::DateTime,  qtTrId("rule.payload.datetime")}}},
            {Type::FileTimeRule,         {qtTrId("rule.type.file_time"),         {true },  {PayloadConstraint::DateTime,  qtTrId("rule.payload.file_time")}}},
            {Type::HashRule,             {qtTrId("rule.type.hash"),              {true },  {PayloadConstraint::Hash,      qtTrId("rule.payload.hash")}}},
            {Type::VoteRule,             {qtTrId("rule.type.vote"),              {true },  {PayloadConstraint::Integer,   qtTrId("rule.payload.vote")}}},
            {Type::DigitalSignatureRule, {qtTrId("rule.type.digital_signature"), {false}, {PayloadConstraint::NoPayload, qtTrId("rule.payload.none")}}}
        };
    }
    return m_typeMap;
}

const QMap<EType, ETypeMetaData>& RuleMetaDataManager::getETypeMap(bool forceReload)
{
    if(m_etypeMap.empty() || forceReload) {
        m_etypeMap =
        {
            {EType::Equal,           {qtTrId("rule.etype.equal")}},
            {EType::NotEqual,        {qtTrId("rule.etype.notequal")}},
            {EType::Contains,        {qtTrId("rule.etype.contains")}},
            {EType::NotContains,     {qtTrId("rule.etype.notcontains")}},
            {EType::BeginWith,       {qtTrId("rule.etype.beginwith")}},
            {EType::NotBeginWith,    {qtTrId("rule.etype.notbeginwith")}},
            {EType::EndWith,         {qtTrId("rule.etype.endwith")}},
            {EType::NotEndWith,      {qtTrId("rule.etype.notendwith")}},
            {EType::RegexMatch,      {qtTrId("rule.etype.regexmatch")}},
            {EType::RegexNotMatch,   {qtTrId("rule.etype.regexnotmatch")}},
            {EType::Greater,         {qtTrId("rule.etype.greater")}},
            {EType::GreaterEqual,    {qtTrId("rule.etype.greaterequal")}},
            {EType::Less,            {qtTrId("rule.etype.less")}},
            {EType::LessEqual,       {qtTrId("rule.etype.lessequal")}},
            {EType::NoneMatches,     {qtTrId("rule.etype.nonematches")}},
            {EType::AllMatches,      {qtTrId("rule.etype.allmatches")}},
            {EType::AnyMatches,      {qtTrId("rule.etype.anymatches")}}
        };
    }
    return m_etypeMap;
}

const QMap<Action, QString>& RuleMetaDataManager::getActionMap(bool forceReload)
{
    if(m_actionMap.empty() || forceReload) {
        m_actionMap =
        {
            {Action::Approve, qtTrId("rule.action.approve")},
            {Action::Deny, qtTrId("rule.action.deny")},
            {Action::Bypass, qtTrId("rule.action.bypass")},
            {Action::VoteUp, qtTrId("rule.action.vote_up")},
            {Action::VoteDown, qtTrId("rule.action.vote_down")},
            {Action::RequestConfirmation, qtTrId("rule.action.request_confirmation")}
        };
    }
    return m_actionMap;
}

const QMap<PermissionLevel, QString>& RuleMetaDataManager::getPermissionLevelMap(bool forceReload)
{
    if(m_permissionLevelMap.empty() || forceReload) {
        m_permissionLevelMap =
        {
            {PermissionLevel::User, qtTrId("rule.permission.user")},
            {PermissionLevel::Admin, qtTrId("rule.permission.admin")},
            {PermissionLevel::System, qtTrId("rule.permission.system")},
        };
    }
    return m_permissionLevelMap;
}

QString RuleMetaDataManager::getTypeDisplayName(Type type)
{
    auto name = getTypeMap().value(type, {QString("Unknown Type %1").arg(static_cast<uint16_t>(type))});
    return name.displayName;
}

QString RuleMetaDataManager::getETypeDisplayName(EType etype)
{
    auto name = getETypeMap().value(etype, {QString("Unknown EType %1").arg(static_cast<uint16_t>(etype))});
    return name.displayName;
}

QString RuleMetaDataManager::getActionDisplayName(Action action)
{
    return getActionMap().value(action, QString("Unknown Action %1").arg(static_cast<uint16_t>(action)));
}

QString RuleMetaDataManager::getPermissionLevelDisplayName(PermissionLevel perm)
{
    return getPermissionLevelMap().value(perm, QString("Unknown PermissionLevel %1").arg(static_cast<int>(perm)));
}

PayloadConstraint RuleMetaDataManager::getPayloadConstraint(Type type)
{
    return getTypeMap().value(type, {QString(), {false}, {PayloadConstraint::NoPayload, QString()}}).payload;
}

QString RuleMetaDataManager::payloadToDisplayText(const RuleEntry& entry)
{
    return QString::fromStdString(AutoSudoSdk::ParseRulePayload(entry));
}

bool RuleMetaDataManager::createRuleByType(Type type,
                                           EType etype,
                                           Action action,
                                           PermissionLevel allow,
                                           const QString& payloadText,
                                           QString* errorMessage)
{
    switch (type) {
    case Type::Constant:
        AutoSudoSdk::CreateConstantRule(etype, action, allow);
        return true;

    case Type::DirectoryRule:
        AutoSudoSdk::CreateDirectoryRule(etype, action, allow, payloadText.toStdWString());
        return true;

    case Type::FullPathRule:
        AutoSudoSdk::CreateFullPathRule(etype, action, allow, payloadText.toStdWString());
        return true;

    case Type::ExecutableNameRule:
        AutoSudoSdk::CreateExecutableNameRule(etype, action, allow, payloadText.toStdString());
        return true;

    case Type::StartupDirectoryRule:
        AutoSudoSdk::CreateStartupDirectoryRule(etype, action, allow, payloadText.toStdWString());
        return true;

    case Type::SidRule:
        AutoSudoSdk::CreateSidRule(etype, action, allow, payloadText.toStdString());
        return true;

    case Type::SessionRule:
        AutoSudoSdk::CreateSessionRule(etype, action, allow, payloadText.toUInt());
        return true;

    case Type::ParameterRule:
        AutoSudoSdk::CreateParameterRule(etype, action, allow, payloadText.toStdString());
        return true;

    case Type::ParametersRule:
        AutoSudoSdk::CreateParametersRule(etype, action, allow, payloadText.toStdString());
        return true;

    case Type::CustomScriptRule:
        AutoSudoSdk::CreateCustomScriptRule(etype, action, allow, payloadText.toStdWString());
        return true;

    case Type::HashRule:
        AutoSudoSdk::CreateHashRule(etype, action, allow, payloadText.toStdString());
        return true;

    case Type::VoteRule:
        AutoSudoSdk::CreateVoteRule(etype, action, allow, payloadText.toInt());
        return true;

    case Type::DigitalSignatureRule:
        AutoSudoSdk::CreateDigitalSignatureRule(etype, action, allow);
        return true;

    case Type::DateRule:
    case Type::TimeRule:
    case Type::DateTimeRule:
    case Type::FileTimeRule:
        // TODO: enable when sdk adds time-evaluate create API.
        if (errorMessage) {
            *errorMessage = QStringLiteral("time-evaluate rules are not enabled in sdk yet");
        }
        return false;

    default:
        if (errorMessage) {
            *errorMessage = QStringLiteral("invalid rule type");
        }
        return false;
    }
}

bool RuleMetaDataManager::modifyRuleByType(uint16_t uid,
                                           Type type,
                                           EType etype,
                                           Action action,
                                           PermissionLevel allow,
                                           const QString& payloadText,
                                           QString* errorMessage)
{
    switch (type) {
    case Type::Constant:
        return AutoSudoSdk::ModifyConstantRule(uid, etype, action, allow);

    case Type::DirectoryRule:
        return AutoSudoSdk::ModifyDirectoryRule(uid, etype, action, allow, payloadText.toStdWString());

    case Type::FullPathRule:
        return AutoSudoSdk::ModifyFullPathRule(uid, etype, action, allow, payloadText.toStdWString());

    case Type::ExecutableNameRule:
        return AutoSudoSdk::ModifyExecutableNameRule(uid, etype, action, allow, payloadText.toStdString());

    case Type::StartupDirectoryRule:
        return AutoSudoSdk::ModifyStartupDirectoryRule(uid, etype, action, allow, payloadText.toStdWString());

    case Type::SidRule:
        return AutoSudoSdk::ModifySidRule(uid, etype, action, allow, payloadText.toStdString());

    case Type::SessionRule:
        return AutoSudoSdk::ModifySessionRule(uid, etype, action, allow, payloadText.toUInt());

    case Type::ParameterRule:
        return AutoSudoSdk::ModifyParameterRule(uid, etype, action, allow, payloadText.toStdString());

    case Type::ParametersRule:
        return AutoSudoSdk::ModifyParametersRule(uid, etype, action, allow, payloadText.toStdString());

    case Type::CustomScriptRule:
        return AutoSudoSdk::ModifyCustomScriptRule(uid, etype, action, allow, payloadText.toStdWString());

    case Type::HashRule:
        return AutoSudoSdk::ModifyHashRule(uid, etype, action, allow, payloadText.toStdString());

    case Type::VoteRule:
        return AutoSudoSdk::ModifyVoteRule(uid, etype, action, allow, payloadText.toInt());

    case Type::DigitalSignatureRule:
        return AutoSudoSdk::ModifyDigitalSignatureRule(uid, etype, action, allow);

    case Type::DateRule:
    case Type::TimeRule:
    case Type::DateTimeRule:
    case Type::FileTimeRule:
        // TODO: enable when sdk adds time-evaluate modify API.
        if (errorMessage) {
            *errorMessage = QStringLiteral("time-evaluate rules are not enabled in sdk yet");
        }
        return false;

    default:
        if (errorMessage) {
            *errorMessage = QStringLiteral("invalid rule type");
        }
        return false;
    }
}

std::vector<std::pair<std::string, EType>> RuleMetaDataManager::getAvailableETypes(Type type)
{
    return AutoSudoSdk::getAvailableETypes(type);
}
