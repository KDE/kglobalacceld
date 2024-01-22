/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2022 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "globalshortcutsregistry.h"
#include "component.h"
#include "globalshortcut.h"
#include "globalshortcutcontext.h"
#include "kglobalaccel_interface.h"
#include "kglobalshortcutinfo_p.h"
#include "kserviceactioncomponent.h"
#include "logging_p.h"
#include <config-kglobalaccel.h>

#include <KFileUtils>
#include <KPluginMetaData>

#include <KApplicationTrader>
#include <QDBusConnection>
#include <QDir>
#include <QGuiApplication>
#include <QJsonArray>
#include <QPluginLoader>
#include <QStandardPaths>

static bool checkPlatform(const QJsonObject &metadata, const QString &platformName)
{
    const QJsonArray platforms = metadata.value(QStringLiteral("MetaData")).toObject().value(QStringLiteral("platforms")).toArray();
    return std::any_of(platforms.begin(), platforms.end(), [&platformName](const QJsonValue &value) {
        return QString::compare(platformName, value.toString(), Qt::CaseInsensitive) == 0;
    });
}

static KGlobalAccelInterface *loadPlugin(GlobalShortcutsRegistry *parent)
{
    QString platformName = QString::fromLocal8Bit(qgetenv("KGLOBALACCELD_PLATFORM"));
    if (platformName.isEmpty()) {
        platformName = QGuiApplication::platformName();
    }

    const QList<QStaticPlugin> staticPlugins = QPluginLoader::staticPlugins();
    for (const QStaticPlugin &staticPlugin : staticPlugins) {
        const QJsonObject metadata = staticPlugin.metaData();
        if (metadata.value(QLatin1String("IID")) != QLatin1String(KGlobalAccelInterface_iid)) {
            continue;
        }
        if (checkPlatform(metadata, platformName)) {
            KGlobalAccelInterface *interface = qobject_cast<KGlobalAccelInterface *>(staticPlugin.instance());
            if (interface) {
                qCDebug(KGLOBALACCELD) << "Loaded a static plugin for platform" << platformName;
                interface->setRegistry(parent);
                return interface;
            }
        }
    }

    const QList<KPluginMetaData> candidates = KPluginMetaData::findPlugins(QStringLiteral("org.kde.kglobalacceld.platforms"));
    for (const KPluginMetaData &candidate : candidates) {
        QPluginLoader loader(candidate.fileName());
        if (checkPlatform(loader.metaData(), platformName)) {
            KGlobalAccelInterface *interface = qobject_cast<KGlobalAccelInterface *>(loader.instance());
            if (interface) {
                qCDebug(KGLOBALACCELD) << "Loaded plugin" << candidate.fileName() << "for platform" << platformName;
                interface->setRegistry(parent);
                return interface;
            }
        }
    }

    qCWarning(KGLOBALACCELD) << "Could not find any platform plugin";
    return nullptr;
}

static QString getConfigFile()
{
    return qEnvironmentVariableIsSet("KGLOBALACCEL_TEST_MODE") ? QString() : QStringLiteral("kglobalshortcutsrc");
}

/*
 * Migrate the config for service actions to a new format that only stores the actual shortcut if not default.
 * All other information is read from the desktop file.
 * Keep the old data for compatibility with KF5-based kglobalaccel.
 * Once Plasma 6 settles down consider dropping this data
 */
void GlobalShortcutsRegistry::migrateConfig()
{
    const QStringList groups = _config.groupList();

    KConfigGroup services = _config.group(QStringLiteral("services"));

    for (const QString &componentName : groups) {
        if (!componentName.endsWith(QLatin1String(".desktop"))) {
            continue;
        }

        KConfigGroup component = _config.group(componentName);
        KConfigGroup newGroup = services.group(componentName);

        for (auto [key, value] : component.entryMap().asKeyValueRange()) {
            if (key == QLatin1String("_k_friendly_name")) {
                continue;
            }

            const QString shortcut = value.split(QLatin1Char(','))[0];
            const QString defaultShortcut = value.split(QLatin1Char(','))[1];

            if (shortcut != defaultShortcut) {
                newGroup.writeEntry(key, shortcut);
            }
        }

        component.deleteGroup();
    }

    _config.sync();
}

