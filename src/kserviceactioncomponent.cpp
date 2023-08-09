/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kserviceactioncomponent.h"
#include "logging_p.h"

#include <QFileInfo>

#include <KIO/ApplicationLauncherJob>
#include <KIO/UntrustedProgramHandlerInterface>
#include <KNotificationJobUiDelegate>

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

QString makeUniqueName(const KService::Ptr &service)
{
    if (service->storageId().startsWith(QLatin1Char('/'))) {
        return QFileInfo(service->storageId()).fileName();
    }

    return service->storageId();
}

KServiceActionComponent::KServiceActionComponent(KService::Ptr service)
    : Component(makeUniqueName(service), service->name())
    , m_service(service)
{
}

KServiceActionComponent::~KServiceActionComponent() = default;

void KServiceActionComponent::emitGlobalShortcutPressed(const GlobalShortcut &shortcut)
{
    KIO::ApplicationLauncherJob *job = nullptr;

    if (shortcut.uniqueName() == QLatin1String("_launch")) {
        job = new KIO::ApplicationLauncherJob(m_service);
    } else {
        const auto actions = m_service->actions();
        const auto it = std::find_if(actions.cbegin(), actions.cend(), [&shortcut](const KServiceAction &action) {
            return action.name() == shortcut.uniqueName();
        });
        if (it == actions.cend()) {
            qCCritical(KGLOBALACCELD, "failed to find an action matching the '%s' name", qPrintable(shortcut.uniqueName()));
            return;
        } else {
            job = new KIO::ApplicationLauncherJob(*it);
        }
    }

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
    const QString shortcutString = m_service->property(QStringLiteral("X-KDE-Shortcuts")).toStringList().join(QLatin1Char('\t'));
    GlobalShortcut *shortcut = registerShortcut(QStringLiteral("_launch"), m_service->name(), shortcutString, shortcutString);
    shortcut->setIsPresent(true);

    const auto lstActions = m_service->actions();
    for (const KServiceAction &action : lstActions) {
        const QString shortcutString = action.property(QStringLiteral("X-KDE-Shortcuts"), QMetaType::QStringList).toStringList().join(QLatin1Char('\t'));
        GlobalShortcut *shortcut = registerShortcut(action.name(), action.text(), shortcutString, shortcutString);
        shortcut->setIsPresent(true);
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
