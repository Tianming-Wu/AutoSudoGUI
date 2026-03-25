#ifndef RULEMETADATAMANAGER_H
#define RULEMETADATAMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

#include <AutoSudoSdk/sdk.hpp>

using AutoSudoSdk::Rule::Type;
using AutoSudoSdk::Rule::EType;
using AutoSudoSdk::Rule::Action;

// Constraints for payload values
struct PayloadConstraint {
    enum ConstraintType {
        NoPayload,      // 常量、数字签名
        String,         // 路径、目录、脚本、SID、正则等
        Integer,        // SessionRule、VoteRule
        Date,           // DateRule
        Time,           // TimeRule
        DateTime,       // DateTimeRule、FileTimeRule
        Hash            // HashRule（十六进制字符串）
    };

    ConstraintType type;
    QString hint;  // UI 提示文本
};

struct TypeMetaData {
    QString displayName;
    struct {
        bool editEnabled;
    } uicfg;
    PayloadConstraint payload;
};

struct ETypeMetaData {
    QString displayName;
};

class RuleMetaDataManager : public QObject
{
    Q_OBJECT

public:
    static RuleMetaDataManager& instance();

    const QMap<Type, TypeMetaData>& getTypeMap(bool forceReload = false);
    const QMap<EType, ETypeMetaData>& getETypeMap(bool forceReload = false);
    const QMap<Action, QString>& getActionMap(bool forceReload = false);
    const QMap<PermissionLevel, QString>& getPermissionLevelMap(bool forceReload = false);

    QString getTypeDisplayName(Type type);
    QString getETypeDisplayName(EType etype);
    QString getActionDisplayName(Action action);
    QString getPermissionLevelDisplayName(PermissionLevel perm);
    PayloadConstraint getPayloadConstraint(Type type);

    QString payloadToDisplayText(const RuleEntry& entry);
    bool createRuleByType(Type type,
                          EType etype,
                          Action action,
                          PermissionLevel allow,
                          const QString& payloadText,
                          QString* errorMessage = nullptr);
    bool modifyRuleByType(uint16_t uid,
                          Type type,
                          EType etype,
                          Action action,
                          PermissionLevel allow,
                          const QString& payloadText,
                          QString* errorMessage = nullptr);

    // Get available ETypes for a given Type
    std::vector<std::pair<std::string, EType>> getAvailableETypes(Type type);

private:
    RuleMetaDataManager();
    ~RuleMetaDataManager();

    QMap<Type, TypeMetaData> m_typeMap;
    QMap<EType, ETypeMetaData> m_etypeMap;
    QMap<Action, QString> m_actionMap;
    QMap<PermissionLevel, QString> m_permissionLevelMap;
};

#endif // RULEMETADATAMANAGER_H
