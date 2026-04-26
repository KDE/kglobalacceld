/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcuttrigger.h"

#include <QMetaEnum>
#include <QtAssert>

#include <variant>

using namespace Qt::StringLiterals;
using namespace KGlobalShortcutTriggerTypes;

static const QString TouchpadSwipeString = "TouchpadSwipe"_L1;
static const QString TouchpadPinchString = "TouchpadPinch"_L1;
static const QString TouchscreenSwipeString = "TouchscreenSwipe"_L1;
static const QString TouchscreenPinchString = "TouchscreenPinch"_L1;
static const QString LineShapeString = "LineShape"_L1;

namespace
{
QString combineTypeAndParamStrings(const QString &triggerType, const QString &triggerParamString)
{
    if (triggerType.isEmpty() && triggerParamString.isEmpty()) {
        return QString();
    }
    QString combined = triggerType;
    combined.reserve(triggerType.size() + 1 + triggerParamString.size());
    combined += "@"_L1;
    combined += triggerParamString;
    return combined;
}

template<class Variant>
QPair<QString, QString> serializeParams(const Variant &trigger);
} // namespace

class KGlobalShortcutTriggerPrivate
{
public:
    struct Uninitialized {
    };
    struct Unparseable {
    };
    using TriggerVariant = std::variant<Uninitialized, // have not yet tried to deserialize
                                        Unparseable, // have tried to deserialize and failed
                                        TouchpadSwipeGesture,
                                        TouchpadPinchGesture,
                                        TouchscreenSwipeGesture,
                                        TouchscreenPinchGesture,
                                        LineShapeGesture>;
    QString serialized; // triggerType + delimiter character + triggerParamString
    int serializedTriggerTypeLength;
    TriggerVariant variant;

public:
    template<class Variant = Uninitialized>
    KGlobalShortcutTriggerPrivate(const QString &serialized, int serializedTriggerTypeLength, Variant &&trigger)
        : serialized(serialized)
        , serializedTriggerTypeLength(serializedTriggerTypeLength)
        , variant(trigger)
    {
    }

    template<class Variant = Uninitialized>
    KGlobalShortcutTriggerPrivate(Variant &&trigger)
        : variant(trigger)
    {
        const auto triggerTypeAndParams = serializeParams(trigger);
        serializedTriggerTypeLength = triggerTypeAndParams.first.size();
        serialized = combineTypeAndParamStrings(triggerTypeAndParams.first, triggerTypeAndParams.second);
    }

    QStringView paramStringView() const
    {
        // return the part after the type/params delimiter
        return serializedTriggerTypeLength < serialized.size() ? QStringView(serialized).sliced(serializedTriggerTypeLength + 1) : QStringView();
    }

    void deserialize();
};

//
// KGlobalShortcutTrigger

KGlobalShortcutTrigger::KGlobalShortcutTrigger()
    : d(new KGlobalShortcutTriggerPrivate(QString(), 0, KGlobalShortcutTriggerPrivate::Unparseable{}))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const QString &triggerType, const QString &triggerParamString)
    : d(new KGlobalShortcutTriggerPrivate( //
          combineTypeAndParamStrings(triggerType, triggerParamString),
          triggerType.size(),
          KGlobalShortcutTriggerPrivate::Uninitialized{}))
{
    // variant parsing will happen later in KGlobalShortcutTriggerPrivate::deserialize()
}

KGlobalShortcutTrigger::~KGlobalShortcutTrigger()
{
    delete d;
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const KGlobalShortcutTrigger &rhs)
    : d(new KGlobalShortcutTriggerPrivate(rhs.d->serialized, rhs.d->serializedTriggerTypeLength, rhs.d->variant))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const QString &triggerId)
    : d(new KGlobalShortcutTriggerPrivate(triggerId, std::max(triggerId.indexOf("@"_L1), 0ll), KGlobalShortcutTriggerPrivate::Uninitialized{}))
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

