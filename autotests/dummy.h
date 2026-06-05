/*
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DUMMY_H
#define DUMMY_H

#include "kglobalaccel_interface.h"

#include <QObject>

class KGlobalAccelImpl : public KGlobalAccelInterface
{
    Q_OBJECT

public:
    KGlobalAccelImpl();

    static KGlobalAccelImpl *instance();

public Q_SLOTS:
    bool checkKeyEvent(int keyQt, ShortcutKeyState state);
    bool checkPointerPressed(Qt::MouseButtons button);
    bool checkAxisTriggered(int axis);
};

#endif // DUMMY_H
