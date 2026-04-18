/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcuttrigger.h"
#include "kglobalshortcuttrigger_p.h"

#include <QMetaEnum>
#include <QtAssert>
#include <variant>

using namespace Qt::StringLiterals;
using namespace KGlobalShortcutTriggerTypes;

static const QString TouchpadSwipeString = "TouchpadSwipe"_L1;
static const QString TouchpadSwipe2DString = "TouchpadSwipe2D"_L1;
static const QString TouchpadPinchString = "TouchpadPinch"_L1;
static const QString TouchpadRotateString = "TouchpadRotate"_L1;
static const QString TouchpadHoldString = "TouchpadHold"_L1;
static const QString ApproachScreenBorderString = "ApproachScreenBorder"_L1;
static const QString TouchscreenSwipeString = "TouchscreenSwipe"_L1;
static const QString TouchscreenSwipe2DString = "TouchscreenSwipe2D"_L1;
static const QString TouchscreenSwipeFromEdgeString = "TouchscreenSwipeFromEdge"_L1;
static const QString TouchscreenPinchString = "TouchscreenPinch"_L1;
static const QString TouchscreenRotateString = "TouchscreenRotate"_L1;
static const QString TouchscreenHoldString = "TouchscreenHold"_L1;
static const QString PointerAxisString = "PointerAxis"_L1;
static const QString LineShapeString = "LineShape"_L1;

//
// Trigger type constructors

namespace KGlobalShortcutTriggerTypes
{

// TouchpadSwipeGesture::TouchpadSwipeGesture(int fingerCount, SwipeDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchpadSwipe2DGesture::TouchpadSwipe2DGesture(int fingerCount)
//     : fingerCount(fingerCount)
// {
// }
//
// TouchpadPinchGesture::TouchpadPinchGesture(int fingerCount, PinchDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchpadRotateGesture::TouchpadRotateGesture(int fingerCount, RotateDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchpadHoldGesture::TouchpadHoldGesture(int fingerCount, std::chrono::milliseconds duration)
//     : fingerCount(fingerCount)
//     , duration(duration)
// {
// }
//
// ApproachScreenBorderGesture::ApproachScreenBorderGesture(ScreenBorder border)
//     : border(border)
// {
// }
//
// TouchscreenSwipeGesture::TouchscreenSwipeGesture(int fingerCount, SwipeDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchscreenSwipe2DGesture::TouchscreenSwipe2DGesture(int fingerCount)
//     : fingerCount(fingerCount)
// {
// }
//
// TouchscreenSwipeFromEdgeGesture::TouchscreenSwipeFromEdgeGesture(int fingerCount, EdgeSwipeDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchscreenPinchGesture::TouchscreenPinchGesture(int fingerCount, PinchDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchscreenRotateGesture::TouchscreenRotateGesture(int fingerCount, RotateDirection direction)
//     : fingerCount(fingerCount)
//     , direction(direction)
// {
// }
//
// TouchscreenHoldGesture::TouchscreenHoldGesture(int fingerCount, std::chrono::milliseconds duration)
//     : fingerCount(fingerCount)
//     , duration(duration)
// {
// }
//
// PointerAxisGesture::PointerAxisGesture(PointerAxisDirection direction, MouseButtonRequirement button)
//     : direction(direction)
//     , button(button)
// {
// }
//
// LineShapeGesture::LineShapeGesture(QList<QPointF> points)
//     : points(std::move(points))
// {
// }

} // namespace KGlobalShortcutTriggerTypes

