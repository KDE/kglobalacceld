/*
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dummy.h"

//----------------------------------------------------

static KGlobalAccelImpl *s_interface = nullptr;

KGlobalAccelImpl *KGlobalAccelImpl::instance()
{
    return s_interface;
}

KGlobalAccelImpl::KGlobalAccelImpl(QObject *parent)
    : KGlobalAccelInterface(parent)
{
}

KGlobalAccelImpl::~KGlobalAccelImpl()
{
}

bool KGlobalAccelImpl::grabKey(int keyQt, bool grab)
{
    Q_UNUSED(keyQt);
    Q_UNUSED(grab);
    return true;
}

void KGlobalAccelImpl::setEnabled(bool enable)
{
    s_interface = enable ? this : nullptr;
}

bool KGlobalAccelImpl::checkKeyEvent(int keyQt, ShortcutKeyState state)
{
    return keyEvent(keyQt, state);
}

bool KGlobalAccelImpl::checkPointerPressed(Qt::MouseButtons button)
{
    return pointerPressed(button);
}

bool KGlobalAccelImpl::checkAxisTriggered(int axis)
{
    return axisTriggered(axis);
}

#include "moc_dummy.cpp"