GlobalShortcutsRegistry::GlobalShortcutsRegistry()
    : QObject()
    , _manager(loadPlugin(this))
    , _config(getConfigFile(), KConfig::SimpleConfig)
{
    migrateConfig();

    if (_manager) {
        _manager->setEnabled(true);
    }
}

GlobalShortcutsRegistry::~GlobalShortcutsRegistry()
{
    m_components.clear();

    if (_manager) {
        _manager->setEnabled(false);

        // Ungrab all keys. We don't go over GlobalShortcuts because
        // GlobalShortcutsRegistry::self() doesn't work anymore.
        const auto listKeys = _active_keys.keys();
        for (const QKeySequence &key : listKeys) {
            for (int i = 0; i < key.count(); i++) {
                _manager->grabKey(key[i].toCombined(), false);
            }
        }
    }
    _active_keys.clear();
    _keys_count.clear();
}

Component *GlobalShortcutsRegistry::registerComponent(ComponentPtr component)
{
    m_components.push_back(std::move(component));
    auto *comp = m_components.back().get();
    Q_ASSERT(!comp->dbusPath().path().isEmpty());
    QDBusConnection conn(QDBusConnection::sessionBus());
    conn.registerObject(comp->dbusPath().path(), comp, QDBusConnection::ExportScriptableContents);
    return comp;
}

void GlobalShortcutsRegistry::activateShortcuts()
{
    for (auto &component : m_components) {
        component->activateShortcuts();
    }
}

QList<QDBusObjectPath> GlobalShortcutsRegistry::componentsDbusPaths() const
{
    QList<QDBusObjectPath> dbusPaths;
    dbusPaths.reserve(m_components.size());
    std::transform(m_components.cbegin(), m_components.cend(), std::back_inserter(dbusPaths), [](const auto &comp) {
        return comp->dbusPath();
    });
    return dbusPaths;
}

QList<QStringList> GlobalShortcutsRegistry::allComponentNames() const
{
    QList<QStringList> ret;
    ret.reserve(m_components.size());
    std::transform(m_components.cbegin(), m_components.cend(), std::back_inserter(ret), [](const auto &component) {
        // A string for each enumerator in KGlobalAccel::actionIdFields
        return QStringList{component->uniqueName(), component->friendlyName(), {}, {}};
    });

    return ret;
}

void GlobalShortcutsRegistry::clear()
{
    m_components.clear();

    // The shortcuts should have deregistered themselves
    Q_ASSERT(_active_keys.isEmpty());
}

QDBusObjectPath GlobalShortcutsRegistry::dbusPath() const
{
    return _dbusPath;
}

void GlobalShortcutsRegistry::deactivateShortcuts(bool temporarily)
{
    for (ComponentPtr &component : m_components) {
        component->deactivateShortcuts(temporarily);
    }
}

Component *GlobalShortcutsRegistry::getComponent(const QString &uniqueName)
{
    auto it = findByName(uniqueName);
    return it != m_components.cend() ? (*it).get() : nullptr;
}

GlobalShortcut *GlobalShortcutsRegistry::getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    for (const ComponentPtr &component : m_components) {
        GlobalShortcut *rc = component->getShortcutByKey(key, type);
        if (rc) {
            return rc;
        }
    }
    return nullptr;
}

QList<GlobalShortcut *> GlobalShortcutsRegistry::getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    QList<GlobalShortcut *> rc;
    for (const ComponentPtr &component : m_components) {
        rc = component->getShortcutsByKey(key, type);
        if (!rc.isEmpty()) {
            return rc;
        }
    }
    return {};
}

