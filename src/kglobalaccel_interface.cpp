/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalaccel_interface.h"
#include "globalshortcutsregistry.h"

class KGlobalAccelInterface::Private
{
public:
    GlobalShortcutsRegistry *owner;
};

KGlobalAccelInterface::KGlobalAccelInterface()
    : QObject()
    , d(new Private)
{
}

KGlobalAccelInterface::~KGlobalAccelInterface() = default;

bool KGlobalAccelInterface::setTriggerActive(const KGlobalShortcutTrigger &, bool, const QString &, const QString &, const QString &, const QString &)
{
    return false;
}

void KGlobalAccelInterface::setRegistry(GlobalShortcutsRegistry *registry)
{
    d->owner = registry;
}

bool KGlobalAccelInterface::keyEvent(int keyQt, ShortcutKeyState state)
{
    return d->owner->keyEvent(keyQt, state);
}

bool KGlobalAccelInterface::pointerPressed(Qt::MouseButtons buttons)
{
    return d->owner->pointerPressed(buttons);
}

bool KGlobalAccelInterface::axisTriggered(int axis)
{
    return d->owner->axisTriggered(axis);
}

void KGlobalAccelInterface::resetModifierOnlyState()
{
    d->owner->resetModifierOnlyState();
}

#include "moc_kglobalaccel_interface.cpp"
