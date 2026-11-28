/*
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "component.h"
#include "kglobalacceld.h"

#include <QFile>
#include <QPluginLoader>
#include <QSignalSpy>
#include <QStandardPaths>

class ShortcutsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testShortcuts_data();
    void testShortcuts();
    void testSerialization();
    void testContestedKeys();
    void testBacktab_data();
    void testBacktab();
    void testEmptyShortcut_data();
    void testEmptyShortcut();
    void testHeldShortcut_data();
    void testHeldShortcut();
    void testBacktabRelease_data();
    void testBacktabRelease();
    void testShortcutDuringHeldShortcut_data();
    void testShortcutDuringHeldShortcut();
    void testOverlappingHeldShortcuts();
    void testShortcutWithTwoHeldKeys_data();
    void testShortcutWithTwoHeldKeys();
    void testHeldShortcutUnregistered();
    void testLostRelease();
    void testLostReleaseWithoutModifier();

private:
    void sendKeyCombination(QKeyCombination keyCombination, ShortcutKeyState state);
    void sendKeyCombinationPressAndRelease(QKeyCombination keyCombination);
    static QList<bool> activeStates(const QSignalSpy &spy, QAction *action);

    std::unique_ptr<KGlobalAccelD> m_globalacceld;
    KGlobalAccel *m_globalaccel;
};

void ShortcutsTest::sendKeyCombination(QKeyCombination keyCombination, ShortcutKeyState state)
{
    struct {
        Qt::KeyboardModifier modifier;
        Qt::Key key;
    } modifiers[] = {
        {
            .modifier = Qt::MetaModifier,
            .key = Qt::Key_Meta,
        },
        {
            .modifier = Qt::AltModifier,
            .key = Qt::Key_Alt,
        },
        {
            .modifier = Qt::ControlModifier,
            .key = Qt::Key_Control,
        },
        {
            .modifier = Qt::ShiftModifier,
            .key = Qt::Key_Shift,
        },
    };

    if (state == ShortcutKeyState::Pressed || state == ShortcutKeyState::Repeated) {
        Qt::KeyboardModifiers formerModifiers;
        for (const auto &[modifier, key] : modifiers) {
            if (keyCombination.keyboardModifiers() & modifier) {
                m_globalacceld->keyEvent((formerModifiers | key).toCombined(), state);
                formerModifiers |= modifier;
            }
        }

        if (keyCombination.key()) {
            m_globalacceld->keyEvent(keyCombination.toCombined(), state);
        }
    } else {
        if (keyCombination.key()) {
            m_globalacceld->keyEvent(keyCombination.toCombined(), state);
        }

        Qt::KeyboardModifiers formerModifiers = keyCombination.keyboardModifiers();
        for (const auto &[modifier, key] : modifiers) {
            if (formerModifiers & modifier) {
                formerModifiers &= ~modifier;
                m_globalacceld->keyEvent((formerModifiers | key).toCombined(), state);
            }
        }
    }
}

void ShortcutsTest::sendKeyCombinationPressAndRelease(QKeyCombination keyCombination)
{
    sendKeyCombination(keyCombination, ShortcutKeyState::Pressed);
    sendKeyCombination(keyCombination, ShortcutKeyState::Released);
}

QList<bool> ShortcutsTest::activeStates(const QSignalSpy &spy, QAction *action)
{
    QList<bool> states;
    for (const QList<QVariant> &arguments : spy) {
        if (arguments.at(0).value<QAction *>() == action) {
            states.append(arguments.at(1).toBool());
        }
    }
    return states;
}

void ShortcutsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    if (const QString filePath = QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("kglobalshortcutsrc")); !filePath.isEmpty()) {
        QFile::remove(filePath);
    }
    if (const QString filePath = QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("kglobalaccelrc")); !filePath.isEmpty()) {
        QFile::remove(filePath);
    }

    m_globalacceld = std::make_unique<KGlobalAccelD>();
    QVERIFY(m_globalacceld->init());
    m_globalaccel = KGlobalAccel::self();
    QVERIFY(m_globalaccel);
}

typedef std::pair<QEvent::Type, int> Event;
typedef QList<Event> Events;

void ShortcutsTest::testShortcuts_data()
{
    QTest::addColumn<QKeySequence>("shortcut");
    QTest::addColumn<Events>("events");
    QTest::addColumn<bool>("triggered");

    // make sure all pressed modifiers are released
    QTest::newRow("no mod") << QKeySequence(Qt::Key_A)
                            << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_A) << std::make_pair(QEvent::KeyRelease, Qt::Key_A)) << true;
    QTest::newRow("mod+key trigger") << QKeySequence(Qt::ControlModifier | Qt::Key_P)
                                     << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                  << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_P).toCombined())
                                                  << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_P).toCombined())
                                                  << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                     << true;
    QTest::newRow("mods+key trigger") << QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_M)
                                      << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                   << std::make_pair(QEvent::KeyPress, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                                                   << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_M).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_M).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                      << true;
    QTest::newRow("mods+key does not trigger mod+key")
        << QKeySequence(Qt::ControlModifier | Qt::Key_P)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+key does not trigger mods+key")
        << QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_M)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_M).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_M).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;

    QTest::newRow("modifier-only single mod") << QKeySequence(
        Qt::Key_Control) << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                              << true;
    QTest::newRow("mod+key does not trigger modifier-only single mod")
        << QKeySequence(Qt::Key_Control)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("modifier-only multiple mods") << QKeySequence(Qt::ControlModifier | Qt::Key_Alt)
                                                 << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                              << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                                                              << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                                                              << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                                 << true;
    QTest::newRow("modifier-only multiple mods trigger when released out of order")
        << QKeySequence(Qt::ControlModifier | Qt::Key_Alt)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Alt) << std::make_pair(QEvent::KeyPress, (Qt::AltModifier | Qt::Key_Control).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << true;
    QTest::newRow("modifier-only multiple mods does not trigger modifier-only single mod")
        << QKeySequence(Qt::Key_Control)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("modifier-only multiple mods trigger when part of the sequence")
        << QKeySequence(Qt::ControlModifier | Qt::Key_Alt)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::AltModifier | Qt::Key_Control).toCombined()) << std::make_pair(QEvent::KeyRelease, Qt::Key_Alt))
        << true;
    QTest::newRow("modifier-only multiple mods trigger when any mod is released")
        << QKeySequence(Qt::ControlModifier | Qt::Key_Shift)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::AltModifier | Qt::Key_Control).toCombined()) << std::make_pair(QEvent::KeyRelease, Qt::Key_Alt))
        << true;
    QTest::newRow("mod+pointer does not trigger modifier-only single mod")
        << QKeySequence(Qt::Key_Control)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::MouseButtonPress, Qt::LeftButton)
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+wheel does not trigger modifier-only single mod")
        << QKeySequence(Qt::Key_Control)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::Wheel, 0)
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+invalid does not trigger modifier-only single mod")
        << QKeySequence(Qt::Key_Control)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::ControlModifier) << std::make_pair(QEvent::KeyRelease, Qt::ControlModifier)) << false;
}

void ShortcutsTest::testShortcuts()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("ActionForShortcutTest"));
    QFETCH(QKeySequence, shortcut);
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcut));
    QCOMPARE(m_globalaccel->shortcut(action.get()), QList<QKeySequence>() << shortcut);

    QSignalSpy spy(action.get(), &QAction::triggered);

    QFETCH(Events, events);
    for (const auto &event : events) {
        switch (event.first) {
        case QEvent::KeyPress:
            m_globalacceld->keyEvent(event.second, ShortcutKeyState::Pressed);
            break;
        case QEvent::KeyRelease:
            m_globalacceld->keyEvent(event.second, ShortcutKeyState::Released);
            break;
        case QEvent::MouseButtonPress:
            m_globalacceld->pointerPressed(static_cast<Qt::MouseButtons>(event.second));
            break;
        case QEvent::Wheel:
            m_globalacceld->axisTriggered(event.second);
            break;
        default:
            qFatal("Unknown event type");
        }
    }

    QFETCH(bool, triggered);
    if (triggered) {
        QVERIFY(spy.wait());
        QCOMPARE(spy.count(), 1);
    } else {
        QVERIFY(!spy.wait(100));
        QCOMPARE(spy.count(), 0);
    }
    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testSerialization()
{
    QCOMPARE(Component::keysFromString(QLatin1String("")), QSet<QKeySequence>());
    QCOMPARE(Component::keysFromString(QLatin1String("none")), QSet<QKeySequence>());
    QCOMPARE(Component::stringFromKeys(QSet<QKeySequence>()), QLatin1String("none"));

    QCOMPARE(Component::keysFromString(QLatin1String("Ctrl+P")), QSet<QKeySequence>() << QKeySequence(Qt::CTRL | Qt::Key_P));
    QCOMPARE(Component::stringFromKeys(QSet<QKeySequence>() << QKeySequence(Qt::CTRL | Qt::Key_P)), QLatin1String("Ctrl+P"));

    QCOMPARE(Component::keysFromString(QLatin1String("\tCtrl+P")), QSet<QKeySequence>() << QKeySequence(Qt::CTRL | Qt::Key_P));
    QCOMPARE(Component::keysFromString(QLatin1String("\tCtrl+P\t")), QSet<QKeySequence>() << QKeySequence(Qt::CTRL | Qt::Key_P));
    QCOMPARE(Component::stringFromKeys(QSet<QKeySequence>() << QKeySequence(Qt::CTRL | Qt::Key_P)), QLatin1String("Ctrl+P"));
}

void ShortcutsTest::testContestedKeys()
{
    const QKeySequence shortcut(Qt::META | Qt::CTRL | Qt::Key_K);

    auto firstAction = std::make_unique<QAction>();
    firstAction->setObjectName(QStringLiteral("First Shortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(firstAction.get(), shortcut));
    QSignalSpy firstActionTriggeredSpy(firstAction.get(), &QAction::triggered);

    auto secondAction = std::make_unique<QAction>();
    secondAction->setObjectName(QStringLiteral("Second Shortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(secondAction.get(), shortcut));
    QSignalSpy secondActionTriggeredSpy(secondAction.get(), &QAction::triggered);

    sendKeyCombination(Qt::MetaModifier | Qt::ControlModifier | Qt::Key_K, ShortcutKeyState::Pressed);

    QVERIFY(firstActionTriggeredSpy.wait());
    QCOMPARE(firstActionTriggeredSpy.count(), 1);
    QVERIFY(!secondActionTriggeredSpy.wait(100));
    QCOMPARE(secondActionTriggeredSpy.count(), 0);

    sendKeyCombination(Qt::MetaModifier | Qt::ControlModifier | Qt::Key_K, ShortcutKeyState::Released);

    m_globalaccel->removeAllShortcuts(firstAction.get());
    m_globalaccel->removeAllShortcuts(secondAction.get());
}

void ShortcutsTest::testBacktab_data()
{
    QTest::addColumn<QKeySequence>("shortcut");

    QTest::addRow("Shift+Tab") << QKeySequence(Qt::ShiftModifier | Qt::Key_Tab);
    QTest::addRow("Backtab") << QKeySequence(Qt::Key_Backtab);
    QTest::addRow("Shift+Backtab") << QKeySequence(Qt::ShiftModifier | Qt::Key_Backtab);
}

void ShortcutsTest::testBacktab()
{
    QFETCH(QKeySequence, shortcut);

    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("Backtab Shortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcut));
    QSignalSpy actionTriggeredSpy(action.get(), &QAction::triggered);

    // Shift+Tab should be treated as Backtab or Shift+Backtab. Note that Tab should not treated as Backtab.
    sendKeyCombinationPressAndRelease(Qt::Key_Tab);
    QVERIFY(!actionTriggeredSpy.wait(100));

    sendKeyCombinationPressAndRelease(Qt::ShiftModifier | Qt::Key_Tab);
    QVERIFY(actionTriggeredSpy.wait());

    sendKeyCombinationPressAndRelease(Qt::MetaModifier | Qt::Key_Tab);
    QVERIFY(!actionTriggeredSpy.wait(100));

    sendKeyCombinationPressAndRelease(Qt::MetaModifier | Qt::ShiftModifier | Qt::Key_Tab);
    QVERIFY(!actionTriggeredSpy.wait(100));

    // Backtab and Shift+Backtab should be treated as Shift+Tab.
    sendKeyCombinationPressAndRelease(Qt::Key_Backtab);
    QVERIFY(actionTriggeredSpy.wait());

    sendKeyCombinationPressAndRelease(Qt::ShiftModifier | Qt::Key_Backtab);
    QVERIFY(actionTriggeredSpy.wait());

    sendKeyCombinationPressAndRelease(Qt::MetaModifier | Qt::Key_Backtab);
    QVERIFY(!actionTriggeredSpy.wait(100));

    sendKeyCombinationPressAndRelease(Qt::MetaModifier | Qt::ShiftModifier | Qt::Key_Backtab);
    QVERIFY(!actionTriggeredSpy.wait(100));

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testEmptyShortcut_data()
{
    QTest::addColumn<QList<QKeySequence>>("shortcuts");
    QTest::addColumn<QList<QKeySequence>>("expected");

    QTest::addRow("List()") << QList<QKeySequence>{} << QList<QKeySequence>{};

    QTest::addRow("List(QKeySequence())") << QList<QKeySequence>{QKeySequence()} << QList<QKeySequence>{};

    QTest::addRow("List(QKeySequence(), QKeySequence(Meta+A))")
        << QList<QKeySequence>{QKeySequence(), QKeySequence(Qt::MetaModifier | Qt::Key_A)} << QList<QKeySequence>{QKeySequence(Qt::MetaModifier | Qt::Key_A)};
}

void ShortcutsTest::testEmptyShortcut()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("Empty Shortcut"));
    QFETCH(QList<QKeySequence>, shortcuts);
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcuts));
    QTEST(m_globalaccel->shortcut(action.get()), "expected");

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testHeldShortcut_data()
{
    QTest::addColumn<QKeySequence>("shortcut");
    QTest::addColumn<QList<int>>("keysTypedBefore");
    QTest::addColumn<QList<int>>("keysHeldBefore");

    QTest::newRow("unrelated key") << QKeySequence(Qt::Key_F13) << QList<int>{} << QList<int>{};
    QTest::newRow("unrelated key held before shortcut") << QKeySequence(Qt::Key_F13) << QList<int>{} << QList<int>{Qt::Key_A};
    QTest::newRow("unrelated keys typed before shortcut") << QKeySequence(Qt::Key_F13) << QList<int>{Qt::Key_A, Qt::Key_B} << QList<int>{};
    QTest::newRow("mod+key") << QKeySequence(Qt::ControlModifier | Qt::Key_F13) << QList<int>{Qt::Key_A} << QList<int>{};
}

void ShortcutsTest::testHeldShortcut()
{
    QFETCH(QKeySequence, shortcut);
    QFETCH(QList<int>, keysTypedBefore);
    QFETCH(QList<int>, keysHeldBefore);

    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("HeldShortcutTest"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcut));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    for (int key : std::as_const(keysTypedBefore)) {
        sendKeyCombinationPressAndRelease(static_cast<Qt::Key>(key));
    }
    for (int key : std::as_const(keysHeldBefore)) {
        sendKeyCombination(static_cast<Qt::Key>(key), ShortcutKeyState::Pressed);
    }

    // Hold the shortcut
    sendKeyCombination(shortcut[0], ShortcutKeyState::Pressed);
    QVERIFY(activeSpy.wait());
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(activeSpy.last().at(1).toBool(), true);

    // Releasing keys that are not the shortcut's key must not end it
    for (int key : std::as_const(keysHeldBefore)) {
        sendKeyCombination(static_cast<Qt::Key>(key), ShortcutKeyState::Released);
    }
    for (int key : std::as_const(keysTypedBefore) + QList<int>{Qt::Key_C}) {
        sendKeyCombinationPressAndRelease(static_cast<Qt::Key>(key));
    }
    QVERIFY(!activeSpy.wait(100));
    QCOMPARE(activeSpy.count(), 1);

    // Releasing the shortcut ends it
    sendKeyCombination(shortcut[0], ShortcutKeyState::Released);
    QVERIFY(activeSpy.wait());
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(activeSpy.last().at(1).toBool(), false);

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testBacktabRelease_data()
{
    QTest::addColumn<Events>("events");

    QTest::newRow("Shift released last") << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Shift)
                                                      << std::make_pair(QEvent::KeyPress, (Qt::ShiftModifier | Qt::Key_Backtab).toCombined())
                                                      << std::make_pair(QEvent::KeyRelease, (Qt::ShiftModifier | Qt::Key_Backtab).toCombined())
                                                      << std::make_pair(QEvent::KeyRelease, (Qt::ShiftModifier | Qt::Key_Shift).toCombined()));
    // The release arrives as Tab here, so this order worked before too. Checks that it keeps working.
    QTest::newRow("Shift released first") << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Shift)
                                                       << std::make_pair(QEvent::KeyPress, (Qt::ShiftModifier | Qt::Key_Backtab).toCombined())
                                                       << std::make_pair(QEvent::KeyRelease, (Qt::ShiftModifier | Qt::Key_Shift).toCombined())
                                                       << std::make_pair(QEvent::KeyRelease, Qt::Key_Tab));
    QTest::newRow("Backtab without Shift") << (Events()
                                               << std::make_pair(QEvent::KeyPress, Qt::Key_Backtab) << std::make_pair(QEvent::KeyRelease, Qt::Key_Backtab));
}

void ShortcutsTest::testBacktabRelease()
{
    QFETCH(Events, events);

    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("BacktabReleaseTest"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), QKeySequence(Qt::ShiftModifier | Qt::Key_Tab)));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    for (const auto &event : std::as_const(events)) {
        m_globalacceld->keyEvent(event.second, event.first == QEvent::KeyPress ? ShortcutKeyState::Pressed : ShortcutKeyState::Released);
    }

    // Backtab is normalized to Shift+Tab, its release must still end the shortcut
    QTRY_COMPARE(activeSpy.count(), 2);
    QCOMPARE(activeSpy.at(0).at(1).toBool(), true);
    QCOMPARE(activeSpy.at(1).at(1).toBool(), false);

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testShortcutDuringHeldShortcut_data()
{
    QTest::addColumn<QKeySequence>("secondShortcut");

    QTest::newRow("key") << QKeySequence(Qt::Key_F14);
    QTest::newRow("mod+key") << QKeySequence(Qt::ControlModifier | Qt::Key_F14);
    QTest::newRow("modifier-only Meta") << QKeySequence(Qt::MetaModifier);
}

void ShortcutsTest::testShortcutDuringHeldShortcut()
{
    QFETCH(QKeySequence, secondShortcut);

    auto held = std::make_unique<QAction>();
    held->setObjectName(QStringLiteral("HeldDuringOtherShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(held.get(), QKeySequence(Qt::Key_F13)));
    auto other = std::make_unique<QAction>();
    other->setObjectName(QStringLiteral("OtherDuringHeldShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(other.get(), secondShortcut));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, held.get()), QList<bool>{true});

    sendKeyCombinationPressAndRelease(secondShortcut[0]);
    QTRY_COMPARE(activeStates(activeSpy, other.get()), (QList<bool>{true, false}));
    QCOMPARE(activeStates(activeSpy, held.get()), QList<bool>{true});

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Released);
    QTRY_COMPARE(activeStates(activeSpy, held.get()), (QList<bool>{true, false}));

    m_globalaccel->removeAllShortcuts(held.get());
    m_globalaccel->removeAllShortcuts(other.get());
}

void ShortcutsTest::testOverlappingHeldShortcuts()
{
    auto first = std::make_unique<QAction>();
    first->setObjectName(QStringLiteral("FirstOverlappingShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(first.get(), QKeySequence(Qt::Key_F13)));
    auto second = std::make_unique<QAction>();
    second->setObjectName(QStringLiteral("SecondOverlappingShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(second.get(), QKeySequence(Qt::Key_F14)));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    sendKeyCombination(Qt::Key_F14, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, second.get()), QList<bool>{true});
    QCOMPARE(activeStates(activeSpy, first.get()), QList<bool>{true});

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Released);
    QTRY_COMPARE(activeStates(activeSpy, first.get()), (QList<bool>{true, false}));
    QCOMPARE(activeStates(activeSpy, second.get()), QList<bool>{true});

    sendKeyCombination(Qt::Key_F14, ShortcutKeyState::Released);
    QTRY_COMPARE(activeStates(activeSpy, second.get()), (QList<bool>{true, false}));

    m_globalaccel->removeAllShortcuts(first.get());
    m_globalaccel->removeAllShortcuts(second.get());
}

void ShortcutsTest::testShortcutWithTwoHeldKeys_data()
{
    QTest::addColumn<bool>("releaseFirstPressedFirst");

    QTest::newRow("released in press order") << true;
    QTest::newRow("released in reverse order") << false;
}

void ShortcutsTest::testShortcutWithTwoHeldKeys()
{
    QFETCH(bool, releaseFirstPressedFirst);

    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("ShortcutWithTwoHeldKeys"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), QList<QKeySequence>{QKeySequence(Qt::Key_F13), QKeySequence(Qt::Key_F14)}));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    sendKeyCombination(Qt::Key_F14, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), QList<bool>{true});

    sendKeyCombination(releaseFirstPressedFirst ? Qt::Key_F13 : Qt::Key_F14, ShortcutKeyState::Released);
    QVERIFY(!activeSpy.wait(100));
    QCOMPARE(activeStates(activeSpy, action.get()), QList<bool>{true});

    sendKeyCombination(releaseFirstPressedFirst ? Qt::Key_F14 : Qt::Key_F13, ShortcutKeyState::Released);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false}));

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testHeldShortcutUnregistered()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("HeldShortcutUnregistered"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), QKeySequence(Qt::Key_F13)));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), QList<bool>{true});

    QVERIFY(m_globalaccel->setShortcut(action.get(), {QKeySequence(Qt::Key_F15)}, KGlobalAccel::NoAutoloading));
    QTRY_COMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false}));

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Released);
    QVERIFY(!activeSpy.wait(100));
    QCOMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false}));

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testLostRelease()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("LostReleaseShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), QKeySequence(Qt::Key_F13)));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), QList<bool>{true});

    // The release got lost, pressing the key again restarts the shortcut
    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false, true}));

    // Autorepeat doesn't end the shortcut
    m_globalacceld->keyEvent(Qt::Key_F13, ShortcutKeyState::Repeated);
    QVERIFY(!activeSpy.wait(100));
    QCOMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false, true}));

    sendKeyCombination(Qt::Key_F13, ShortcutKeyState::Released);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false, true, false}));

    m_globalaccel->removeAllShortcuts(action.get());
}

void ShortcutsTest::testLostReleaseWithoutModifier()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("LostReleaseWithoutModifierShortcut"));
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), QKeySequence(Qt::ControlModifier | Qt::Key_F13)));
    QSignalSpy activeSpy(m_globalaccel, &KGlobalAccel::globalShortcutActiveChanged);

    sendKeyCombination(Qt::ControlModifier | Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), QList<bool>{true});

    // F13 without Ctrl ends the shortcut without starting it again
    m_globalacceld->keyEvent(Qt::Key_F13, ShortcutKeyState::Pressed);
    QTRY_COMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false}));

    m_globalacceld->keyEvent(Qt::Key_F13, ShortcutKeyState::Released);
    QVERIFY(!activeSpy.wait(100));
    QCOMPARE(activeStates(activeSpy, action.get()), (QList<bool>{true, false}));

    m_globalaccel->removeAllShortcuts(action.get());
}

QTEST_MAIN(ShortcutsTest)

#include "shortcutstest.moc"
