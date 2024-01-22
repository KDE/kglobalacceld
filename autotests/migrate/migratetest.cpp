/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QProcess>
#include <QStandardPaths>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

class MigrateConfigTest : public QObject
{
    Q_OBJECT

public:
    MigrateConfigTest()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

private Q_SLOTS:
    void init()
    {
        QDir configDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
        configDir.mkpath(QStringLiteral("."));
        configDir.remove(QStringLiteral("kglobalshortcutsrc"));
    }

    void moveGroup_data()
    {
        QTest::addColumn<QString>("initialConfig");
        QTest::addColumn<QString>("resultConfig");
        QTest::addColumn<QString>("sourceComponent");
        QTest::addColumn<QString>("targetComponent");

        QTest::addRow("normal") << QStringLiteral("move-group") << QStringLiteral("move-group.expected") << QStringLiteral("/org.kde.foo.desktop")
                                << QStringLiteral("/services/org.kde.foo.desktop");
        QTest::addRow("reset") << QStringLiteral("move-group-reset") << QStringLiteral("move-group-reset.expected") << QStringLiteral("/org.kde.foo.desktop")
                               << QStringLiteral("/services/org.kde.foo.desktop");
        QTest::addRow("regex") << QStringLiteral("move-group-regex") << QStringLiteral("move-group-regex.expected") << QStringLiteral("/*.desktop")
                               << QStringLiteral("/services/");
        QTest::addRow("non-existent") << QStringLiteral("move-group") << QStringLiteral("move-group") << QStringLiteral("/org.kde.foo42.desktop")
                                      << QStringLiteral("/services/org.kde.foo42.desktop");
        QTest::addRow("rename") << QStringLiteral("move-group-rename") << QStringLiteral("move-group-rename.expected") << QStringLiteral("/org.kde.foo.desktop")
                                << QStringLiteral("/org.kde.buf.desktop");
        QTest::addRow("self") << QStringLiteral("move-group") << QStringLiteral("move-group") << QStringLiteral("/org.kde.foo.desktop")
                              << QStringLiteral("/org.kde.foo.desktop");
    }

    void moveGroup()
    {
        QFETCH(QString, initialConfig);
        QFETCH(QString, resultConfig);
        QFETCH(QString, sourceComponent);
        QFETCH(QString, targetComponent);

        const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kglobalshortcutsrc");
        QFile::copy(QFINDTESTDATA(initialConfig), configFileName);

        run({
            QStringLiteral("move"),
            QStringLiteral("--config=") + configFileName,
            QStringLiteral("--source-component=") + sourceComponent,
            QStringLiteral("--target-component=") + targetComponent,
        });

        // Compare actual with expected config
        KConfig actual(QStringLiteral("kglobalshortcutsrc"));
        KConfig expected(QFINDTESTDATA(resultConfig));

        compareGroupList(actual, expected);
        compareGroupList(actual.group(QStringLiteral("services")), expected.group(QStringLiteral("services")));
    }

    void moveAction_data()
    {
        QTest::addColumn<QString>("initialConfig");
        QTest::addColumn<QString>("resultConfig");
        QTest::addColumn<QString>("sourceComponent");
        QTest::addColumn<QString>("sourceAction");
        QTest::addColumn<QString>("targetComponent");
        QTest::addColumn<QString>("targetAction");

        QTest::addRow("normal") << QStringLiteral("move-action") << QStringLiteral("move-action.expected") << QStringLiteral("/org.kde.foo.desktop")
                                << QStringLiteral("Toggle") << QStringLiteral("/org.kde.bar.desktop") << QString();
        QTest::addRow("new-component") << QStringLiteral("move-action-new-component") << QStringLiteral("move-action-new-component.expected")
                                       << QStringLiteral("/org.kde.foo.desktop") << QStringLiteral("Toggle") << QStringLiteral("/org.kde.bar.desktop")
                                       << QString();
        QTest::addRow("non-existent") << QStringLiteral("move-action") << QStringLiteral("move-action") << QStringLiteral("/org.kde.foo42.desktop")
                                      << QStringLiteral("Toggle") << QStringLiteral("/org.kde.bar.desktop") << QString();
        QTest::addRow("rename") << QStringLiteral("move-action-rename") << QStringLiteral("move-action-rename.expected")
                                << QStringLiteral("/org.kde.foo.desktop") << QStringLiteral("Toggle") << QStringLiteral("/org.kde.bar.desktop")
                                << QStringLiteral("Start");
        QTest::addRow("self") << QStringLiteral("move-action") << QStringLiteral("move-action") << QStringLiteral("/org.kde.foo.desktop")
                              << QStringLiteral("Toggle") << QStringLiteral("/org.kde.foo.desktop") << QStringLiteral("Toggle");
    }

