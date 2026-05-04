/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTINFOEXT_H
#define KGLOBALSHORTCUTINFOEXT_H

#include "kglobalacceld_export.h"

#include <KGlobalShortcutInfo>
#include <QString>
#include <memory>

class GlobalShortcutContext;
class KGlobalShortcutInfoExtPrivate;
class QDBusArgument;

/*!
 * A wrapper around KGlobalShortcutInfo, with additional info that wasn't part of the original D-Bus wire format.
 *
 * Ideally this information would be part of KGlobalShortcutInfo itself, but then we'd also want to
 * 1. Introduce a new D-Bus method with extended wire format into KGlobalAccel itself;
 * 2. Probably stop carrying a copy of KGlobalShortcutInfoPrivate inside kglobalacceld,
 *    and use the Builder pattern instead (as is done here);
 * 3. Make any additional types available to KGlobalAccel, so KGlobalShortcutInfo can reference it.
 *
 * This class was written with gesture triggers in mind, which live here in KGlobalAccelD.
 * So the last point is not currently feasible, until we expose trigger classes through KGlobalAccel.
 * Hence this wrapper will be private to KGlobalAccelD for now.
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

    KGlobalShortcutInfoExt(const KGlobalShortcutInfoExt &other);
    KGlobalShortcutInfoExt(KGlobalShortcutInfoExt &&other);
    KGlobalShortcutInfoExt &operator=(const KGlobalShortcutInfoExt &other);
    KGlobalShortcutInfoExt &operator=(KGlobalShortcutInfoExt &&other);
    ~KGlobalShortcutInfoExt();

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

private:
    std::unique_ptr<KGlobalShortcutInfoExtPrivate> d;
};

KGLOBALACCELD_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfoExt &shortcut);
KGLOBALACCELD_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfoExt &shortcut);

Q_DECLARE_METATYPE(KGlobalShortcutInfoExt)

#endif /* #ifndef KGLOBALSHORTCUTINFOEXT_H */
