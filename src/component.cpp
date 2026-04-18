/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "component.h"

#include "globalshortcutcontext.h"
#include "globalshortcutsregistry.h"
#include "logging.h"
#include "sequencehelpers_p.h"

#include <KGlobalShortcutTrigger>

#include <QKeySequence>
#include <QStringList>

using namespace Qt::StringLiterals;

namespace
{
constexpr QLatin1StringView CurrentTriggerSuffix = "|Current"_L1;
constexpr QLatin1StringView DefaultTriggerSuffix = "|Default"_L1;
}

QSet<QKeySequence> Component::keysFromString(const QString &str)
{
    QSet<QKeySequence> ret;
    if (str == QLatin1String("none")) {
        return ret;
    }
    const QStringList strList = str.split(QLatin1Char('\t'));
    for (const QString &s : strList) {
        QKeySequence key = QKeySequence::fromString(s, QKeySequence::PortableText);
        ret.insert(key);
    }
    return Utils::normalizeSequences(ret);
}

QString Component::stringFromKeys(const QSet<QKeySequence> &keys)
{
    if (keys.isEmpty()) {
        return QStringLiteral("none");
    }

    QList<QKeySequence> sortedKeys(keys.begin(), keys.end());
    std::sort(sortedKeys.begin(), sortedKeys.end());

    QString ret;
    for (const QKeySequence &key : sortedKeys) {
        Q_ASSERT(!key.isEmpty());
        ret.append(key.toString(QKeySequence::PortableText));
        ret.append(QLatin1Char('\t'));
    }
    ret.chop(1);
    return ret;
}

Component::Component(const QString &uniqueName, const QString &friendlyName, GlobalShortcutsRegistry *registry)
    : _uniqueName(uniqueName)
    , _friendlyName(friendlyName)
    , _registry(registry)
{
    // Make sure we do no get uniquenames still containing the context
    Q_ASSERT(uniqueName.indexOf(QLatin1Char('|')) == -1);

    const QString DEFAULT(QStringLiteral("default"));
    createGlobalShortcutContext(DEFAULT, QStringLiteral("Default Context"));
    _current = _contexts.value(DEFAULT);
}

Component::~Component()
{
    // We delete all shortcuts from all contexts
    qDeleteAll(_contexts);
}

bool Component::activateGlobalShortcutContext(const QString &uniqueName)
{
    if (!_contexts.value(uniqueName)) {
        createGlobalShortcutContext(uniqueName, QStringLiteral("TODO4"));
        return false;
    }

    // Deactivate the current contexts shortcuts
    deactivateShortcuts();

    // Switch the context
    _current = _contexts.value(uniqueName);

    return true;
}

void Component::activateShortcuts()
{
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        shortcut->setActive();
    }
}

QList<GlobalShortcut *> Component::allShortcuts(const QString &contextName) const
{
    GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->_actionsMap.values() : QList<GlobalShortcut *>{};
}

QList<KGlobalShortcutInfo> Component::allShortcutInfos(const QString &contextName) const
{
    GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->allShortcutInfos() : QList<KGlobalShortcutInfo>{};
}

bool Component::cleanUp()
{
    bool changed = false;

    const auto actions = _current->_actionsMap;
    for (GlobalShortcut *shortcut : actions) {
        qCDebug(KGLOBALACCELD) << _current->_actionsMap.size();
        if (!shortcut->isPresent()) {
            changed = true;
            shortcut->unRegister();
        }
    }

    if (changed) {
        _registry->writeSettings();
        // We could be destroyed after this call!
    }

    return changed;
}

bool Component::createGlobalShortcutContext(const QString &uniqueName, const QString &friendlyName)
{
    if (_contexts.value(uniqueName)) {
        qCDebug(KGLOBALACCELD) << "Shortcut Context " << uniqueName << "already exists for component " << _uniqueName;
        return false;
    }
    _contexts.insert(uniqueName, new GlobalShortcutContext(uniqueName, friendlyName, this));
    return true;
}

GlobalShortcutContext *Component::currentContext()
{
    return _current;
}

QDBusObjectPath Component::dbusPath() const
{
    auto isNonAscii = [](QChar ch) {
        const char c = ch.unicode();
        const bool isAscii = c == '_' //
            || (c >= 'A' && c <= 'Z') //
            || (c >= 'a' && c <= 'z') //
            || (c >= '0' && c <= '9');
        return !isAscii;
    };

    QString dbusPath = _uniqueName;
    // DBus path can only contain ASCII characters, any non-alphanumeric char should
    // be turned into '_'
    std::replace_if(dbusPath.begin(), dbusPath.end(), isNonAscii, QLatin1Char('_'));

    // QDBusObjectPath could be a little bit easier to handle :-)
    return QDBusObjectPath(QLatin1String("/component/") + dbusPath);
}

