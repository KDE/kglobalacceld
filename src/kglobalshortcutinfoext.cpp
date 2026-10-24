/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcutinfoext.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QHash>
#include <QSet>
#include <QVariantMap>

using namespace Qt::StringLiterals;

namespace
{
struct TriggerSets {
    QSet<KGlobalShortcutTrigger> defaults;
    QSet<KGlobalShortcutTrigger> assigned;
};
}

struct KGlobalShortcutInfoExtPrivate {
    KGlobalShortcutInfoExtPrivate() = default;
    KGlobalShortcutInfoExtPrivate(const KGlobalShortcutInfoExtPrivate &) = default;

    KGlobalShortcutInfoExtPrivate(KGlobalShortcutInfo &&info)
        : m_info(std::move(info))
    {
    }

    KGlobalShortcutInfo m_info;
    QHash<QString, TriggerSets> m_triggersByType;
    QString m_inverseActionUniqueName;
};

KGlobalShortcutInfoExt::KGlobalShortcutInfoExt()
    : d(new KGlobalShortcutInfoExtPrivate())
{
}

KGlobalShortcutInfoExt::KGlobalShortcutInfoExt(KGlobalShortcutInfoExtPrivate *d)
    : d(d)
{
}

KGlobalShortcutInfoExt::KGlobalShortcutInfoExt(const KGlobalShortcutInfoExt &other)
    : d(new KGlobalShortcutInfoExtPrivate(*other.d))
{
}

KGlobalShortcutInfoExt::KGlobalShortcutInfoExt(KGlobalShortcutInfoExt &&other)
    : d(other.d.release())
{
}

KGlobalShortcutInfoExt &KGlobalShortcutInfoExt::operator=(const KGlobalShortcutInfoExt &other)
{
    d.reset(new KGlobalShortcutInfoExtPrivate(*other.d));
    return *this;
}

KGlobalShortcutInfoExt &KGlobalShortcutInfoExt::operator=(KGlobalShortcutInfoExt &&other)
{
    d = std::move(other.d);
    return *this;
}

KGlobalShortcutInfoExt::~KGlobalShortcutInfoExt()
{
}

const KGlobalShortcutInfo &KGlobalShortcutInfoExt::info() const
{
    return d->m_info;
}

QString KGlobalShortcutInfoExt::inverseAction() const
{
    return d->m_inverseActionUniqueName;
}

QStringList KGlobalShortcutInfoExt::triggerTypes() const
{
    return d->m_triggersByType.keys();
}

QSet<KGlobalShortcutTrigger> KGlobalShortcutInfoExt::triggers(const QString &triggerType) const
{
    return d->m_triggersByType.value(triggerType, TriggerSets{}).assigned;
}

QSet<KGlobalShortcutTrigger> KGlobalShortcutInfoExt::defaultTriggers(const QString &triggerType) const
{
    return d->m_triggersByType.value(triggerType, TriggerSets{}).defaults;
}

//
// Builder

KGlobalShortcutInfoExtBuilder::KGlobalShortcutInfoExtBuilder(KGlobalShortcutInfo &&info)
    : d(new KGlobalShortcutInfoExtPrivate(std::move(info)))
{
}

KGlobalShortcutInfoExtBuilder::~KGlobalShortcutInfoExtBuilder()
{
}

KGlobalShortcutInfoExt KGlobalShortcutInfoExtBuilder::build()
{
    return KGlobalShortcutInfoExt(d.release());
}

KGlobalShortcutInfoExtBuilder &KGlobalShortcutInfoExtBuilder::setInverseAction(const QString &inverseActionUniqueName)
{
    d->m_inverseActionUniqueName = inverseActionUniqueName;
    return *this;
}

KGlobalShortcutInfoExtBuilder &KGlobalShortcutInfoExtBuilder::setTriggers(const QString &triggerType,
                                                                          const QSet<KGlobalShortcutTrigger> &assignedTriggers,
                                                                          const QSet<KGlobalShortcutTrigger> &defaultTriggers)
{
    auto &triggerSets = d->m_triggersByType[triggerType];
    triggerSets.assigned = assignedTriggers;
    triggerSets.defaults = defaultTriggers;
    return *this;
}

//
// D-Bus wire format

QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfoExt &shortcut)
{
    const QStringList triggerTypes = shortcut.triggerTypes();

    QVariantMap extInfo{
        {"inverseAction"_L1, shortcut.inverseAction()},
        {"triggerTypes"_L1, triggerTypes},
    };

    QStringList triggerParamStrings;
    for (const QString &triggerType : triggerTypes) {
        const QSet<KGlobalShortcutTrigger> assignedTriggers = shortcut.triggers(triggerType);
        const QSet<KGlobalShortcutTrigger> defaultTriggers = shortcut.defaultTriggers(triggerType);

        triggerParamStrings.clear();
        for (const KGlobalShortcutTrigger &trigger : assignedTriggers) {
            triggerParamStrings.append(trigger.paramString());
        }
        extInfo.insert("triggersAssigned-"_L1 + triggerType, triggerParamStrings);

        triggerParamStrings.clear();
        for (const KGlobalShortcutTrigger &trigger : defaultTriggers) {
            triggerParamStrings.append(trigger.paramString());
        }
        extInfo.insert("triggersDefault-"_L1 + triggerType, triggerParamStrings);
    }

    argument.beginStructure();
    argument << shortcut.info() << extInfo;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfoExt &shortcut)
{
    KGlobalShortcutInfo info;
    QVariantMap extInfo;

    argument.beginStructure();
    argument >> info >> extInfo;
    argument.endStructure();

    KGlobalShortcutInfoExtBuilder builder(std::move(info));

    if (const QString inverseAction = extInfo.value("inverseAction"_L1).toString(); !inverseAction.isEmpty()) {
        builder.setInverseAction(inverseAction);
    }

    QSet<KGlobalShortcutTrigger> assignedTriggers;
    QSet<KGlobalShortcutTrigger> defaultTriggers;
    for (const QString &triggerType : extInfo.value("triggerTypes"_L1).toStringList()) {
        assignedTriggers.clear();
        for (const QString &triggerParamString : extInfo.value("triggersAssigned-"_L1 + triggerType).toStringList()) {
            assignedTriggers.insert(KGlobalShortcutTrigger(triggerType, triggerParamString));
        }
        defaultTriggers.clear();
        for (const QString &triggerParamString : extInfo.value("triggersDefault-"_L1 + triggerType).toStringList()) {
            defaultTriggers.insert(KGlobalShortcutTrigger(triggerType, triggerParamString));
        }
        builder.setTriggers(triggerType, assignedTriggers, defaultTriggers);
    }

    shortcut = builder.build();
    return argument;
}