bool KGlobalShortcutTrigger::isEmpty() const
{
    return d->serializedTriggerTypeLength == 0;
}

QString KGlobalShortcutTrigger::type() const
{
    return d->serialized.first(d->serializedTriggerTypeLength);
}

QString KGlobalShortcutTrigger::paramString() const
{
    return d->paramStringView().toString();
}

QString KGlobalShortcutTrigger::toString() const
{
    return d->serialized;
}

KGlobalShortcutTrigger KGlobalShortcutTrigger::fromString(const QString &triggerString)
{
    return KGlobalShortcutTrigger(triggerString);
}

bool KGlobalShortcutTrigger::isKnownTriggerType() const
{
    d->deserialize();
    return !std::holds_alternative<KGlobalShortcutTriggerPrivate::Unparseable>(d->variant);
}

bool KGlobalShortcutTrigger::operator==(const KGlobalShortcutTrigger &rhs) const
{
    return d->serialized == rhs.d->serialized;
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

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadSwipeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchpadPinchGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenSwipeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const TouchscreenPinchGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(sc))
{
}

KGlobalShortcutTrigger::KGlobalShortcutTrigger(const LineShapeGesture &sc)
    : d(new KGlobalShortcutTriggerPrivate(sc))
{
}

const TouchpadSwipeGesture *KGlobalShortcutTrigger::asTouchpadSwipeGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadSwipeGesture>(&d->variant);
}

const TouchpadPinchGesture *KGlobalShortcutTrigger::asTouchpadPinchGesture() const
{
    d->deserialize();
    return std::get_if<TouchpadPinchGesture>(&d->variant);
}

const TouchscreenSwipeGesture *KGlobalShortcutTrigger::asTouchscreenSwipeGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenSwipeGesture>(&d->variant);
}

const TouchscreenPinchGesture *KGlobalShortcutTrigger::asTouchscreenPinchGesture() const
{
    d->deserialize();
    return std::get_if<TouchscreenPinchGesture>(&d->variant);
}

const LineShapeGesture *KGlobalShortcutTrigger::asLineShapeGesture() const
{
    d->deserialize();
    return std::get_if<LineShapeGesture>(&d->variant);
}