void Component::deactivateShortcuts(bool temporarily)
{
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        if (temporarily //
            && _uniqueName == QLatin1String("kwin") //
            && shortcut->uniqueName() == QLatin1String("Block Global Shortcuts")) {
            continue;
        }
        shortcut->setInactive();
    }
}

void Component::emitGlobalShortcutEvent(const GlobalShortcut &shortcut, ShortcutKeyState state)
{
    const long timestamp = 0;

    if (shortcut.context()->component() != this) {
        return;
    }

    switch (state) {
    case ShortcutKeyState::Pressed:
        Q_EMIT globalShortcutPressed(shortcut.context()->component()->uniqueName(), shortcut.uniqueName(), timestamp);
        break;
    case ShortcutKeyState::Repeated:
        Q_EMIT globalShortcutRepeated(shortcut.context()->component()->uniqueName(), shortcut.uniqueName(), timestamp);
        break;
    case ShortcutKeyState::Released:
        Q_EMIT globalShortcutReleased(shortcut.context()->component()->uniqueName(), shortcut.uniqueName(), timestamp);
        break;
    }
}

void Component::invokeShortcut(const QString &shortcutName, const QString &context)
{
    GlobalShortcut *shortcut = getShortcutByName(shortcutName, context);
    if (shortcut) {
        emitGlobalShortcutEvent(*shortcut, ShortcutKeyState::Pressed);
    }
}

QString Component::friendlyName() const
{
    return !_friendlyName.isEmpty() ? _friendlyName : _uniqueName;
}

QList<GlobalShortcut *> Component::getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    QList<GlobalShortcut *> rc;
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        rc += context->getShortcutsByKey(key, type);
    }
    return rc;
}

QList<GlobalShortcut *> Component::getShortcutsByTrigger(const KGlobalShortcutTrigger &trigger) const
{
    QList<GlobalShortcut *> rc;
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        rc += context->getShortcutsByTrigger(trigger);
    }
    return rc;
}

GlobalShortcut *Component::getShortcutByName(const QString &uniqueName, const QString &context) const
{
    const GlobalShortcutContext *shortcutContext = _contexts.value(context);
    return shortcutContext ? shortcutContext->_actionsMap.value(uniqueName) : nullptr;
}

QStringList Component::getShortcutContexts() const
{
    return _contexts.keys();
}

bool Component::isActive() const
{
    // The component is active if at least one of it's global shortcuts is
    // present.
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        if (shortcut->isPresent()) {
            return true;
        }
    }
    return false;
}

bool Component::isShortcutKeyAvailable(const QKeySequence &key, const QString &component, const QString &context) const
{
    qCDebug(KGLOBALACCELD) << key.toString() << component;

    // if this component asks for the key. only check the keys in the same
    // context
    if (component == uniqueName()) {
        return shortcutContext(context)->isShortcutKeyAvailable(key);
    } else {
        for (auto it = _contexts.cbegin(), endIt = _contexts.cend(); it != endIt; ++it) {
            const GlobalShortcutContext *ctx = it.value();
            if (!ctx->isShortcutKeyAvailable(key)) {
                return false;
            }
        }
    }
    return true;
}

bool Component::isShortcutTriggerAvailable(const KGlobalShortcutTrigger &trigger, const QString &component, const QString &context) const
{
    qCDebug(KGLOBALACCELD) << trigger.type() << trigger.paramString() << component;

    // if this component asks for the trigger, only check the triggers in the same context
    if (component == uniqueName()) {
        return shortcutContext(context)->isShortcutTriggerAvailable(trigger);
    } else {
        for (auto it = _contexts.cbegin(), endIt = _contexts.cend(); it != endIt; ++it) {
            const GlobalShortcutContext *ctx = it.value();
            if (!ctx->isShortcutTriggerAvailable(trigger)) {
                return false;
            }
        }
    }
    return true;
}