static QString useStaticTriggerTypeConstantIfPossible(const QString &triggerType)
{
    if (triggerType == TouchpadSwipeString) {
        return TouchpadSwipeString;
    } else if (triggerType == TouchpadSwipe2DString) {
        return TouchpadSwipe2DString;
    } else if (triggerType == TouchpadPinchString) {
        return TouchpadPinchString;
    } else if (triggerType == TouchpadRotateString) {
        return TouchpadRotateString;
    } else if (triggerType == TouchpadHoldString) {
        return TouchpadHoldString;
    } else if (triggerType == ApproachScreenBorderString) {
        return ApproachScreenBorderString;
    } else if (triggerType == TouchscreenSwipeString) {
        return TouchscreenSwipeString;
    } else if (triggerType == TouchscreenSwipe2DString) {
        return TouchscreenSwipe2DString;
    } else if (triggerType == TouchscreenSwipeFromEdgeString) {
        return TouchscreenSwipeFromEdgeString;
    } else if (triggerType == TouchscreenPinchString) {
        return TouchscreenPinchString;
    } else if (triggerType == TouchscreenRotateString) {
        return TouchscreenRotateString;
    } else if (triggerType == TouchscreenHoldString) {
        return TouchscreenHoldString;
    } else if (triggerType == PointerAxisString) {
        return PointerAxisString;
    } else if (triggerType == LineShapeString) {
        return LineShapeString;
    } else {
        return triggerType;
    }
}

//
// KGlobalShortcutTrigger

KGlobalShortcutTrigger::KGlobalShortcutTrigger()
    : d(new KGlobalShortcutTriggerPrivate(QString(), KGlobalShortcutTriggerPrivate::Unparseable{}))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const QString &triggerType, const QString &serializedTriggerParams)
    : d(new KGlobalShortcutTriggerPrivate(useStaticTriggerTypeConstantIfPossible(triggerType),
                                          KGlobalShortcutTriggerPrivate::Uninitialized{.serializedTriggerParams = serializedTriggerParams}))
{
    // variant parsing will happen later in KGlobalShortcutTriggerPrivate::deserialize()
}

KGlobalShortcutTrigger::~KGlobalShortcutTrigger()
{
    delete d;
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const KGlobalShortcutTrigger &rhs)
    : d(new KGlobalShortcutTriggerPrivate(rhs.d->triggerType, rhs.d->variant))
{
}

KGlobalShortcutTrigger &KGlobalShortcutTrigger::operator=(const KGlobalShortcutTrigger &rhs)
{
    KGlobalShortcutTrigger tmp(rhs);
    KGlobalShortcutTriggerPrivate *swap = d;
    d = tmp.d;
    tmp.d = swap;
    return *this;
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadSwipeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchpadSwipeString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadSwipe2DGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchpadSwipe2DString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadPinchGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchpadPinchString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadRotateGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchpadRotateString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadHoldGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchpadHoldString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const ApproachScreenBorderGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(ApproachScreenBorderString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenSwipeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenSwipeString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenSwipe2DGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenSwipe2DString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenSwipeFromEdgeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenSwipeFromEdgeString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenPinchGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenPinchString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenRotateGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenRotateString, sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenHoldGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(TouchscreenHoldString, sc))
{
}

#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS // see kglobalshortcuttrigger.h
KGlobalShortcutTrigger::KGlobalShortcutTrigger(const PointerAxisGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(PointerAxisString, sc))
{
}
#endif

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const LineShapeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(LineShapeString, sc))
{
}

bool KGlobalShortcutTrigger::isEmpty() const
{
    return d->triggerType.isEmpty();
}

QString KGlobalShortcutTrigger::type() const
{
    return d->triggerType;
}

bool KGlobalShortcutTrigger::isKnownTriggerType() const
{
    d->deserialize();
    return !std::holds_alternative<KGlobalShortcutTriggerPrivate::Unparseable>(d->variant);
}

bool KGlobalShortcutTrigger::operator==(const KGlobalShortcutTrigger &rhs) const
{
    return d->triggerType == rhs.d->triggerType && serializedTriggerParams() == rhs.serializedTriggerParams();
    // TODO: use d->variant == rhs.d->variant instead of serializedTriggerParams, but it's lacking variants' operator==()
}

bool KGlobalShortcutTrigger::conflictsWith(const KGlobalShortcutTrigger &other) const
{
    if (isEmpty() || other.isEmpty()) {
        return false;
    } else if (*this == other) {
        return true;
    }
    return false;
}

const TouchpadSwipeGesture *KGlobalShortcutTrigger::asTouchpadSwipeGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadSwipeGesture>(&d->variant);
}

const TouchpadSwipe2DGesture *KGlobalShortcutTrigger::asTouchpadSwipe2DGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadSwipe2DGesture>(&d->variant);
}

