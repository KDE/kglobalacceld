/*
    SPDX-FileCopyrightText: 2026 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTTRIGGER_P_H
#define KGLOBALSHORTCUTTRIGGER_P_H

/**
 * @internal
 */

#include "kglobalshortcuttrigger.h"

#include <variant>

class KGlobalShortcutTriggerPrivate
{
public:
    struct Uninitialized {
        QString serializedTriggerParams;
    };
    struct Unparseable {
        QString serializedTriggerParams;
    };
    using TriggerVariant = std::variant<Uninitialized,
                                        Unparseable,
                                        KGlobalShortcutTriggerTypes::TouchpadSwipeGesture,
                                        KGlobalShortcutTriggerTypes::TouchpadSwipe2DGesture,
                                        KGlobalShortcutTriggerTypes::TouchpadPinchGesture,
                                        KGlobalShortcutTriggerTypes::TouchpadRotateGesture,
                                        KGlobalShortcutTriggerTypes::TouchpadHoldGesture,
                                        KGlobalShortcutTriggerTypes::ApproachScreenBorderGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenSwipeGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenSwipe2DGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenSwipeFromEdgeGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenPinchGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenRotateGesture,
                                        KGlobalShortcutTriggerTypes::TouchscreenHoldGesture,
#ifdef WE_HAVE_A_PLAN_FOR_ACTIVATION_REQUIREMENTS // see kglobalshortcuttrigger.h
                                        KGlobalShortcutTriggerTypes::PointerAxisGesture,
#endif
                                        KGlobalShortcutTriggerTypes::LineShapeGesture>;
    QString triggerType;
    TriggerVariant variant;

public:
    template<class Variant = Uninitialized>
    KGlobalShortcutTriggerPrivate(const QString &triggerType, Variant &&trigger)
        : triggerType(triggerType)
        , variant(trigger)
    {
    }

    void deserialize();
};

#endif /* #ifndef KGLOBALSHORTCUTTRIGGER_P_H */