GlobalShortcut *Component::registerShortcut(const QString &uniqueName,
                                            const QString &friendlyName,
                                            const QString &shortcutKeysString,
                                            const QString &defaultShortcutKeysString,
                                            uint64_t serial)
{
    if (!serial) {
        serial = _registry->nextSerial();
        Q_EMIT _registry->needsSave();
    }

    GlobalShortcut *shortcut = new GlobalShortcut(uniqueName, friendlyName, serial, currentContext(), _registry);
    shortcut->setKeys(keysFromString(shortcutKeysString));
    shortcut->setDefaultKeys(keysFromString(defaultShortcutKeysString));
    shortcut->setIsFresh(false);
    return shortcut;
}

void Component::loadInverseAction(const QString &aUniqueName, const QStringList &configEntry)
{
    if (configEntry.size() != 1) { // possibility of later expansion with optional list entries
        qCWarning(KGLOBALACCELD) << "Inverse action invalid format, ignoring:" << aUniqueName << configEntry;
        return;
    }
    const QString &bUniqueName = configEntry.last();
    GlobalShortcut *a = currentContext()->_actionsMap.value(aUniqueName);
    GlobalShortcut *b = currentContext()->_actionsMap.value(bUniqueName);
    if (!a || !b) {
        qCWarning(KGLOBALACCELD) << "Inverse action configured but one or both shortcuts not found, ignoring:" << //
            aUniqueName << static_cast<bool>(a) << "|" << bUniqueName << static_cast<bool>(b);
        return;
    }
    if ((!a->inverseActionUniqueName().isEmpty() && a->inverseActionUniqueName() != bUniqueName)
        || (!b->inverseActionUniqueName().isEmpty() && b->inverseActionUniqueName() != aUniqueName)) {
        qCWarning(KGLOBALACCELD) << "Inverse action configured for actions with a pre-existing assignment:" << //
            aUniqueName << a->inverseActionUniqueName() << "|" << bUniqueName << b->inverseActionUniqueName();
        return;
    }
    a->setInverseActionUniqueName(bUniqueName);
    b->setInverseActionUniqueName(aUniqueName);
}

void Component::loadTriggers(GlobalShortcut *shortcut, const QString &triggerType, const QStringList &triggerParamStrings, bool isDefault)
{
    QSet<KGlobalShortcutTrigger> triggers;
    triggers.reserve(triggerParamStrings.size());

    for (const QString &triggerParamString : triggerParamStrings) {
        triggers.insert(KGlobalShortcutTrigger(triggerType, triggerParamString));
    }

    if (isDefault) {
        shortcut->setDefaultTriggers(triggerType, triggers);
    }
    shortcut->setTriggers(triggerType, triggers, isDefault ? GlobalShortcut::FromDefaults : GlobalShortcut::FromOverride);
}

void Component::loadSettings(const KConfigGroup &configGroup, const KConfigGroup &stateGroup)
{
    // GlobalShortcutsRegistry::loadSettings handles contexts.
    const auto listKeys = configGroup.keyList();
    for (const QString &confKey : listKeys) {
        const QStringList entry = configGroup.readEntry(confKey, QStringList());
        if (entry.size() != 3) {
            continue;
        }

        const uint64_t serial = stateGroup.readEntry<uint64_t>(confKey, 0);

        registerShortcut(confKey, entry[2], entry[0], entry[1], serial);
    }

    const KConfigGroup triggersGroup(&configGroup, "$Triggers"_L1);

    const auto actionTriggersGroups = triggersGroup.groupList();
    for (const QString &actionUnique : actionTriggersGroups) {
        GlobalShortcut *shortcut = _current->_actionsMap.value(actionUnique, nullptr);
        if (!shortcut) {
            continue;
        }
        const KConfigGroup actionTriggersGroup(&triggersGroup, actionUnique);
        const auto triggerTypeKeys = actionTriggersGroup.keyList();

        for (const QString &confKey : triggerTypeKeys) {
            if (confKey.endsWith(DefaultTriggerSuffix)) {
                const QString triggerType = confKey.first(confKey.size() - DefaultTriggerSuffix.size());
                loadTriggers(shortcut, triggerType, actionTriggersGroup.readEntry(confKey, QStringList()), true);
                continue;
            }
            if (confKey.endsWith(CurrentTriggerSuffix)) {
                const QString triggerType = confKey.first(confKey.size() - CurrentTriggerSuffix.size());
                loadTriggers(shortcut, triggerType, actionTriggersGroup.readEntry(confKey, QStringList()), false);
                continue;
            }
        }
    }

    const KConfigGroup inverseActionGroup(&configGroup, "$InverseAction"_L1);

    const auto inverseActionKeys = inverseActionGroup.keyList();
    for (const QString &confKey : inverseActionKeys) {
        loadInverseAction(confKey, inverseActionGroup.readEntry(confKey, QStringList()));
    }
}