const TouchpadPinchGesture *KGlobalShortcutTrigger::asTouchpadPinchGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadPinchGesture>(&d->variant);
}

const TouchpadRotateGesture *KGlobalShortcutTrigger::asTouchpadRotateGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadRotateGesture>(&d->variant);
}

const TouchpadHoldGesture *KGlobalShortcutTrigger::asTouchpadHoldGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadHoldGesture>(&d->variant);
}

const ApproachScreenBorderGesture *KGlobalShortcutTrigger::asApproachScreenBorderGesture() const
{
    d->deserialize();
    return std::get_if<ApproachScreenBorderGesture>(&d->variant);
}

const TouchscreenSwipeGesture *KGlobalShortcutTrigger::asTouchscreenSwipeGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenSwipeGesture>(&d->variant);
}

const TouchscreenSwipe2DGesture *KGlobalShortcutTrigger::asTouchscreenSwipe2DGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenSwipe2DGesture>(&d->variant);
}

const TouchscreenSwipeFromEdgeGesture *KGlobalShortcutTrigger::asTouchscreenSwipeFromEdgeGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenSwipeFromEdgeGesture>(&d->variant);
}

const TouchscreenPinchGesture *KGlobalShortcutTrigger::asTouchscreenPinchGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenPinchGesture>(&d->variant);
}

const TouchscreenRotateGesture *KGlobalShortcutTrigger::asTouchscreenRotateGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenRotateGesture>(&d->variant);
}

const TouchscreenHoldGesture *KGlobalShortcutTrigger::asTouchscreenHoldGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenHoldGesture>(&d->variant);
}

#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS // see kglobalshortcuttrigger.h
const PointerAxisGesture *KGlobalShortcutTrigger::asPointerAxisGesture() const
{
    d->deserialize();
    return std::get_if<PointerAxisGesture>(&d->variant);
}
#endif

const LineShapeGesture *KGlobalShortcutTrigger::asLineShapeGesture() const
{
    d->deserialize();
    return std::get_if<LineShapeGesture>(&d->variant);
}

QString KGlobalShortcutTrigger::serializedTriggerParams() const
{
    if (const auto *un = std::get_if<KGlobalShortcutTriggerPrivate::Uninitialized>(&d->variant)) {
        return un->serializedTriggerParams;
    }
    if (const auto *un = std::get_if<KGlobalShortcutTriggerPrivate::Unparseable>(&d->variant)) {
        return un->serializedTriggerParams;
    }
    if (const TouchpadSwipeGesture *g = asTouchpadSwipeGesture()) {
        auto directionStr = QString::fromLatin1(QMetaEnum::fromType<SwipeDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % directionStr;
    }
    if (const TouchpadSwipe2DGesture *g = asTouchpadSwipe2DGesture()) {
        return QString::number(g->fingerCount);
    }
    if (const TouchpadPinchGesture *g = asTouchpadPinchGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<PinchDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % direction;
    }
    if (const TouchpadRotateGesture *g = asTouchpadRotateGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<RotateDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % direction;
    }
    if (const TouchpadHoldGesture *g = asTouchpadHoldGesture()) {
        return QString::number(g->fingerCount) % ":"_L1 % QString::number(g->duration.count());
    }
    if (const ApproachScreenBorderGesture *g = asApproachScreenBorderGesture()) {
        return QString::fromLatin1(QMetaEnum::fromType<ScreenBorder>().valueToKey(static_cast<quint64>(g->border)));
    }
    if (const TouchscreenSwipeGesture *g = asTouchscreenSwipeGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<SwipeDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % direction;
    }
    if (const TouchscreenSwipe2DGesture *g = asTouchscreenSwipe2DGesture()) {
        return QString::number(g->fingerCount);
    }
    if (const TouchscreenSwipeFromEdgeGesture *g = asTouchscreenSwipeFromEdgeGesture()) {
        return QString::fromLatin1(QMetaEnum::fromType<EdgeSwipeDirection>().valueToKey(static_cast<quint64>(g->edge)));
    }
    if (const TouchscreenPinchGesture *g = asTouchscreenPinchGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<PinchDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % direction;
    }
    if (const TouchscreenRotateGesture *g = asTouchscreenRotateGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<RotateDirection>().valueToKey(static_cast<quint64>(g->direction)));
        return QString::number(g->fingerCount) % ":"_L1 % direction;
    }
    if (const TouchscreenHoldGesture *g = asTouchscreenHoldGesture()) {
        return QString::number(g->fingerCount) % ":"_L1 % QString::number(g->duration.count());
    }
#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS
    if (const PointerAxisGesture *g = asPointerAxisGesture()) {
        auto direction = QString::fromLatin1(QMetaEnum::fromType<PointerAxisDirection>().valueToKey(static_cast<quint64>(g->direction)));
        auto button = QString::fromLatin1(QMetaEnum::fromType<PointerAxisGesture::MouseButtonRequirement>().valueToKey(static_cast<quint64>(g->button)));
        return direction % ":"_L1 % button;
    }
#endif
    if (const LineShapeGesture *g = asLineShapeGesture()) {
        QString serialized;
        serialized.reserve(g->points.size() * 4); // at least one char each for x, y, point delim, axis delim

        if (!g->points.isEmpty()) {
            serialized += QString::number(g->points[0].x()) % ','_L1 % QString::number(g->points[0].y());
        }
        for (int i = 1; i < g->points.size(); ++i) {
            serialized += ';'_L1 % QString::number(g->points[i].x()) % ','_L1 % QString::number(g->points[i].y());
        }
    }

    Q_UNREACHABLE_RETURN(QString());
}

