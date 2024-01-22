/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "match.h"

#include <KConfigGroup>

#include <QRegularExpression>

KConfigGroup resolveGroup(KConfigBase &config, const QString &path)
{
    const QStringList parts = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    KConfigGroup ret = config.group(parts.first());
    for (int i = 1; i < parts.size(); ++i) {
        ret = ret.group(parts[i]);
    }
    return ret;
}

static QList<KConfigGroup> selectComponents(const QList<KConfigGroup> &pool, const QStringList &patterns, int index = 0)
{
    if (pool.isEmpty()) {
        return QList<KConfigGroup>();
    }

    QList<KConfigGroup> next;
    const QRegularExpression expression(QRegularExpression::wildcardToRegularExpression(patterns[index]));
    for (const KConfigGroup &candidate : pool) {
        if (expression.match(candidate.name()).hasMatch()) {
            next.append(candidate);
        }
    }

    if (patterns.size() - 1 == index) {
        return next;
    }

    QList<KConfigGroup> children;
    for (const KConfigGroup &cg : next) {
        const QStringList groupNames = cg.groupList();
        for (const QString &groupName : groupNames) {
            children.append(cg.group(groupName));
        }
    }

    return selectComponents(children, patterns, index + 1);
}

QList<KConfigGroup> selectComponents(KConfigBase &config, const QString &pattern)
{
    const QStringList parts = pattern.split(QLatin1Char('/'), Qt::SkipEmptyParts);

    QList<KConfigGroup> candidates;
    const QStringList groupNames = config.groupList();
    for (const QString &groupName : groupNames) {
        candidates.append(config.group(groupName));
    }

    return selectComponents(candidates, parts);
}