bool GlobalShortcutsRegistry::isShortcutAvailable(const QKeySequence &shortcut, const QString &componentName, const QString &contextName) const
{
    return std::all_of(m_components.cbegin(), m_components.cend(), [&shortcut, &componentName, &contextName](const ComponentPtr &component) {
        return component->isShortcutAvailable(shortcut, componentName, contextName);
    });
}

Q_GLOBAL_STATIC(GlobalShortcutsRegistry, _self)
GlobalShortcutsRegistry *GlobalShortcutsRegistry::self()
{
    return _self;
}

/**
 * When we are provided just a Shift key press, interpret it as "Shift" not as "Shift+Shift"
 */
static void correctKeyEvent(int &keyQt)
{
    switch (keyQt) {
    case (Qt::ShiftModifier | Qt::Key_Shift).toCombined():
        keyQt = Qt::Key_Shift;
        break;
    case (Qt::ControlModifier | Qt::Key_Control).toCombined():
        keyQt = Qt::Key_Control;
        break;
    case (Qt::AltModifier | Qt::Key_Alt).toCombined():
        keyQt = Qt::Key_Alt;
        break;
    case (Qt::MetaModifier | Qt::Key_Meta).toCombined():
        keyQt = Qt::Key_Meta;
        break;
    }
}

bool GlobalShortcutsRegistry::keyPressed(int keyQt)
{
    correctKeyEvent(keyQt);
    int keys[maxSequenceLength] = {0, 0, 0, 0};
    int count = _active_sequence.count();
    if (count == maxSequenceLength) {
        // buffer is full, rotate it
        for (int i = 1; i < count; i++) {
            keys[i - 1] = _active_sequence[i].toCombined();
        }
        keys[maxSequenceLength - 1] = keyQt;
    } else {
        // just append the new key
        for (int i = 0; i < count; i++) {
            keys[i] = _active_sequence[i].toCombined();
        }
        keys[count] = keyQt;
    }

    _active_sequence = QKeySequence(keys[0], keys[1], keys[2], keys[3]);

    GlobalShortcut *shortcut = nullptr;
    QKeySequence tempSequence;
    for (int length = 1; length <= _active_sequence.count(); length++) {
        // We have to check all possible matches from the end since we're rotating active sequence
        // instead of cleaning it when it's full
        int sequenceToCheck[maxSequenceLength] = {0, 0, 0, 0};
        for (int i = 0; i < length; i++) {
            sequenceToCheck[i] = _active_sequence[_active_sequence.count() - length + i].toCombined();
        }
        tempSequence = QKeySequence(sequenceToCheck[0], sequenceToCheck[1], sequenceToCheck[2], sequenceToCheck[3]);
        shortcut = getShortcutByKey(tempSequence);

        if (shortcut) {
            break;
        }
    }

    qCDebug(KGLOBALACCELD) << "Pressed key" << QKeySequence(keyQt).toString() << ", current sequence" << _active_sequence.toString() << "="
                           << (shortcut ? shortcut->uniqueName() : QStringLiteral("(no shortcut found)"));
    if (!shortcut) {
        // This can happen for example with the ALT-Print shortcut of kwin.
        // ALT+PRINT is SYSREQ on my keyboard. So we grab something we think
        // is ALT+PRINT but symXToKeyQt and modXToQt make ALT+SYSREQ of it
        // when pressed (correctly). We can't match that.
        qCDebug(KGLOBALACCELD) << "Got unknown key" << QKeySequence(keyQt).toString();

        // In production mode just do nothing.
        return false;
    } else if (!shortcut->isActive()) {
        qCDebug(KGLOBALACCELD) << "Got inactive key" << QKeySequence(keyQt).toString();

        // In production mode just do nothing.
        return false;
    }

    qCDebug(KGLOBALACCELD) << QKeySequence(keyQt).toString() << "=" << shortcut->uniqueName();

    // shortcut is found, reset active sequence
    _active_sequence = QKeySequence();

    QStringList data(shortcut->context()->component()->uniqueName());
    data.append(shortcut->uniqueName());
    data.append(shortcut->context()->component()->friendlyName());
    data.append(shortcut->friendlyName());

    if (m_lastShortcut && m_lastShortcut != shortcut) {
        m_lastShortcut->context()->component()->emitGlobalShortcutReleased(*m_lastShortcut);
    }

    // Invoke the action
    shortcut->context()->component()->emitGlobalShortcutPressed(*shortcut);
    m_lastShortcut = shortcut;

    return true;
}

