/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTTRIGGER_H
#define KGLOBALSHORTCUTTRIGGER_H

#include <kglobalacceld_export.h>

#include <QList>
#include <QObject>
#include <QPointF>
#include <QString>

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

enum class PinchDirection : unsigned int {
    Expanding = 0,
    Contracting,
};
Q_ENUM_NS(PinchDirection)

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
};

class KGLOBALACCELD_EXPORT TouchpadPinchGesture
{
    Q_GADGET
public:
    int fingerCount;
    PinchDirection direction;
};

class KGLOBALACCELD_EXPORT TouchscreenSwipeGesture
{
    Q_GADGET
public:
    int fingerCount;
    SwipeDirection direction;
};

class KGLOBALACCELD_EXPORT TouchscreenPinchGesture
{
    Q_GADGET
public:
    int fingerCount;
    PinchDirection direction;
};

class KGLOBALACCELD_EXPORT LineShapeGesture
{
    Q_GADGET
public:
    QList<QPointF> points;
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

    //! Create a touchpad pinch gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchpadPinchGesture &);

    //! Create a touchscreen swipe gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenSwipeGesture &);

    //! Create a touchscreen pinch gesture trigger.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::TouchscreenPinchGesture &);

    //! Create a line shape gesture trigger, also known as mouse gesture.
    KGlobalShortcutTrigger(const KGlobalShortcutTriggerTypes::LineShapeGesture &);

    /*!
     * Create a trigger value from a strings that were previously exported via type() and paramString().
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
     * \sa paramString()
     */
    explicit KGlobalShortcutTrigger(const QString &triggerType, const QString &triggerParamString);

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
     * \sa paramString()
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
    QString paramString() const;

    /*!
     * Returns a string that uniquely, deterministically identifies this trigger.
     *
     * Can be used as key in maps and hashes, and to construct an equal trigger object
     * using KGlobalShortcutTrigger::fromString().
     */
    QString toString() const;

    /*!
     * Reconstructs a trigger object that was previously returned from toString().
     */
    static KGlobalShortcutTrigger fromString(const QString &triggerId);

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

    const KGlobalShortcutTriggerTypes::TouchpadSwipeGesture *asTouchpadSwipeGesture() const;
    const KGlobalShortcutTriggerTypes::TouchpadPinchGesture *asTouchpadPinchGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenSwipeGesture *asTouchscreenSwipeGesture() const;
    const KGlobalShortcutTriggerTypes::TouchscreenPinchGesture *asTouchscreenPinchGesture() const;
    const KGlobalShortcutTriggerTypes::LineShapeGesture *asLineShapeGesture() const;

    bool operator==(const KGlobalShortcutTrigger &rhs) const;

private:
    //! use KGlobalShortcutTrigger::fromString() instead, for clarity
    explicit KGlobalShortcutTrigger(const QString &triggerString);

    //! Implementation details
    KGlobalShortcutTriggerPrivate *d;
};

template<>
struct std::hash<KGlobalShortcutTrigger> {
    std::size_t operator()(const KGlobalShortcutTrigger &t) const noexcept
    {
        return std::hash<QString>{}(t.toString());
    }
};

Q_DECLARE_METATYPE(KGlobalShortcutTrigger)

#endif /* #ifndef KGLOBALSHORTCUTTRIGGER_H */
