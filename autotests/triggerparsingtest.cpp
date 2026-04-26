/*
    SPDX-FileCopyrightText: 2026 Muhammad Ahmad

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kglobalshortcuttrigger.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace KGlobalShortcutTriggerTypes;

class TriggerParsingTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void parsesEmptyTrigger();
    void parsesTouchpadSwipe();
    void parsesTouchscreenSwipe();
    void parsesTouchpadPinch();
    void rejectsInvalidTouchpadPinch();
    void parsesTouchscreenPinch();
    void parsesLineShape();
};

void TriggerParsingTest::parsesEmptyTrigger()
{
    const KGlobalShortcutTrigger t1{QString(), QString()};
    const KGlobalShortcutTrigger t2{QString(), u"3:Left"_s};
    const KGlobalShortcutTrigger t3{QString(), u"TouchpadSwipe@3:Left"_s};
    const auto t4 = KGlobalShortcutTrigger::fromString(QString());

    QVERIFY(t1.isEmpty());
    QVERIFY(t2.isEmpty());
    QVERIFY(t3.isEmpty());
    QVERIFY(t4.isEmpty());

    QVERIFY(t3.asTouchpadSwipeGesture() == nullptr);
}

void TriggerParsingTest::parsesTouchpadSwipe()
{
    const KGlobalShortcutTrigger t1("TouchpadSwipe"_L1, "3:Left"_L1);
    const KGlobalShortcutTrigger t2("TouchpadSwipe"_L1, "4:Right"_L1);
    const KGlobalShortcutTrigger t3(u"TouchpadSwipe"_s, "2:Up"_L1);
    const auto t4 = KGlobalShortcutTrigger::fromString(u"TouchpadSwipe@5:Down"_s);

    QVERIFY(!t1.isEmpty());
    QVERIFY(!t2.isEmpty());
    QVERIFY(!t3.isEmpty());
    QVERIFY(!t4.isEmpty());

    QCOMPARE(t1.type(), "TouchpadSwipe"_L1);
    QCOMPARE(t2.type(), "TouchpadSwipe"_L1);
    QCOMPARE(t3.type(), "TouchpadSwipe"_L1);
    QCOMPARE(t4.type(), "TouchpadSwipe"_L1);

    const TouchpadSwipeGesture *swipe = t1.asTouchpadSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 3);
    QCOMPARE(swipe->direction, SwipeDirection::Left);

    swipe = t2.asTouchpadSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 4);
    QCOMPARE(swipe->direction, SwipeDirection::Right);

    swipe = t3.asTouchpadSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 2);
    QCOMPARE(swipe->direction, SwipeDirection::Up);

    swipe = t4.asTouchpadSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 5);
    QCOMPARE(swipe->direction, SwipeDirection::Down);
}

void TriggerParsingTest::parsesTouchscreenSwipe()
{
    const KGlobalShortcutTrigger t1("TouchscreenSwipe"_L1, "3:Left"_L1);
    const KGlobalShortcutTrigger t2("TouchscreenSwipe"_L1, "4:Right"_L1);
    const KGlobalShortcutTrigger t3(u"TouchscreenSwipe"_s, "2:Up"_L1);
    const auto t4 = KGlobalShortcutTrigger::fromString(u"TouchscreenSwipe@5:Down"_s);

    QVERIFY(!t1.isEmpty());
    QVERIFY(!t2.isEmpty());
    QVERIFY(!t3.isEmpty());
    QVERIFY(!t4.isEmpty());

    QCOMPARE(t1.type(), "TouchscreenSwipe"_L1);
    QCOMPARE(t2.type(), "TouchscreenSwipe"_L1);
    QCOMPARE(t3.type(), "TouchscreenSwipe"_L1);
    QCOMPARE(t4.type(), "TouchscreenSwipe"_L1);

    const TouchscreenSwipeGesture *swipe = t1.asTouchscreenSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 3);
    QCOMPARE(swipe->direction, SwipeDirection::Left);

    swipe = t2.asTouchscreenSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 4);
    QCOMPARE(swipe->direction, SwipeDirection::Right);

    swipe = t3.asTouchscreenSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 2);
    QCOMPARE(swipe->direction, SwipeDirection::Up);

    swipe = t4.asTouchscreenSwipeGesture();
    QVERIFY(swipe != nullptr);
    QCOMPARE(swipe->fingerCount, 5);
    QCOMPARE(swipe->direction, SwipeDirection::Down);
}

void TriggerParsingTest::parsesTouchpadPinch()
{
    const KGlobalShortcutTrigger t1("TouchpadPinch"_L1, "3:Expanding"_L1);
    const auto t2 = KGlobalShortcutTrigger::fromString(u"TouchpadPinch@4:Contracting"_s);

    QVERIFY(!t1.isEmpty());
    QVERIFY(!t2.isEmpty());

    QCOMPARE(t1.type(), "TouchpadPinch"_L1);
    QCOMPARE(t2.type(), "TouchpadPinch"_L1);

    const TouchpadPinchGesture *pinch = t1.asTouchpadPinchGesture();
    QVERIFY(pinch != nullptr);
    QCOMPARE(pinch->fingerCount, 3);
    QCOMPARE(pinch->direction, PinchDirection::Expanding);

    pinch = t2.asTouchpadPinchGesture();
    QVERIFY(pinch != nullptr);
    QCOMPARE(pinch->fingerCount, 4);
    QCOMPARE(pinch->direction, PinchDirection::Contracting);
}

void TriggerParsingTest::rejectsInvalidTouchpadPinch()
{
    const KGlobalShortcutTrigger t1("TouchpadPinch"_L1, "foo:Expanding"_L1);
    QVERIFY(t1.asTouchpadPinchGesture() == nullptr);

    const KGlobalShortcutTrigger t2("TouchpadPinch"_L1, "3:Zoom"_L1);
    QVERIFY(t2.asTouchpadPinchGesture() == nullptr);

    const KGlobalShortcutTrigger t3("TouchpadPinch"_L1, "3"_L1);
    QVERIFY(t3.asTouchpadPinchGesture() == nullptr);

    const auto t4 = KGlobalShortcutTrigger::fromString("TouchpadPinch@3"_L1);
    QVERIFY(t4.asTouchpadPinchGesture() == nullptr);

    const KGlobalShortcutTrigger t5("TouchpadPinch"_L1, QString());
    QVERIFY(t5.asTouchpadPinchGesture() == nullptr);
    QCOMPARE(t5.isEmpty(), false);

    const KGlobalShortcutTrigger t6(QString(), u"TouchpadPinch@3:Expanding"_s); // incorrect use of API
    QVERIFY(t6.asTouchpadPinchGesture() == nullptr);
    QCOMPARE(t6.isEmpty(), true);
}

void TriggerParsingTest::parsesTouchscreenPinch()
{
    const KGlobalShortcutTrigger t1("TouchscreenPinch"_L1, "3:Expanding"_L1);
    const auto t2 = KGlobalShortcutTrigger::fromString(u"TouchscreenPinch@4:Contracting"_s);

    QVERIFY(!t1.isEmpty());
    QVERIFY(!t2.isEmpty());

    QCOMPARE(t1.type(), "TouchscreenPinch"_L1);
    QCOMPARE(t2.type(), "TouchscreenPinch"_L1);

    const TouchscreenPinchGesture *pinch = t1.asTouchscreenPinchGesture();
    QVERIFY(pinch != nullptr);
    QCOMPARE(pinch->fingerCount, 3);
    QCOMPARE(pinch->direction, PinchDirection::Expanding);

    pinch = t2.asTouchscreenPinchGesture();
    QVERIFY(pinch != nullptr);
    QCOMPARE(pinch->fingerCount, 4);
    QCOMPARE(pinch->direction, PinchDirection::Contracting);
}

void TriggerParsingTest::parsesLineShape()
{
    const KGlobalShortcutTrigger t1("LineShape"_L1, "(0;0)(0;1)"_L1); // straight line down
    const KGlobalShortcutTrigger t2(u"LineShape"_s, u"(-0.0;0.5)(0;1)(1;1)(1;-1)"_s);

    QVERIFY(!t1.isEmpty());
    QVERIFY(!t2.isEmpty());

    QCOMPARE(t1.type(), "LineShape"_L1);
    QCOMPARE(t2.type(), "LineShape"_L1);

    const LineShapeGesture *lineShape = t1.asLineShapeGesture();
    const auto p1 = QList<QPointF>{{0, 0}, {0, 1}};
    QVERIFY(lineShape != nullptr);
    QCOMPARE(lineShape->points, p1);

    lineShape = t2.asLineShapeGesture();
    const auto p2 = QList<QPointF>{{0, 0.5}, {0, 1}, {1, 1}, {1, -1}};
    QVERIFY(lineShape != nullptr);
    QCOMPARE(lineShape->points, p2);
}

QTEST_MAIN(TriggerParsingTest)
#include "triggerparsingtest.moc"
