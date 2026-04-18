/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GLOBALSHORTCUT_H
#define GLOBALSHORTCUT_H

#include <KGlobalShortcutInfo>

class GlobalShortcutContext;
class GlobalShortcutsRegistry;
class KGlobalShortcutTrigger;

/**
 * Represents a global shortcut.
 *
 * @internal
 *
 * \note This class can handle multiple keys and gesture triggers (default and active).
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

    //! Returns a list of trigger types in both the default and current trigger lists.
    QStringList triggerTypes() const;

    //! Returns the default triggers of \a triggerType for this shortcut.
    //! \sa triggerTypes()
    QSet<KGlobalShortcutTrigger> defaultTriggers(const QString &triggerType) const;

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

    //! Returns a list of keys associated with this shortcut.
    QSet<QKeySequence> keys() const;

    //! Returns a list of triggers of \a triggerType associated with this shortcut.
    QSet<KGlobalShortcutTrigger> triggers(const QString &triggerType) const;

    //! Activates the shortcut. Its keys are grabbed, triggers are activated.
    void setActive();

    //! Sets the default keys for this shortcut.
    void setDefaultKeys(const QSet<QKeySequence> &);

    //! Sets the default triggers for this shortcut.
    void setDefaultTriggers(const QString &triggerType, const QSet<KGlobalShortcutTrigger> &);

    //! Sets the friendly name for the shortcut. For display.
    void setFriendlyName(const QString &);

    //! Sets the inverse action for this shortcut.
    void setInverseActionUniqueName(const QString &uniqueName);

    //! Sets the shortcut inactive. No longer grabs its keys or triggers.
    void setInactive();

    void setIsPresent(bool);
    void setIsFresh(bool);

    //! Sets the keys activated with this shortcut. The old keys are freed.
    void setKeys(const QSet<QKeySequence> &);

    enum AssignmentType {
        FromDefaults,
        FromOverride,
    };

    //! Sets the triggers \a triggerType activated with this shortcut.
    //! The old triggers of the same type are freed.
    void setTriggers(const QString &triggerType, const QSet<KGlobalShortcutTrigger> &, AssignmentType);

    //! Returns true if the default trigger assignments have been customized by the user.
    bool hasOverrideTriggerAssignments(const QString &triggerType) const;

    //! Returns the unique name aka id for the shortcuts.
    QString uniqueName() const;

    operator KGlobalShortcutInfo() const;

    //! Remove this shortcut and it's siblings
    void unRegister();

    //! The serial indicates the time when the shortcut has been registered.
    uint64_t serial() const;

private:
    //! means the associated application is present.
    bool _isPresent : 1;

    //! means the shortcut is registered with GlobalShortcutsRegistry
    bool _isRegistered : 1;

    //! means the shortcut is new
    bool _isFresh : 1;

    GlobalShortcutsRegistry *_registry = nullptr;

    //! The context the shortcut belongs too
    GlobalShortcutContext *_context = nullptr;

    QString _uniqueName;
    QString _friendlyName; // usually localized

    QSet<QKeySequence> _keys;
    QSet<QKeySequence> _defaultKeys;
    uint64_t _serial;

    struct TriggerLists {
        QSet<KGlobalShortcutTrigger> defaults;
        QSet<KGlobalShortcutTrigger> assigned;
        bool hasOverrideAssignments = false;
    };
    QHash<QString, TriggerLists> _triggers; // per trigger type

    QString _inverseActionUniqueName;
};

#endif /* #ifndef GLOBALSHORTCUT_H */