bool GlobalShortcutsRegistry::keyReleased(int keyQt)
{
    Q_UNUSED(keyQt)
    if (m_lastShortcut) {
        m_lastShortcut->context()->component()->emitGlobalShortcutReleased(*m_lastShortcut);
        m_lastShortcut = nullptr;
    }
    return false;
}

Component *GlobalShortcutsRegistry::createComponent(const QString &uniqueName, const QString &friendlyName)
{
    auto it = findByName(uniqueName);
    if (it != m_components.cend()) {
        Q_ASSERT_X(false, //
                   "GlobalShortcutsRegistry::createComponent",
                   QLatin1String("A Component with the name: %1, already exists").arg(uniqueName).toUtf8().constData());
        return (*it).get();
    }

    auto *c = registerComponent(ComponentPtr(new Component(uniqueName, friendlyName), &unregisterComponent));
    return c;
}

void GlobalShortcutsRegistry::unregisterComponent(Component *component)
{
    QDBusConnection::sessionBus().unregisterObject(component->dbusPath().path());
    delete component;
}

KServiceActionComponent *GlobalShortcutsRegistry::createServiceActionComponent(KService::Ptr service)
{
    auto it = findByName(service->storageId());
    if (it != m_components.cend()) {
        Q_ASSERT_X(false, //
                   "GlobalShortcutsRegistry::createServiceActionComponent",
                   QLatin1String("A KServiceActionComponent with the name: %1, already exists").arg(service->storageId()).toUtf8().constData());
        return static_cast<KServiceActionComponent *>((*it).get());
    }

    auto *c = registerComponent(ComponentPtr(new KServiceActionComponent(service), &unregisterComponent));
    return static_cast<KServiceActionComponent *>(c);
}

KServiceActionComponent *GlobalShortcutsRegistry::createServiceActionComponent(const QString &uniqueName)

{
    auto it = findByName(uniqueName);
    if (it != m_components.cend()) {
        Q_ASSERT_X(false, //
                   "GlobalShortcutsRegistry::createServiceActionComponent",
                   QLatin1String("A KServiceActionComponent with the name: %1, already exists").arg(uniqueName).toUtf8().constData());
        return static_cast<KServiceActionComponent *>((*it).get());
    }

    KService::Ptr service = KService::serviceByStorageId(uniqueName);

    if (!service) {
        const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kglobalaccel/") + uniqueName);
        if (filePath.isEmpty()) {
            return nullptr;
        }
        service = new KService(filePath);
    }

    auto *c = registerComponent(ComponentPtr(new KServiceActionComponent(service), &unregisterComponent));

    return static_cast<KServiceActionComponent *>(c);
}

