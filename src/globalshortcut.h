/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GLOBALSHORTCUT_H
#define GLOBALSHORTCUT_H

#include <KGlobalShortcutInfo>

class GlobalShortcutContext;
class GlobalShortcutsRegistry;

/**
 * Represents a global shortcut.
 *
 * @internal
 *
 * \note This class can handle multiple keys (default and active). This
 * feature isn't used currently. kde4 only allows setting one key per global
 * shortcut.
 *
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class GlobalShortcut
{
public:
    GlobalShortcut(const QString &uniqueName, const QString &friendlyName, uint64_t serial, GlobalShortcutContext *context, GlobalShortcutsRegistry *registry);

    ~GlobalShortcut();

    //! Returns the context the shortcuts belongs to
    GlobalShortcutContext *context();
    GlobalShortcutContext const *context() const;

    //! Returns the default keys for this shortcut.
    QSet<QKeySequence> defaultKeys() const;

    //! Return the friendly display name for this shortcut.
    QString friendlyName() const;

    //! Returns the unique name of the associated inverse action, or empty if not paired.
    QString inverseActionUniqueName() const;

    //! Check if the shortcut is active. Its keys are grabbed
    bool isActive() const;

    //! Check if the shortcut is fresh/new. Is an internal state
    bool isFresh() const;

    //! Check if the shortcut is present. It application is running.
    bool isPresent() const;

    //! Returns true if the shortcut is a session shortcut
    bool isSessionShortcut() const;

    //! Returns true if the shortcut's effect will be active only from press to release event.
    bool isMomentary() const;

    //! Returns true if the key event should be passed through, false if consumed
    bool shouldPassThroughKeyEvent() const;

    //! Returns a list of keys associated with this shortcut.
    QSet<QKeySequence> keys() const;

    //! Activates the shortcut. The keys are grabbed.
    void setActive();

    //! Sets the default keys for this shortcut.
    void setDefaultKeys(const QSet<QKeySequence> &);

    //! Sets the friendly name for the shortcut. For display.
    void setFriendlyName(const QString &);

    //! Sets the inverse action for this shortcut.
    void setInverseActionUniqueName(const QString &uniqueName);

    //! Sets the shortcut inactive. No longer grabs the keys.
    void setInactive();

    void setIsPresent(bool);
    void setIsFresh(bool);

    //! Sets the keys activated with this shortcut. The old keys are freed.
    void setKeys(const QSet<QKeySequence> &);

    //! Marks the shortcut as momentary. Its effect will be active only from press to release event.
    void setMomentary(bool);

    //! Sets whether the key event should be passed through to subsequent key handlers. Default is false (i.e. key event is consumed).
    void setPassThroughKeyEvent(bool);

    //! Returns the unique name aka id for the shortcuts.
    QString uniqueName() const;

    operator KGlobalShortcutInfo() const;

    //! Remove this shortcut and it's siblings
    void unRegister();

    //! The serial indicates the time when the shortcut has been registered.
    uint64_t serial() const;

    //! D-Bus clients whose application owns the shortcut and is running
    QSet<QString> clients;

private:
    //! means the associated application is present.
    bool _isPresent : 1;

    //! means the shortcut is registered with GlobalShortcutsRegistry
    bool _isRegistered : 1;

    //! means the shortcut is new
    bool _isFresh : 1;

    bool _isMomentary : 1 = false;

    bool _shouldPassThroughKeyEvent : 1 = false;

    GlobalShortcutsRegistry *_registry = nullptr;

    //! The context the shortcut belongs too
    GlobalShortcutContext *_context = nullptr;

    QString _uniqueName;
    QString _friendlyName; // usually localized

    QSet<QKeySequence> _keys;
    QSet<QKeySequence> _defaultKeys;
    uint64_t _serial;

    QString _inverseActionUniqueName;
};

#endif /* #ifndef GLOBALSHORTCUT_H */
