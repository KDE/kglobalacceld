/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTTRIGGER_H
#define KGLOBALSHORTCUTTRIGGER_H

#include <kglobalacceld_export.h>

// #include <QDBusArgument> // TODO: implement or remove
#include <QList>
#include <QObject>
#include <QPointF>

#include <chrono>
#include <optional>

// This file contains a class to represent generic input triggers,
// as well as a number of concrete trigger types with their respective parameters.
// Eventually this may move from KGlobalAccelD (plasma) to KGlobalAccel (frameworks).
// In the meantime though, being part of plasma avoids backwards compatibility responsibilities.
// As long as only KWin and System Settings use this, there's no need to lock it in as public API.

namespace KGlobalShortcutTriggerTypes
{
Q_NAMESPACE_EXPORT(KGLOBALACCELD_EXPORT)

enum class SwipeDirection : unsigned int {
    Left = 0,
    UpLeft,
    Up,
    UpRight,
    Right,
    DownRight,
    Down,
    DownLeft,
};
Q_ENUM_NS(SwipeDirection)

enum class EdgeSwipeDirection : unsigned int {
    FromLeft = 0,
    FromTopLeft,
    FromTop,
    FromTopRight,
    FromRight,
    FromBottomRight,
    FromBottom,
    FromBottomLeft,
};
Q_ENUM_NS(EdgeSwipeDirection)

enum class PinchDirection : unsigned int {
    Expanding = 0,
    Contracting,
};
Q_ENUM_NS(PinchDirection)

enum class RotateDirection : unsigned int {
    Clockwise = 0,
    CounterClockwise,
};
Q_ENUM_NS(RotateDirection)

enum class ScreenBorder : unsigned int {
    Left = 0,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
};
Q_ENUM_NS(ScreenBorder)

enum class PointerAxisDirection : unsigned int {
    Down = 0,
    Left,
    Up,
    Right,
};
Q_ENUM_NS(PointerAxisDirection)

// A KeyboardShortcut class is deliberately left out from the set of trigger types.
// It may be a useful addition so that all kinds of triggers can be represented in the same way,
// but the existing KGlobalAccel API is heavy on `QList<QKeySequence> keys`. Pairing these key lists
// with trigger lists can get awkward if now you have to look for keys in both lists.

class KGLOBALACCELD_EXPORT TouchpadSwipeGesture
{
    Q_GADGET
public:
    int fingerCount;
    SwipeDirection direction;

    // explicit TouchpadSwipeGesture(int fingerCount, SwipeDirection);
};

class KGLOBALACCELD_EXPORT TouchpadSwipe2DGesture
{
    Q_GADGET
public:
    int fingerCount;

    // explicit TouchpadSwipe2DGesture(int fingerCount);
};

class KGLOBALACCELD_EXPORT TouchpadPinchGesture
{
    Q_GADGET
public:
    int fingerCount;
    PinchDirection direction;

    // explicit TouchpadPinchGesture(int fingerCount, PinchDirection);
};

class KGLOBALACCELD_EXPORT TouchpadRotateGesture
{
    Q_GADGET
public:
    int fingerCount;
    RotateDirection direction;

    // explicit TouchpadRotateGesture(int fingerCount, RotateDirection);
};

class KGLOBALACCELD_EXPORT TouchpadHoldGesture
{
    Q_GADGET
public:
    int fingerCount;
    std::chrono::milliseconds duration;

    // explicit TouchpadHoldGesture(int fingerCount, std::chrono::milliseconds duration);
};

class KGLOBALACCELD_EXPORT ApproachScreenBorderGesture
{
    Q_GADGET
public:
    ScreenBorder border;

    // explicit ApproachScreenBorderGesture(ScreenBorder);
};

class KGLOBALACCELD_EXPORT TouchscreenSwipeGesture
{
    Q_GADGET
public:
    int fingerCount;
    SwipeDirection direction;

    // explicit TouchscreenSwipeGesture(int fingerCount, SwipeDirection);
};

class KGLOBALACCELD_EXPORT TouchscreenSwipe2DGesture
{
    Q_GADGET
public:
    int fingerCount;

    // explicit TouchscreenSwipe2DGesture(int fingerCount);
};

class KGLOBALACCELD_EXPORT TouchscreenSwipeFromEdgeGesture
{
    Q_GADGET
public:
    EdgeSwipeDirection edge;

    // explicit TouchscreenSwipeFromEdgeGesture(EdgeSwipeDirection);
};

class KGLOBALACCELD_EXPORT TouchscreenPinchGesture
{
    Q_GADGET
public:
    int fingerCount;
    PinchDirection direction;