namespace
{
template<>
QPair<QString, QString> serializeParams(const TouchpadSwipeGesture &g)
{
    auto directionStr = QString::fromLatin1(QMetaEnum::fromType<SwipeDirection>().valueToKey(static_cast<quint64>(g.direction)));
    return {TouchpadSwipeString, QString::number(g.fingerCount) % ":"_L1 % directionStr};
}

template<>
QPair<QString, QString> serializeParams(const TouchscreenSwipeGesture &g)
{
    auto directionStr = QString::fromLatin1(QMetaEnum::fromType<SwipeDirection>().valueToKey(static_cast<quint64>(g.direction)));
    return {TouchscreenSwipeString, QString::number(g.fingerCount) % ":"_L1 % directionStr};
}

template<typename SwipeGestureClass>
static KGlobalShortcutTriggerPrivate::TriggerVariant parseSwipeGestureParams(const QStringView &serializedParams)
{
    QList<QStringView> params = serializedParams.tokenize(u':').toContainer();
    if (params.size() < 2) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    bool ok = false;
    int fingerCount = params[0].toInt(&ok);
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    ok = false;
    auto direction = static_cast<SwipeDirection>(QMetaEnum::fromType<SwipeDirection>().keyToValue(params[1].toLatin1().data(), &ok));
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    return {SwipeGestureClass{.fingerCount = fingerCount, .direction = direction}};
}

template<>
QPair<QString, QString> serializeParams(const TouchpadPinchGesture &g)
{
    auto direction = QString::fromLatin1(QMetaEnum::fromType<PinchDirection>().valueToKey(static_cast<quint64>(g.direction)));
    return {TouchpadPinchString, QString::number(g.fingerCount) % ":"_L1 % direction};
}

template<>
QPair<QString, QString> serializeParams(const TouchscreenPinchGesture &g)
{
    auto direction = QString::fromLatin1(QMetaEnum::fromType<PinchDirection>().valueToKey(static_cast<quint64>(g.direction)));
    return {TouchscreenPinchString, QString::number(g.fingerCount) % ":"_L1 % direction};
}

template<typename PinchGestureClass>
static KGlobalShortcutTriggerPrivate::TriggerVariant parsePinchGestureParams(const QStringView &serializedParams)
{
    QList<QStringView> params = serializedParams.tokenize(u':').toContainer();
    if (params.size() < 2) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    bool ok = false;
    int fingerCount = params[0].toInt(&ok);
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    ok = false;
    auto direction = static_cast<PinchDirection>(QMetaEnum::fromType<PinchDirection>().keyToValue(params[1].toLatin1().data(), &ok));
    if (!ok) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    return {PinchGestureClass{.fingerCount = fingerCount, .direction = direction}};
}

template<>
QPair<QString, QString> serializeParams(const LineShapeGesture &g)
{
    QString serialized;
    serialized.reserve(g.points.size() * 5); // at least one char each for x, y, point delim, 2 parentheses
    for (const QPointF &p : g.points) {
        serialized += '('_L1 % QString::number(p.x()) % ';'_L1 % QString::number(p.y()) % ')'_L1;
    }
    return {LineShapeString, serialized};
}

static KGlobalShortcutTriggerPrivate::TriggerVariant parseLineShapeParams(const QStringView &serialized)
{
    // e.g. "(x1;y1)(x2;y2)(x3;y3)"
    QList<QPointF> points;
    bool okX = false;
    bool okY = false;

    if (serialized.size() < 2 || serialized[0] != '('_L1 || serialized[serialized.size() - 1] != ')'_L1) {
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }

    for (const QStringView &pointString : serialized.sliced(1, serialized.size() - 2).tokenize(")("_L1)) {
        QList<QStringView> numStrings = pointString.split(';'_L1);
        if (numStrings.size() != 2) { // x, y
            return {KGlobalShortcutTriggerPrivate::Unparseable{}};
        }
        qreal x = numStrings[0].toDouble(&okX);
        qreal y = numStrings[1].toDouble(&okY);
        if (!okX || !okY) {
            return {KGlobalShortcutTriggerPrivate::Unparseable{}};
        }
        points.emplaceBack(x, y);
    }

    if (points.size() < 2) { // need at least source and destination points
        return {KGlobalShortcutTriggerPrivate::Unparseable{}};
    }
    return {LineShapeGesture{.points = points}};
}
} // namespace

void KGlobalShortcutTriggerPrivate::deserialize()
{
    const auto *uninitialized = std::get_if<KGlobalShortcutTriggerPrivate::Uninitialized>(&variant);
    if (!uninitialized) {
        return;
    }

    if (serialized.isEmpty()) {
        variant = KGlobalShortcutTriggerPrivate::Unparseable{};
        return;
    }

    auto triggerType = QStringView(serialized).first(serializedTriggerTypeLength);

    if (triggerType == TouchpadSwipeString) {
        variant = parseSwipeGestureParams<TouchpadSwipeGesture>(paramStringView());
    } else if (triggerType == TouchpadPinchString) {
        variant = parsePinchGestureParams<TouchpadPinchGesture>(paramStringView());
    } else if (triggerType == TouchscreenSwipeString) {
        variant = parseSwipeGestureParams<TouchscreenSwipeGesture>(paramStringView());
    } else if (triggerType == TouchscreenPinchString) {
        variant = parsePinchGestureParams<TouchscreenPinchGesture>(paramStringView());
    } else if (triggerType == LineShapeString) {
        variant = parseLineShapeParams(paramStringView());
    } // TODO 6.8: parse more variants
    else {
        variant = KGlobalShortcutTriggerPrivate::Unparseable{};
    }
}

#include "moc_kglobalshortcuttrigger.cpp"
