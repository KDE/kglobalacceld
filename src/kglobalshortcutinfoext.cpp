/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcutinfoext.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QVariantMap>

using namespace Qt::StringLiterals;

struct KGlobalShortcutInfoExtPrivate {
    KGlobalShortcutInfoExtPrivate() = default;
    KGlobalShortcutInfoExtPrivate(const KGlobalShortcutInfoExtPrivate &) = default;

    KGlobalShortcutInfoExtPrivate(KGlobalShortcutInfo &&info)
        : m_info(std::move(info))
    {
    }

    KGlobalShortcutInfo m_info;
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

//
// D-Bus wire format

QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfoExt &shortcut)
{
    QVariantMap extInfo{
        {"inverseAction"_L1, shortcut.inverseAction()},
    };

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

    shortcut = builder.build();
    return argument;
}