    // explicit TouchscreenPinchGesture(int fingerCount, PinchDirection);
};

class KGLOBALACCELD_EXPORT TouchscreenRotateGesture
{
    Q_GADGET
public:
    int fingerCount;
    RotateDirection direction;

    // explicit TouchscreenRotateGesture(int fingerCount, RotateDirection);
};

class KGLOBALACCELD_EXPORT TouchscreenHoldGesture
{
    Q_GADGET
public:
    int fingerCount;
    std::chrono::milliseconds duration;

    // explicit TouchscreenHoldGesture(int fingerCount, std::chrono::milliseconds duration);
};

// Pointer axis gestures are supported by KWin and should be added as a trigger type, more or less
// like below. It's the only current "gesture" with a modifier key requirement. We could add
// a field for modifier keys right here in this PointerAxisGesture class, but ideally we'll
// support modifier requirements (and other conditions, working title "activation requirements")
// for all trigger types with unified syntax and centralized parsing. This is not included in the
// initial changes for gesture configuration, so we'll leave out PointerAxisGesture until
// activation requirements are properly defined and implemented.
//
#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS
class KGLOBALACCELD_EXPORT PointerAxisGesture
{
    Q_GADGET
public:
    enum class MouseButtonRequirement : unsigned int {
        NoButton,
        ActivationButton,
    };
    Q_ENUM(MouseButtonRequirement)

    PointerAxisDirection direction;
    MouseButtonRequirement button;

    // explicit PointerAxisGesture(PointerAxisDirection, MouseButtonRequirement);
};
#endif

class KGLOBALACCELD_EXPORT LineShapeGesture
{
    Q_GADGET
public:
    QList<QPointF> points;

    // explicit LineShapeGesture(QList<QPointF> points);
};

} // namespace KGlobalShortcutTriggerTypes

class KGlobalShortcutTriggerPrivate;

/*!
 * \class KGlobalShortcutTrigger
 * \inmodule KGlobalAccel
 * \brief Description of an event that can trigger an action, such as a keyboard shortcut or touch gesture.
 */
class KGLOBALACCELD_EXPORT KGlobalShortcutTrigger
{
public:
    //! Create an empty trigger value, i.e. \c {isEmpty() == true}.
    KGlobalShortcutTrigger();

    ~KGlobalShortcutTrigger();

    KGlobalShortcutTrigger(const KGlobalShortcutTrigger &rhs);
    KGlobalShortcutTrigger &operator=(const KGlobalShortcutTrigger &rhs);

    //! Create a touchpad swipe gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadSwipeGesture &);

    //! Create a freeform 2D touchpad swipe gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadSwipe2DGesture &);

    //! Create a touchpad pinch gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadPinchGesture &);

    //! Create a touchpad rotate gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadRotateGesture &);

    //! Create a touchpad hold gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadHoldGesture &);

    //! Create a gesture trigger for approaching a screen border with the pointer.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::ApproachScreenBorderGesture &);

    //! Create a touchscreen swipe gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenSwipeGesture &);

    //! Create a freeform 2D touchscreen swipe gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenSwipe2DGesture &);

    //! Create a touchscreen swipe-from-edge gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenSwipeFromEdgeGesture &);

    //! Create a touchscreen pinch gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenPinchGesture &);

    //! Create a touchscreen rotate gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenRotateGesture &);

    //! Create a touchscreen hold gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenHoldGesture &);

#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS
    //! Create a pointer axis gesture trigger, most commonly actived using a scroll wheel.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::PointerAxisGesture &);
#endif

    //! Create a line shape gesture trigger, also known as mouse gesture.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::LineShapeGesture &);

    /*!
     * Create a trigger value from a strings that were previously exported via type()
     * and serializedTriggerParams().
     *
     * Passing an empty trigger type will produce an empty KGlobalShortcutTrigger value,
     * i.e. \c {isEmpty() == true}.
     *
     * A non-empty trigger type will produce a non-empty KGlobalShortcutTrigger, regardless of
     * whether the string can be interpreted by this version of KGlobalAccel. The trigger type
     * and params are retained verbatim so they can be written back unmodified to a config file
     * even if they cannot be parsed.
     *
     * \sa isEmpty()
     * \sa type()
     * \sa serializedTriggerParams()
     */
    explicit KGlobalShortcutTrigger(const QString &triggerType, const QString &serializedTriggerParams);

