/*
    SPDX-FileCopyrightText: 2026 Kanishk Ranjan <kanishkranjan17@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "component.h"

#include <QTest>

class ComponentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSanitizeDBusPathSegment_data()
    {
        QTest::addColumn<QString>("uniqueName");
        QTest::addColumn<QString>("expectedSegment");

        QTest::newRow("ascii") << QStringLiteral("org.kde.konsole.desktop") << QStringLiteral("org_kde_konsole_desktop");
        QTest::newRow("underscore kept") << QStringLiteral("my_app.desktop") << QStringLiteral("my_app_desktop");
        QTest::newRow("latin-1") << QStringLiteral("vlc-bébé.desktop") << QStringLiteral("vlc_b_b__desktop");
        // Low byte of U+0161 is 'a' and of U+0141 is 'A': these must not slip through (bug 407139)
        QTest::newRow("low byte looks like ascii") << QStringLiteral("šablona.desktop") << QStringLiteral("_ablona_desktop");
        QTest::newRow("polish") << QStringLiteral("Łódź.desktop") << QStringLiteral("__d__desktop");
        QTest::newRow("cjk") << QStringLiteral("测试.desktop") << QStringLiteral("___desktop");
    }

    void testSanitizeDBusPathSegment()
    {
        QFETCH(QString, uniqueName);
        QFETCH(QString, expectedSegment);

        QCOMPARE(Component::sanitizeDBusPathSegment(uniqueName), expectedSegment);
    }
};

QTEST_APPLESS_MAIN(ComponentTest)

#include "componenttest.moc"