void GlobalShortcutsRegistry::loadSettings()
{
    if (!m_components.empty()) {
        qCDebug(KGLOBALACCELD) << "Registry settings already loaded. Skipped loading again.";
        return;
    }

    auto groupList = _config.groupList();
    for (const QString &groupName : groupList) {
        if (groupName == QLatin1String("services")) {
            continue;
        }

        if (groupName.endsWith(QLatin1String(".desktop"))) {
            continue;
        }

        qCDebug(KGLOBALACCELD) << "Loading group " << groupName;

        Q_ASSERT(groupName.indexOf(QLatin1Char('\x1d')) == -1);

        // loadSettings isn't designed to be called in between. Only at the
        // beginning.
        Q_ASSERT(!getComponent(groupName));

        KConfigGroup configGroup(&_config, groupName);

        const QString friendlyName = configGroup.readEntry("_k_friendly_name");

        Component *component = createComponent(groupName, friendlyName);

        // Now load the contexts
        const auto groupList = configGroup.groupList();
        for (const QString &context : groupList) {
            // Skip the friendly name group, this was previously used instead of _k_friendly_name
            if (context == QLatin1String("Friendly Name")) {
                continue;
            }

            KConfigGroup contextGroup(&configGroup, context);
            QString contextFriendlyName = contextGroup.readEntry("_k_friendly_name");
            component->createGlobalShortcutContext(context, contextFriendlyName);
            component->activateGlobalShortcutContext(context);
            component->loadSettings(contextGroup);
        }

        // Load the default context
        component->activateGlobalShortcutContext(QStringLiteral("default"));
        component->loadSettings(configGroup);
    }

    groupList = _config.group(QStringLiteral("services")).groupList();
    for (const QString &groupName : groupList) {
        qCDebug(KGLOBALACCELD) << "Loading group " << groupName;

        Q_ASSERT(groupName.indexOf(QLatin1Char('\x1d')) == -1);

        // loadSettings isn't designed to be called in between. Only at the
        // beginning.
        Q_ASSERT(!getComponent(groupName));

        KConfigGroup configGroup = _config.group(QStringLiteral("services")).group(groupName);

        Component *component = createServiceActionComponent(groupName);

        if (!component) {
            qDebug() << "could not create a component for " << groupName;
            continue;
        }
        Q_ASSERT(!component->uniqueName().isEmpty());

        // Now load the contexts
        const auto groupList = configGroup.groupList();
        for (const QString &context : groupList) {
            // Skip the friendly name group, this was previously used instead of _k_friendly_name
            if (context == QLatin1String("Friendly Name")) {
                continue;
            }

            KConfigGroup contextGroup(&configGroup, context);
            QString contextFriendlyName = contextGroup.readEntry("_k_friendly_name");
            component->createGlobalShortcutContext(context, contextFriendlyName);
            component->activateGlobalShortcutContext(context);
            component->loadSettings(contextGroup);
        }

        // Load the default context
        component->activateGlobalShortcutContext(QStringLiteral("default"));
        component->loadSettings(configGroup);
    }

    // Load the configured KServiceActions
    const QStringList desktopPaths =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kglobalaccel"), QStandardPaths::LocateDirectory);

    const QStringList desktopFiles = KFileUtils::findAllUniqueFiles(desktopPaths, {QStringLiteral("*.desktop")});

    for (const QString &file : desktopFiles) {
        const QString fileName = QFileInfo(file).fileName();
        auto it = findByName(fileName);
        if (it != m_components.cend()) {
            continue;
        }

        KService::Ptr service(new KService(file));
        if (service->noDisplay()) {
            continue;
        }

        auto *actionComp = createServiceActionComponent(service);
        actionComp->activateGlobalShortcutContext(QStringLiteral("default"));
        actionComp->loadFromService();
    }

    auto appsWithShortcuts = KApplicationTrader::query([](const KService::Ptr &service) {
        return !service->property<QString>(QStringLiteral("X-KDE-Shortcuts")).isEmpty();
    });

    for (auto service : appsWithShortcuts) {
        auto it = findByName(service->storageId());
        if (it != m_components.cend()) {
            continue;
        }

        auto *actionComp = createServiceActionComponent(service);
        actionComp->activateGlobalShortcutContext(QStringLiteral("default"));
        actionComp->loadFromService();
    }
}

void GlobalShortcutsRegistry::grabKeys()
{
    activateShortcuts();
}

