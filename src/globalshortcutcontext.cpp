/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "globalshortcutcontext.h"

#include "globalshortcut.h"

#include "kglobalaccel.h"
#include "sequencehelpers_p.h"

GlobalShortcutContext::GlobalShortcutContext(const QString &uniqueName, const QString &friendlyName, Component *component)

    : _uniqueName(uniqueName)
    , _friendlyName(friendlyName)
    , _component(component)
{
}

GlobalShortcutContext::~GlobalShortcutContext()
{
    qDeleteAll(_actionsMap);
    _actionsMap.clear();
}

void GlobalShortcutContext::addShortcut(GlobalShortcut *shortcut)
{
    _actionsMap.insert(shortcut->uniqueName(), shortcut);
}

QList<KGlobalShortcutInfo> GlobalShortcutContext::allShortcutInfos() const
{
    QList<KGlobalShortcutInfo> rc;
    for (GlobalShortcut *shortcut : std::as_const(_actionsMap)) {
        rc.append(static_cast<KGlobalShortcutInfo>(*shortcut));
    }
    return rc;
}

Component const *GlobalShortcutContext::component() const
{
    return _component;
}

Component *GlobalShortcutContext::component()
{
    return _component;
}

QString GlobalShortcutContext::friendlyName() const
{
    return _friendlyName;
}

GlobalShortcut *GlobalShortcutContext::getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    const auto shortcuts = getShortcutsByKey(key, type);
    if (shortcuts.isEmpty()) {
        return nullptr;
    } else {
        return shortcuts.first();
    }
}

QList<GlobalShortcut *> GlobalShortcutContext::getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    if (key.isEmpty()) {
        return {};
    }

    QList<GlobalShortcut *> ret;
    const QKeySequence keyMangled = Utils::normalizeSequence(key);
    for (GlobalShortcut *sc : std::as_const(_actionsMap)) {
        const auto keys = sc->keys();
        for (const QKeySequence &other : keys) {
            const QKeySequence otherMangled = Utils::normalizeSequence(other);
            switch (type) {
            case KGlobalAccel::MatchType::Equal:
                if (otherMangled == keyMangled) {
                    ret.append(sc);
                }
                break;
            case KGlobalAccel::MatchType::Shadows:
                if (!other.isEmpty() && Utils::contains(keyMangled, otherMangled)) {
                    ret.append(sc);
                }
                break;
            case KGlobalAccel::MatchType::Shadowed:
                if (!other.isEmpty() && Utils::contains(otherMangled, keyMangled)) {
                    ret.append(sc);
                }
                break;
            }
        }
    }

    return ret;
}

GlobalShortcut *GlobalShortcutContext::takeShortcut(GlobalShortcut *shortcut)
{
    // Try to take the shortcut. Result could be nullptr if the shortcut doesn't
    // belong to this component.
    return _actionsMap.take(shortcut->uniqueName());
}

QString GlobalShortcutContext::uniqueName() const
{
    return _uniqueName;
}

bool GlobalShortcutContext::isShortcutAvailable(const QKeySequence &key) const
{
    for (auto it = _actionsMap.cbegin(), endIt = _actionsMap.cend(); it != endIt; ++it) {
        const GlobalShortcut *sc = it.value();
        if (Utils::matchSequences(key, sc->keys())) {
            return false;
        }
    }
    return true;
}
