/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/** @file
 * Autotests for allow-list functionality.
 *
 * Verifies that when the allow-list is enabled, only shortcuts
 * specified in the allow-list are activated.
 */

#include "dummy.h"
#include "kglobalacceld.h"

#include <QDBusConnection>
#include <QPluginLoader>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

Q_IMPORT_PLUGIN(KGlobalAccelImpl)

/**
 * Timeout for waiting for signals in tests (in milliseconds).
 */
constexpr int SIGNAL_TIMEOUT_MS = 150;

class AllowListTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testAllowList_data();
    void testAllowList();
    void testAllowListMultipleActions();
};

void AllowListTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setApplicationName(QStringLiteral("allowlisttest"));
    QCoreApplication::setOrganizationName(QStringLiteral("kde"));
    qputenv("KGLOBALACCELD_PLATFORM", "dummy");
}

void AllowListTest::testAllowList_data()
{
    QTest::addColumn<bool>("useAllowList");
    QTest::addColumn<bool>("listAction");
    QTest::addColumn<bool>("expectTriggered");

    QTest::newRow("allowlist disabled") << false << false << true;
    QTest::newRow("allowlist enabled, not listed") << true << false << false;
    QTest::newRow("allowlist enabled, listed") << true << true << true;
}

void AllowListTest::testAllowList()
{
    QFETCH(bool, useAllowList);
    QFETCH(bool, listAction);
    QFETCH(bool, expectTriggered);

    const QString componentName = qApp->applicationName();
    const QString actionName = QStringLiteral("AllowListTestAction");

    // Prepare clean config dir and allow-list config before daemon init so it is loaded on startup.
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configDir);
    QFile::remove(configDir + QLatin1String("/kglobalaccelrc"));
    QFile::remove(configDir + QLatin1String("/kglobalshortcutsrc"));

    QFile config(configDir + QLatin1String("/kglobalaccelrc"));
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QTextStream stream(&config);
    stream << "[General]\n";
    stream << "useAllowList=" << (useAllowList ? "true" : "false") << "\n";
    if (useAllowList) {
        stream << "[AllowedShortcuts]\n";
        if (listAction) {
            stream << componentName << '=' << actionName << "\n";
        }
    }
    config.close();

    // Ensure the DBus name is free before each init attempt.
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.kde.kglobalaccel"));

    auto daemon = std::make_unique<KGlobalAccelD>();
    QVERIFY(daemon->init());

    KGlobalAccelImpl *interface = KGlobalAccelImpl::instance();
    QVERIFY(interface);

    auto action = std::make_unique<QAction>();
    action->setObjectName(actionName);

    const QKeySequence shortcut(Qt::CTRL | Qt::Key_L);
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcut));

    QSignalSpy spy(action.get(), &QAction::triggered);

    interface->checkKeyEvent(shortcut[0].toCombined(), ShortcutKeyState::Pressed);
    interface->checkKeyEvent(shortcut[0].toCombined(), ShortcutKeyState::Released);

    if (expectTriggered) {
        QVERIFY(spy.wait());
        QCOMPARE(spy.count(), 1);
    } else {
        QVERIFY(!spy.wait(SIGNAL_TIMEOUT_MS));
        QCOMPARE(spy.count(), 0);
    }

    KGlobalAccel::self()->removeAllShortcuts(action.get());

    daemon.reset();
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.kde.kglobalaccel"));
}

void AllowListTest::testAllowListMultipleActions()
{
    const QString componentName = qApp->applicationName();
    const QStringList allowedActions = {QStringLiteral("AllowListTestActionOne"), QStringLiteral("AllowListTestActionTwo")};
    const QString disallowedAction = QStringLiteral("AllowListTestActionThree");

    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configDir);
    QFile::remove(configDir + QLatin1String("/kglobalaccelrc"));
    QFile::remove(configDir + QLatin1String("/kglobalshortcutsrc"));

    QFile config(configDir + QLatin1String("/kglobalaccelrc"));
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QTextStream stream(&config);
    stream << "[General]\n";
    stream << "useAllowList=true\n";
    stream << "[AllowedShortcuts]\n";
    stream << componentName << '=' << allowedActions.join(QLatin1Char(',')) << "\n";
    config.close();

    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.kde.kglobalaccel"));

    auto daemon = std::make_unique<KGlobalAccelD>();
    QVERIFY(daemon->init());

    KGlobalAccelImpl *interface = KGlobalAccelImpl::instance();
    QVERIFY(interface);

    auto actionOne = std::make_unique<QAction>();
    actionOne->setObjectName(allowedActions.at(0));
    auto actionTwo = std::make_unique<QAction>();
    actionTwo->setObjectName(allowedActions.at(1));
    auto actionThree = std::make_unique<QAction>();
    actionThree->setObjectName(disallowedAction);

    // Use layout-independent keys to avoid CI flakiness on varying keymaps.
    const QKeySequence shortcutOne(Qt::CTRL | Qt::Key_A);
    const QKeySequence shortcutTwo(Qt::CTRL | Qt::Key_B);
    const QKeySequence shortcutThree(Qt::CTRL | Qt::Key_C);
    QVERIFY(KGlobalAccel::setGlobalShortcut(actionOne.get(), shortcutOne));
    QVERIFY(KGlobalAccel::setGlobalShortcut(actionTwo.get(), shortcutTwo));
    QVERIFY(KGlobalAccel::setGlobalShortcut(actionThree.get(), shortcutThree));

    QSignalSpy spyOne(actionOne.get(), &QAction::triggered);
    QSignalSpy spyTwo(actionTwo.get(), &QAction::triggered);
    QSignalSpy spyThree(actionThree.get(), &QAction::triggered);

    const auto triggerShortcut = [interface](const QKeySequence &sequence) {
        interface->checkKeyEvent(sequence[0].toCombined(), ShortcutKeyState::Pressed);
        interface->checkKeyEvent(sequence[0].toCombined(), ShortcutKeyState::Released);
    };

    // Allow event loop to register shortcuts before injecting key events.
    QCoreApplication::processEvents();

    triggerShortcut(shortcutOne);
    triggerShortcut(shortcutTwo);
    triggerShortcut(shortcutThree);

    QTRY_COMPARE_WITH_TIMEOUT(spyOne.count(), 1, 2000);
    QTRY_COMPARE_WITH_TIMEOUT(spyTwo.count(), 1, 2000);
    QVERIFY(!spyThree.wait(SIGNAL_TIMEOUT_MS));
    QCOMPARE(spyThree.count(), 0);

    KGlobalAccel::self()->removeAllShortcuts(actionOne.get());
    KGlobalAccel::self()->removeAllShortcuts(actionTwo.get());
    KGlobalAccel::self()->removeAllShortcuts(actionThree.get());

    daemon.reset();
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.kde.kglobalaccel"));
}

QTEST_MAIN(AllowListTest)

#include "allowlisttest.moc"
