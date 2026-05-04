/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcutinfoext.h"

#include <QDBusArgument>
#include <QDBusMetaType>

struct KGlobalShortcutInfoExtPrivate {
public:
    KGlobalShortcutInfoExtPrivate() = default;
    KGlobalShortcutInfoExtPrivate(const KGlobalShortcutInfoExtPrivate &) = default;

    KGlobalShortcutInfoExtPrivate(KGlobalShortcutInfo &&info)
        : m_info(std::move(info))
    {
    }

    KGlobalShortcutInfo m_info;
    QHash<QString, KGlobalShortcutInfoExt::TriggerSets> m_triggersByType;
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

QHash<QString, KGlobalShortcutInfoExt::TriggerSets> KGlobalShortcutInfoExt::allTriggersByType() const
{
    return d->m_triggersByType;
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

QDBusArgument &operator<<(QDBusArgument &argument, const QHash<QString, KGlobalShortcutInfoExt::TriggerSets> &triggersByType)
{
    QPair<QStringList, QStringList> triggerParamStringsPair;
    QStringList &assignedTriggerParamStrings = triggerParamStringsPair.first;
    QStringList &defaultTriggerParamStrings = triggerParamStringsPair.second;

    argument.beginMap(QMetaType::fromType<QString>(), QMetaType::fromType<std::pair<QStringList, QStringList>>());
    for (const auto [triggerType, triggerSets] : triggersByType.asKeyValueRange()) {
        assignedTriggerParamStrings.clear();
        defaultTriggerParamStrings.clear();
        for (const KGlobalShortcutTrigger &trigger : triggerSets.assigned) {
            assignedTriggerParamStrings.append(trigger.paramString());
        }
        for (const KGlobalShortcutTrigger &trigger : triggerSets.defaults) {
            defaultTriggerParamStrings.append(trigger.paramString());
        }
        argument.beginMapEntry();
        argument << triggerType << triggerParamStringsPair;
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QHash<QString, KGlobalShortcutInfoExt::TriggerSets> &triggersByType)
{
    triggersByType.clear();
    KGlobalShortcutInfoExt::TriggerSets triggerSets;

    argument.beginMap();
    while (!argument.atEnd()) {
        QString triggerType;
        QStringList assignedTriggerParamStrings;
        QStringList defaultTriggerParamStrings;
        argument.beginMapEntry();
        argument >> triggerType;
        argument.beginStructure();
        argument >> assignedTriggerParamStrings >> defaultTriggerParamStrings;
        argument.endStructure();
        argument.endMapEntry();

        triggerSets.assigned.clear();
        triggerSets.defaults.clear();
        for (const QString &triggerParamString : assignedTriggerParamStrings) {
            triggerSets.assigned.insert(KGlobalShortcutTrigger(triggerType, triggerParamString));
        }
        for (const QString &triggerParamString : defaultTriggerParamStrings) {
            triggerSets.defaults.insert(KGlobalShortcutTrigger(triggerType, triggerParamString));
        }
        triggersByType.insert(triggerType, triggerSets);
    }
    argument.endMap();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfoExt &shortcut)
{
    const QHash<QString, KGlobalShortcutInfoExt::TriggerSets> allTriggers(shortcut.allTriggersByType());
    const uint flags = 0;
    argument.beginStructure();
    argument << shortcut.info() << allTriggers << shortcut.inverseAction() << flags;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfoExt &shortcut)
{
    KGlobalShortcutInfo info;
    QHash<QString, KGlobalShortcutInfoExt::TriggerSets> allTriggers;
    QString inverseAction;
    uint flags;
    argument.beginStructure();
    argument >> info >> allTriggers >> inverseAction >> flags;
    argument.endStructure();

    KGlobalShortcutInfoExtBuilder builder(std::move(info));
    for (const auto [triggerType, triggerSets] : allTriggers.asKeyValueRange()) {
        builder.setTriggers(triggerType, triggerSets.assigned, triggerSets.defaults);
    }
    builder.setInverseAction(inverseAction);
    shortcut = builder.build();
    return argument;
}