bool GlobalShortcutsRegistry::registerKey(const QKeySequence &key, GlobalShortcut *shortcut)
{
    if (!_manager) {
        return false;
    }
    if (key.isEmpty()) {
        qCDebug(KGLOBALACCELD) << shortcut->uniqueName() << ": Key '" << QKeySequence(key).toString() << "' already taken by "
                               << _active_keys.value(key)->uniqueName() << ".";
        return false;
    } else if (_active_keys.value(key)) {
        qCDebug(KGLOBALACCELD) << shortcut->uniqueName() << ": Attempt to register key 0.";
        return false;
    }

    qCDebug(KGLOBALACCELD) << "Registering key" << QKeySequence(key).toString() << "for" << shortcut->context()->component()->uniqueName() << ":"
                           << shortcut->uniqueName();

    bool error = false;
    int i;
    for (i = 0; i < key.count(); i++) {
        const int combined = key[i].toCombined();
        if (!_manager->grabKey(combined, true)) {
            error = true;
            break;
        }
        ++_keys_count[combined];
    }

    if (error) {
        // Last key was not registered, rewind index by 1
        for (--i; i >= 0; i--) {
            const int combined = key[i].toCombined();
            auto it = _keys_count.find(combined);
            if (it == _keys_count.end()) {
                continue;
            }

            if (it.value() == 1) {
                _keys_count.erase(it);
                _manager->grabKey(combined, false);
            } else {
                --(it.value());
            }
        }
        return false;
    }

    _active_keys.insert(key, shortcut);

    return true;
}

void GlobalShortcutsRegistry::setDBusPath(const QDBusObjectPath &path)
{
    _dbusPath = path;
}

void GlobalShortcutsRegistry::ungrabKeys()
{
    deactivateShortcuts();
}

bool GlobalShortcutsRegistry::unregisterKey(const QKeySequence &key, GlobalShortcut *shortcut)
{
    if (!_manager) {
        return false;
    }
    if (_active_keys.value(key) != shortcut) {
        // The shortcut doesn't own the key or the key isn't grabbed
        return false;
    }

    for (int i = 0; i < key.count(); i++) {
        auto iter = _keys_count.find(key[i].toCombined());
        if ((iter == _keys_count.end()) || (iter.value() <= 0)) {
            continue;
        }

        // Unregister if there's only one ref to given key
        // We should fail earlier when key is not registered
        if (iter.value() == 1) {
            qCDebug(KGLOBALACCELD) << "Unregistering key" << QKeySequence(key[i]).toString() << "for" << shortcut->context()->component()->uniqueName() << ":"
                                   << shortcut->uniqueName();

            _manager->grabKey(key[i].toCombined(), false);
            _keys_count.erase(iter);
        } else {
            qCDebug(KGLOBALACCELD) << "Refused to unregister key" << QKeySequence(key[i]).toString() << ": used by another global shortcut";
            --(iter.value());
        }
    }

    if (shortcut && shortcut == m_lastShortcut) {
        m_lastShortcut->context()->component()->emitGlobalShortcutReleased(*m_lastShortcut);
        m_lastShortcut = nullptr;
    }

    _active_keys.remove(key);
    return true;
}

void GlobalShortcutsRegistry::writeSettings()
{
    auto it = std::remove_if(m_components.begin(), m_components.end(), [this](const ComponentPtr &component) {
        bool isService = component->uniqueName().endsWith(QLatin1String(".desktop"));

        KConfigGroup configGroup =
            isService ? _config.group(QStringLiteral("services")).group(component->uniqueName()) : _config.group(component->uniqueName());

        if (component->allShortcuts().isEmpty()) {
            configGroup.deleteGroup();
            return true;
        } else {
            component->writeSettings(configGroup);
            return false;
        }
    });

    m_components.erase(it, m_components.end());
    _config.sync();
}

#include "moc_globalshortcutsregistry.cpp"
