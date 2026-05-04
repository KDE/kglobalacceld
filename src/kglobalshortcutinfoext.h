/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTINFOEXT_H
#define KGLOBALSHORTCUTINFOEXT_H

#include "kglobalacceld_export.h"

#include "kglobalshortcuttrigger.h"

#include <KGlobalShortcutInfo>
#include <QHash>
#include <QSet>
#include <QString>
#include <memory>

class GlobalShortcutContext;
class KGlobalShortcutInfoExtPrivate;
class QDBusArgument;

/*!
 * A wrapper around KGlobalShortcutInfo, with additional info such as non-keyboard triggers.
 *
 * Ideally this information would be part of KGlobalShortcutInfo itself, but then we'd also want to
 * (1) move KGlobalShortcutTrigger to KGlobalAccel, so KGlobalShortcutInfo can reference it;
 * (2) introduce a new D-Bus method with extended wire format into KGlobalAccel itself;
 * (3) probably stop carrying a copy of KGlobalShortcutInfoPrivate inside kglobalacceld,
 * and use Builder pattern instead.
 */
class KGLOBALACCELD_EXPORT KGlobalShortcutInfoExt
{
public:
    KGlobalShortcutInfoExt();
    KGlobalShortcutInfoExt(KGlobalShortcutInfoExtPrivate *d);

    //! Returns the base shortcut info object, with all its property accessors.
    const KGlobalShortcutInfo &info() const;

    //! Returns the unique name of the shortcut's inverse action within the same component, or an empty string if none is set.
    QString inverseAction() const;

    //! Returns a list of trigger types for which either assigned or default triggers (or both) exist.
    QStringList triggerTypes() const;

    //! Returns a list of currently assigned triggers of \a triggerType associated with this shortcut.
    QSet<KGlobalShortcutTrigger> triggers(const QString &triggerType) const;

    //! Returns a list of default triggers of \a triggerType associated with this shortcut.
    QSet<KGlobalShortcutTrigger> defaultTriggers(const QString &triggerType) const;

    KGlobalShortcutInfoExt(const KGlobalShortcutInfoExt &other);
    KGlobalShortcutInfoExt(KGlobalShortcutInfoExt &&other);
    KGlobalShortcutInfoExt &operator=(const KGlobalShortcutInfoExt &other);
    KGlobalShortcutInfoExt &operator=(KGlobalShortcutInfoExt &&other);
    ~KGlobalShortcutInfoExt();

    // for D-Bus marshalling (use a different wire format than the default one from Qt)
    struct TriggerSets {
        QSet<KGlobalShortcutTrigger> defaults;
        QSet<KGlobalShortcutTrigger> assigned;
    };
    QHash<QString, TriggerSets> allTriggersByType() const;

private:
    friend class KGlobalShortcutInfoExtBuilder;

    std::unique_ptr<KGlobalShortcutInfoExtPrivate> d;
};

class KGlobalShortcutInfoExtBuilder
{
public:
    KGlobalShortcutInfoExtBuilder(KGlobalShortcutInfo &&info);
    ~KGlobalShortcutInfoExtBuilder();

    KGlobalShortcutInfoExt build();

    KGlobalShortcutInfoExtBuilder &setInverseAction(const QString &inverseActionUniqueName);

    KGlobalShortcutInfoExtBuilder &setTriggers(const QString &triggerType, //
                                               const QSet<KGlobalShortcutTrigger> &assignedTriggers,
                                               const QSet<KGlobalShortcutTrigger> &defaultTriggers);

private:
    std::unique_ptr<KGlobalShortcutInfoExtPrivate> d;
};

KGLOBALACCELD_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfoExt &shortcut);
KGLOBALACCELD_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfoExt &shortcut);

KGLOBALACCELD_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const QHash<QString, KGlobalShortcutInfoExt::TriggerSets> &triggersByType);
KGLOBALACCELD_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, QHash<QString, KGlobalShortcutInfoExt::TriggerSets> &triggersByType);

Q_DECLARE_METATYPE(KGlobalShortcutInfoExt)

#endif /* #ifndef KGLOBALSHORTCUTINFOEXT_H */
