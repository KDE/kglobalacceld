/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KConfig>

KConfigGroup resolveGroup(KConfigBase &config, const QString &path);
QList<KConfigGroup> selectComponents(KConfigBase &config, const QString &pattern);
