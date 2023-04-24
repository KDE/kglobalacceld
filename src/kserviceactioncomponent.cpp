/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kserviceactioncomponent.h"
#include "logging_p.h"

#include <QDBusConnectionInterface>
#include <QFileInfo>
#include <QProcess>

#include <KIO/ApplicationLauncherJob>
#include <KIO/UntrustedProgramHandlerInterface>
#include <KNotificationJobUiDelegate>
#include <KService>
#include <KShell>
#include <KWindowSystem>

#include "config-kglobalaccel.h"
#if HAVE_X11
#include <KStartupInfo>
#include <private/qtx11extras_p.h>
#endif

class UntrustedProgramHandler : public KIO::UntrustedProgramHandlerInterface
{
public:
    UntrustedProgramHandler(QObject *parent)
        : KIO::UntrustedProgramHandlerInterface(parent)
    {
    }

    void showUntrustedProgramWarning(KJob * /*job*/, const QString & /*programName*/) override
    {
        Q_EMIT result(true);
    }
};

KServiceActionComponent::KServiceActionComponent(const QString &serviceStorageId, const QString &friendlyName)
    : Component(serviceStorageId, friendlyName)
    , m_serviceStorageId(serviceStorageId)
{
    QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kglobalaccel/") + serviceStorageId);
    if (filePath.isEmpty()) {
        // Fallback to applications data dir for custom shortcut for instance
        filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/") + serviceStorageId);
        m_isInApplicationsDir = true;
    } else {
        QFileInfo info(filePath);
        if (info.isSymLink()) {
            const QString filePath2 = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/") + serviceStorageId);
            if (info.symLinkTarget() == filePath2) {
                filePath = filePath2;
                m_isInApplicationsDir = true;
            }
        }
    }

    if (filePath.isEmpty()) {
        qCWarning(KGLOBALACCELD) << "No desktop file found for service " << serviceStorageId;
    }
    m_desktopFile.reset(new KDesktopFile(filePath));
}

KServiceActionComponent::~KServiceActionComponent() = default;

void KServiceActionComponent::emitGlobalShortcutPressed(const GlobalShortcut &shortcut)
{
    KIO::ApplicationLauncherJob *job = nullptr;

    if (shortcut.uniqueName() == QLatin1String("_launch")) {
        KService::Ptr service = KService::serviceByStorageId(m_desktopFile->fileName());
        job = new KIO::ApplicationLauncherJob(service);
    } else {
        KService::Ptr service = KService::serviceByStorageId(m_desktopFile->fileName());
        const auto actions = service->actions();
        for (const KServiceAction &action : actions) {
            if (action.name() == shortcut.uniqueName()) {
                job = new KIO::ApplicationLauncherJob(action);
                break;
            }
        }
    }

    Q_ASSERT(job);

    auto *delegate = new KNotificationJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled);
    // ApplicationLauncherJob refuses to launch desktop files in /usr/share/kglobalaccel/ unless they are marked as executable
    // to avoid that add our own UntrustedProgramHandler that accepts the launch regardless
    new UntrustedProgramHandler(delegate);
    job->setUiDelegate(delegate);
    if (QX11Info::isPlatformX11()) {
        // Create a startup id ourselves. Otherwise ApplicationLauncherJob will query X11 to get a timestamp, which causes a deadlock
        auto startupId = KStartupInfo::createNewStartupIdForTimestamp(QX11Info::appTime());
        job->setStartupId(startupId);
    }
    job->start();
}

void KServiceActionComponent::loadFromService()
{
    auto registerGroupShortcut = [this](const QString &name, const KConfigGroup &group) {
        const QString shortcutString = group.readEntry(QStringLiteral("X-KDE-Shortcuts"), QString()).replace(QLatin1Char(','), QLatin1Char('\t'));
        GlobalShortcut *shortcut = registerShortcut(name, group.readEntry(QStringLiteral("Name"), QString()), shortcutString, shortcutString);
        shortcut->setIsPresent(true);
    };

    registerGroupShortcut(QStringLiteral("_launch"), m_desktopFile->desktopGroup());
    const auto lstActions = m_desktopFile->readActions();
    for (const QString &action : lstActions) {
        registerGroupShortcut(action, m_desktopFile->actionGroup(action));
    }
}

bool KServiceActionComponent::cleanUp()
{
    qCDebug(KGLOBALACCELD) << "Disabling desktop file";

    const auto shortcuts = allShortcuts();
    for (GlobalShortcut *shortcut : shortcuts) {
        shortcut->setIsPresent(false);
    }

    return Component::cleanUp();
}

#include "moc_kserviceactioncomponent.cpp"