void Component::setFriendlyName(const QString &name)
{
    _friendlyName = name;
}

GlobalShortcutContext *Component::shortcutContext(const QString &contextName)
{
    return _contexts.value(contextName);
}

GlobalShortcutContext const *Component::shortcutContext(const QString &contextName) const
{
    return _contexts.value(contextName);
}

QStringList Component::shortcutNames(const QString &contextName) const
{
    const GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->_actionsMap.keys() : QStringList{};
}

QString Component::uniqueName() const
{
    return _uniqueName;
}

void Component::unregisterShortcut(const QString &uniqueName)
{
    // Now wrote all contexts
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        if (context->_actionsMap.value(uniqueName)) {
            delete context->takeShortcut(context->_actionsMap.value(uniqueName));
        }
    }
}

void Component::writeSettings(KConfigGroup &configGroup, KConfigGroup &stateGroup) const
{
    // If we don't delete the current content global shortcut
    // registrations will never not deleted after forgetGlobalShortcut()
    configGroup.deleteGroup();
    stateGroup.deleteGroup();

    // Now write all contexts
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        KConfigGroup contextGroup;

        if (context->uniqueName() == QLatin1String("default")) {
            contextGroup = configGroup;
            // Write the friendly name
            contextGroup.writeEntry("_k_friendly_name", friendlyName());
        } else {
            contextGroup = KConfigGroup(&configGroup, context->uniqueName());
            // Write the friendly name
            contextGroup.writeEntry("_k_friendly_name", context->friendlyName());
        }

        KConfigGroup inverseActionGroup(&contextGroup, "$InverseAction"_L1);
        KConfigGroup triggersGroup(&contextGroup, "$Triggers"_L1);

        QStringList triggerParamStrings; // avoid reallocating the list for every shortcut with triggers

        // qCDebug(KGLOBALACCELD) << "writing group " << _uniqueName << ":" << context->uniqueName();

        for (const GlobalShortcut *shortcut : std::as_const(context->_actionsMap)) {
            // qCDebug(KGLOBALACCELD) << "writing" << shortcut->uniqueName();

            // We do not write fresh shortcuts.
            // We do not write session shortcuts
            if (shortcut->isFresh() || shortcut->isSessionShortcut()) {
                continue;
            }
            // qCDebug(KGLOBALACCELD) << "really writing" << shortcut->uniqueName();

            QStringList entry(stringFromKeys(shortcut->keys()));
            entry.append(stringFromKeys(shortcut->defaultKeys()));
            entry.append(shortcut->friendlyName());

            contextGroup.writeEntry(shortcut->uniqueName(), entry);
            stateGroup.writeEntry(shortcut->uniqueName(), shortcut->serial());

            if (!shortcut->inverseActionUniqueName().isEmpty() && !inverseActionGroup.hasKey(shortcut->inverseActionUniqueName())) {
                inverseActionGroup.writeEntry(shortcut->uniqueName(), QStringList{shortcut->inverseActionUniqueName()});
            }

            KConfigGroup actionTriggersGroup(&triggersGroup, shortcut->uniqueName());
            const auto triggerTypes = shortcut->triggerTypes();

            for (const QString &triggerType : triggerTypes) {
                const auto defaultTriggers = shortcut->defaultTriggers(triggerType);

                // defaults
                if (!defaultTriggers.isEmpty()) {
                    triggerParamStrings.clear();
                    for (const KGlobalShortcutTrigger &trigger : defaultTriggers) {
                        triggerParamStrings.append(trigger.paramString());
                    }
                    actionTriggersGroup.writeEntry(triggerType + DefaultTriggerSuffix, triggerParamStrings);
                }

                if (shortcut->hasOverrideTriggerAssignments(triggerType)) {
                    triggerParamStrings.clear();
                    const auto triggers = shortcut->triggers(triggerType);
                    for (const KGlobalShortcutTrigger &trigger : triggers) {
                        triggerParamStrings.append(trigger.paramString());
                    }
                    actionTriggersGroup.writeEntry(triggerType + CurrentTriggerSuffix, triggerParamStrings);
                }
            }
        }
    }
}

// static
bool Component::isReservedConfigGroupName(const QString &name)
{
    return name.startsWith("$"_L1); // "$InverseAction", "$Trigger", etc.
}

#include "moc_component.cpp"