    void moveAction()
    {
        QFETCH(QString, initialConfig);
        QFETCH(QString, resultConfig);
        QFETCH(QString, sourceComponent);
        QFETCH(QString, sourceAction);
        QFETCH(QString, targetComponent);
        QFETCH(QString, targetAction);

        const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kglobalshortcutsrc");
        QFile::copy(QFINDTESTDATA(initialConfig), configFileName);

        run({
            QStringLiteral("move"),
            QStringLiteral("--config=") + configFileName,
            QStringLiteral("--source-component=") + sourceComponent,
            QStringLiteral("--source-action=") + sourceAction,
            QStringLiteral("--target-component=") + targetComponent,
            QStringLiteral("--target-action=") + targetAction,
        });

        // Compare actual with expected config
        KConfig actual(QStringLiteral("kglobalshortcutsrc"));
        KConfig expected(QFINDTESTDATA(resultConfig));

        compareGroupList(actual, expected);
        compareGroupList(actual.group(QStringLiteral("services")), expected.group(QStringLiteral("services")));
    }

    void moveToDesktopFile_data()
    {
        QTest::addColumn<QString>("initialConfig");
        QTest::addColumn<QString>("resultConfig");
        QTest::addColumn<QString>("sourceComponent");
        QTest::addColumn<QString>("sourceAction");
        QTest::addColumn<QString>("targetDesktopFile");
        QTest::addColumn<QString>("targetDesktopFileAction");

        QTest::addRow("launch") << QStringLiteral("move-to-desktop-launch") << QStringLiteral("move-to-desktop-launch.expected") << QStringLiteral("/kded5")
                                << QStringLiteral("Display") << QStringLiteral("org.kde.test.desktop") << QString();
        QTest::addRow("action") << QStringLiteral("move-to-desktop-action") << QStringLiteral("move-to-desktop-action.expected") << QStringLiteral("/kded5")
                                << QStringLiteral("Display") << QStringLiteral("org.kde.test.desktop") << QStringLiteral("dotest");
    }

    void moveToDesktopFile()
    {
        QFETCH(QString, initialConfig);
        QFETCH(QString, resultConfig);
        QFETCH(QString, sourceComponent);
        QFETCH(QString, sourceAction);
        QFETCH(QString, targetDesktopFile);
        QFETCH(QString, targetDesktopFileAction);

        const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kglobalshortcutsrc");
        QFile::copy(QFINDTESTDATA(initialConfig), configFileName);

        run({
            QStringLiteral("move"),
            QStringLiteral("--config=") + configFileName,
            QStringLiteral("--source-component=") + sourceComponent,
            QStringLiteral("--source-action=") + sourceAction,
            QStringLiteral("--target-desktop-file=") + QFINDTESTDATA(targetDesktopFile),
            QStringLiteral("--target-desktop-file-action=") + targetDesktopFileAction,
        });

        // Compare actual with expected config
        KConfig actual(QStringLiteral("kglobalshortcutsrc"));
        KConfig expected(QFINDTESTDATA(resultConfig));

        compareGroupList(actual, expected);
        compareGroupList(actual.group(QStringLiteral("services")), expected.group(QStringLiteral("services")));
    }

private:
    void compareGroups(const KConfigGroup &a, const KConfigGroup &b)
    {
        for (auto [key, value] : a.entryMap().asKeyValueRange()) {
            QCOMPARE(value, b.readEntry(key));
        }

        for (auto [key, value] : b.entryMap().asKeyValueRange()) {
            QCOMPARE(value, a.readEntry(key));
        }
    }

    void compareGroupList(const KConfigBase &a, const KConfigBase &b)
    {
        QStringList sortedA = a.groupList();
        sortedA.sort();

        QStringList sortedB = b.groupList();
        sortedB.sort();

        QCOMPARE(sortedA, sortedB);

        const QStringList groups = a.groupList();
        for (const QString &group : groups) {
            compareGroups(a.group(group), b.group(group));
        }
    }

    void run(const QStringList &arguments)
    {
        QProcess process;
        process.setProcessChannelMode(QProcess::ForwardedChannels);
        process.setProgram(QCoreApplication::applicationDirPath() + QLatin1String("/kglobalaccel-migrate"));
        process.setArguments(arguments);
        process.start();
        process.waitForFinished();
    }
};

QTEST_MAIN(MigrateConfigTest)

#include "migratetest.moc"
