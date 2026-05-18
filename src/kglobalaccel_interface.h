/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALACCEL_INTERFACE_H
#define KGLOBALACCEL_INTERFACE_H

#include <QObject>

#include "kglobalacceld_export.h"
#include "shortcutkeystate.h"

class GlobalShortcutsRegistry;

#define KGlobalAccelInterface_iid "org.kde.kglobalaccel5.KGlobalAccelInterface"

/**
 * Abstract interface for plugins to implement
 */
class KGLOBALACCELD_EXPORT KGlobalAccelInterface : public QObject
{
    Q_OBJECT

public:
    explicit KGlobalAccelInterface(QObject *parent);
    ~KGlobalAccelInterface() override;

public:
    void setRegistry(GlobalShortcutsRegistry *registry);

protected:
    /**
     * called by the implementation to inform us about key presses
     * @returns @c true if the key was handled
     **/
    bool keyEvent(int keyQt, ShortcutKeyState state);
    /**
     * Called by the implementation to inform us about pointer presses
     * Currently only used for clearing modifier only shortcuts
     *
     * @param buttons the buttons that were pressed
     *
     * @returns @c true if the key was handled
     */
    bool pointerPressed(Qt::MouseButtons buttons);
    /**
     * Called by the implementation to inform us about pointer axis events
     * Currently only used for clearing modifier only shortcuts
     *
     * @param axis the axis that was triggered
     *
     * @returns @c true if the key was handled
     */
    bool axisTriggered(int axis);
    /**
     * Called by the implementation to inform us about that the modifier-only state should be reset.
     * Used when we don't want to actually handle the event, but still want to reset the state.
     *
     */
    void resetModifierOnlyState();

    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_INTERFACE(KGlobalAccelInterface, KGlobalAccelInterface_iid)

#endif