template<typename SwipeGestureClass>
static KGlobalShortcutTriggerPrivate::TriggerVariant parseSwipeGestureParams(const QString &serialized)
{
    QList<QStringView> params = QStringView(serialized).tokenize(u':').toContainer();
    if (params.size() < 2) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    bool ok = false;
    int fingerCount = params[0].toInt(&ok);
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    ok = false;
    auto direction = static_cast<SwipeDirection>(QMetaEnum::fromType<SwipeDirection>().keyToValue(params[1].toLatin1().data(), &ok));
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    return {SwipeGestureClass{.fingerCount = fingerCount, .direction = direction}};
}

template<typename PinchGestureClass>
static KGlobalShortcutTriggerPrivate::TriggerVariant parsePinchGestureParams(const QString &serialized)
{
    QList<QStringView> params = QStringView(serialized).tokenize(u':').toContainer();
    if (params.size() < 2) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    bool ok = false;
    int fingerCount = params[0].toInt(&ok);
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    ok = false;
    auto direction = static_cast<PinchDirection>(QMetaEnum::fromType<PinchDirection>().keyToValue(params[1].toLatin1().data(), &ok));
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{serialized}};
    }

    return {PinchGestureClass{.fingerCount = fingerCount, .direction = direction}};
}

void KGlobalShortcutTriggerPrivate::deserialize()
{
    const auto *uninitialized = std::get_if<KGlobalShortcutTriggerPrivate::Uninitialized>(&variant);
    if (!uninitialized) {
        return;
    }
    const QString &serialized = uninitialized->serializedTriggerParams;

    // Empty strings and key sequences are the common case, serialized without a prefix.
    // Gestures and any other triggers must be explicitly prefixed, e.g. "T:TouchpadSwipe:3:Up"
    if (serialized.isEmpty()) {
        variant = KGlobalShortcutTriggerPrivate::Unparseable{serialized};
        return;
    }

    if (triggerType == TouchpadSwipeString) {
        variant = parseSwipeGestureParams<TouchpadSwipeGesture>(serialized);
    } else if (triggerType == TouchpadPinchString) {
        variant = parsePinchGestureParams<TouchpadPinchGesture>(serialized);
    } else if (triggerType == TouchscreenSwipeString) {
        variant = parseSwipeGestureParams<TouchscreenSwipeGesture>(serialized);
    } else if (triggerType == TouchscreenPinchString) {
        variant = parsePinchGestureParams<TouchscreenPinchGesture>(serialized);
    } // TODO: parse more variants (and figure out activation requirements parsing)
    else {
        variant = KGlobalShortcutTriggerPrivate::Unparseable{serialized};
    }
}

#include "moc_kglobalshortcuttrigger.cpp"