    /*!
     * Returns true if created with the parameter-less default constructor, or from an empty string
     * as trigger type.
     */
    bool isEmpty() const;

    /*!
     * Returns the trigger type as determined by the constructor.
     *
     * This string can be passed to the constructor together with serialized trigger parameters
     * to recreate an equivalent new trigger object.
     *
     * If this object was created with its string pair constructor, but the trigger type is
     * unsupported or the trigger parameters cannot be parsed, then this function will still
     * return the provided trigger type string as it was given to the constructor.
     * However, isKnownTriggerType() will return false and all trigger variant object accessors
     * (e.g. asTouchpadSwipeGesture()) will return nullptr. The serialized strings may still be
     * valid but generated by a later version of KGlobalShortcutTrigger.
     *
     * \sa serializedTriggerParams()
     */
    QString type() const;

    /*!
     * Returns a string that describes how and when to activate this trigger, in a format specific
     * to this object's trigger type.
     *
     * This string can be passed to the constructor together with the trigger type to recreate
     * an equivalent new trigger object.
     *
     * If this object was created with its string pair constructor, but the trigger type is
     * unsupported or the trigger parameters cannot be parsed, then this function will still
     * return the provided trigger parameter string as it was given to the constructor.
     * However, isKnownTriggerType() will return false and all trigger variant object accessors
     * (e.g. asTouchpadSwipeGesture()) will return nullptr. The serialized strings may still be
     * valid but generated by a later version of KGlobalShortcutTrigger.
     *
     * \sa type()
     */
    QString serializedTriggerParams() const;

    /*!
     * Returns true if the trigger type and all of its parameters are known by this version
     * of KGlobalShortcutTrigger.
     *
     * If this returns true, the corresponding trigger variant object accessor
     * (e.g. asTouchpadSwipeGesture()) will return a valid pointer to an object that exposes
     * all parameters in a typesafe manner.
     *
     * Returns false for an empty or unknown trigger type. Also returns false if the object was
     * initialized from a serialized trigger parameter string that this version of
     * KGlobalShortcutTrigger cannot parse.
     *
     * If this function returns false, the serialized strings may still be valid but may have been
     * generated by a later version of KGlobalShortcutTrigger.
     */
    bool isKnownTriggerType() const;

    /*!
     * Return true if this trigger should not be active if the \a other one is already
     * active in the same context.
     *
     * \sa KGlobalAccel::MatchType
     */
    bool conflictsWith(const KGlobalShortcutTrigger &other) const;

    /*!
     * Return a trigger that inverses this one into the opposite direction.
     *
     * If this trigger has no notion of an inverse, return std::nullopt.
     */
    std::optional<KGlobalShortcutTrigger> inverse();

    const KGlobalShortcutTriggerTypes::TouchpadSwipeGesture *asTouchpadSwipeGesture() const;
    const KGlobalShortcutTriggerTypes::TouchpadSwipe2DGesture *asTouchpadSwipe2DGesture() const;
    const KGlobalShortcutTriggerTypes::TouchpadPinchGesture *asTouchpadPinchGesture() const;
    const KGlobalShortcutTriggerTypes::TouchpadRotateGesture *asTouchpadRotateGesture() const;
    const KGlobalShortcutTriggerTypes::TouchpadHoldGesture *asTouchpadHoldGesture() const;
    const KGlobalShortcutTriggerTypes::ApproachScreenBorderGesture *asApproachScreenBorderGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenSwipeGesture *asTouchscreenSwipeGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenSwipe2DGesture *asTouchscreenSwipe2DGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenSwipeFromEdgeGesture *asTouchscreenSwipeFromEdgeGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenPinchGesture *asTouchscreenPinchGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenRotateGesture *asTouchscreenRotateGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenHoldGesture *asTouchscreenHoldGesture() const;
#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS
    const KGlobalShortcutTriggerTypes::PointerAxisGesture *asPointerAxisGesture() const;
#endif
    const KGlobalShortcutTriggerTypes::LineShapeGesture *asLineShapeGesture() const;

    bool operator==(const KGlobalShortcutTrigger &rhs) const;

private:
    // friend KGLOBALACCELD_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutTrigger &shortcut);

    //! Implementation details
    KGlobalShortcutTriggerPrivate *d;
};

// KGLOBALACCELD_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutTrigger &trigger);
// KGLOBALACCELD_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutTrigger &trigger);

Q_DECLARE_METATYPE(KGlobalShortcutTrigger)

#endif /* #ifndef KGLOBALSHORTCUTTRIGGER_H */
