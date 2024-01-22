/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "move.h"
#include "match.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>

#include <QCommandLineOption>
#include <QFileInfo>

static bool isSpecialKey(const QString &key)
{
    return key == QStringLiteral("_k_friendly_name");
}

enum class Format {
    Shortcut,
    ShortcutDefaultShortcutDescription,
};

static Format formatForGroup(const KConfigGroup &group)
{
    KConfigGroup parent = group.parent();
    if (parent.name() == QLatin1String("services")) {
        return Format::Shortcut;
    }

    return Format::ShortcutDefaultShortcutDescription;
}

struct Migration {
    KConfigGroup sourceGroup;
    QString sourceActionName;

    KConfigGroup targetGroup;
    QString targetActionName;
    QString targetDefaultShortcut;
    QString targetDisplayName;

    void execute()
    {
        if (isSpecialKey(sourceActionName)) {
            return;
        }

        if (!sourceGroup.hasKey(sourceActionName)) {
            return;
        }

        const QStringList actionList = sourceGroup.readEntry<QStringList>(sourceActionName, QStringList());
        const QString shortcut = actionList.size() > 0 ? actionList.at(0) : QLatin1String("none");

        QString defaultShortcut = targetDefaultShortcut;
        if (defaultShortcut.isEmpty()) {
            if (actionList.size() > 1) {
                defaultShortcut = actionList.at(1);
            }
            if (defaultShortcut.isEmpty()) {
                defaultShortcut = QLatin1String("none");
            }
        }

        QString entry = targetActionName;
        if (entry.isEmpty()) {
            entry = sourceActionName;
        }

        const Format targetFormat = formatForGroup(targetGroup);
        if (targetFormat == Format::Shortcut) {
            sourceGroup.deleteEntry(sourceActionName);
            if (shortcut != defaultShortcut) {
                targetGroup.writeEntry(entry, shortcut);
            }
        } else if (targetFormat == Format::ShortcutDefaultShortcutDescription) {
            QString displayText = targetDisplayName;
            if (displayText.isEmpty()) {
                if (actionList.size() > 2) {
                    displayText = actionList.at(2);
                }
            }

            sourceGroup.deleteEntry(sourceActionName);
            targetGroup.writeEntry(entry, QStringList{shortcut, defaultShortcut, displayText});
        }

        if (sourceGroup.entryMap().isEmpty() || (sourceGroup.entryMap().size() == 1 && isSpecialKey(sourceGroup.entryMap().firstKey()))) {
            sourceGroup.deleteGroup();
        }
    }
};

int handleMove(QCommandLineParser &parser, const QStringList &arguments)
{
    QCommandLineOption configOption(QStringLiteral("config"), QStringLiteral("Path to kglobalshortcutsrc"), QStringLiteral("config"));

    QCommandLineOption sourceComponentOption(QStringLiteral("source-component"), QStringLiteral("Source component"), QStringLiteral("source-component"));
    QCommandLineOption sourceActionOption(QStringLiteral("source-action"), QStringLiteral("Source action"), QStringLiteral("source-action"));
    QCommandLineOption targetComponentOption(QStringLiteral("target-component"), QStringLiteral("Target component"), QStringLiteral("target-component"));
    QCommandLineOption targetActionOption(QStringLiteral("target-action"), QStringLiteral("Target action"), QStringLiteral("target-action"));

    QCommandLineOption targetDesktopFileOption(QStringLiteral("target-desktop-file"),
                                               QStringLiteral("Target desktop file"),
                                               QStringLiteral("target-desktop-file"));
    QCommandLineOption targetDesktopFileActionOption(QStringLiteral("target-desktop-file-action"),
                                                     QStringLiteral("Target desktop file action"),
                                                     QStringLiteral("target-desktop-file-action"));

    parser.addOption(configOption);
    parser.addOption(sourceComponentOption);
    parser.addOption(sourceActionOption);
    parser.addOption(targetComponentOption);
    parser.addOption(targetActionOption);
    parser.addOption(targetDesktopFileOption);
    parser.addOption(targetDesktopFileActionOption);
    parser.addHelpOption();

    parser.process(arguments);
    if (!parser.isSet(sourceComponentOption)) {
        qWarning() << "Missing --source-component";
        parser.showHelp();
        return -1;
    }

    if (!(parser.isSet(targetComponentOption) || parser.isSet(targetDesktopFileOption))) {
        qWarning() << "Missing --target-component or --target-desktop-file";
        parser.showHelp();
        return -1;
    }

    QString configFilePath;
    if (parser.isSet(configOption)) {
        configFilePath = parser.value(configOption);
        if (configFilePath.isEmpty()) {
            qFatal("Invalid config file path: %s", qPrintable(configFilePath));
        }
    } else {
        configFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kglobalshortcutsrc");
    }

    KConfig config(configFilePath);

    QList<KConfigGroup> sourceGroups;
    QString sourceAction;
    if (parser.isSet(sourceComponentOption)) {
        const QString sourceComponentPattern = parser.value(sourceComponentOption);
        if (sourceComponentPattern.isEmpty()) {
            qFatal("Missing source component pattern");
        }

        sourceGroups = selectComponents(config, sourceComponentPattern);
        sourceAction = parser.value(sourceActionOption);
    }
    if (sourceGroups.isEmpty()) {
        return 0; // nothing to do
    }

    KConfigGroup targetGroup;
    QString targetComponentPath;
    QString targetAction;
    QString targetDefaultShortcut;
    QString targetDisplayName;
    if (parser.isSet(targetComponentOption)) {
        targetComponentPath = parser.value(targetComponentOption);
        if (targetComponentPath.isEmpty()) {
            qFatal("Invalid target component");
        }
        targetGroup = resolveGroup(config, targetComponentPath);
        targetAction = parser.value(targetActionOption);
    } else if (parser.isSet(targetDesktopFileOption)) {
        if (sourceGroups.size() > 1) {
            qFatal("Invalid input: expected one source componet");
        }

        QString desktopFileName = parser.value(targetDesktopFileOption);
        if (!QFileInfo(desktopFileName).isAbsolute()) {
            desktopFileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kglobalaccel/") + desktopFileName);
        }
        if (!KDesktopFile::isDesktopFile(desktopFileName)) {
            qFatal("%s is not a desktop file", qPrintable(desktopFileName));
        }

        const KDesktopFile desktopFile(desktopFileName);
        targetGroup = resolveGroup(config, QLatin1String("/services/") + QFileInfo(desktopFile.fileName()).fileName());

        const QString actionName = parser.value(targetDesktopFileActionOption);
        if (!actionName.isEmpty()) {
            const KConfigGroup actionGroup = desktopFile.actionGroup(actionName);
            if (!actionGroup.exists()) {
                qFatal("Specified action %s does not exist", qPrintable(actionName));
            }
            targetAction = actionName;
            targetDefaultShortcut = actionGroup.readEntry<QString>(QLatin1String("X-KDE-Shortcuts"), QString());
            targetDisplayName = actionGroup.readEntry<QString>(QLatin1String("Name"), QString());
        } else if (const KConfigGroup group = desktopFile.desktopGroup(); group.hasKey(QLatin1String("X-KDE-Shortcuts"))) {
            targetAction = QLatin1String("_launch");
            targetDefaultShortcut = group.readEntry<QString>(QLatin1String("X-KDE-Shortcuts"), QString());
            targetDisplayName = desktopFile.readName();
        }
    }

    for (KConfigGroup sourceGroup : std::as_const(sourceGroups)) {
        KConfigGroup effectiveTargetGroup;
        if (targetComponentPath.endsWith(QLatin1Char('/'))) {
            effectiveTargetGroup = targetGroup.group(sourceGroup.name());
        } else {
            effectiveTargetGroup = targetGroup;
        }

        if (!sourceAction.isEmpty()) {
            (Migration{
                 .sourceGroup = sourceGroup,
                 .sourceActionName = sourceAction,
                 .targetGroup = effectiveTargetGroup,
                 .targetActionName = targetAction,
                 .targetDefaultShortcut = targetDefaultShortcut,
                 .targetDisplayName = targetDisplayName,
             })
                .execute();
        } else {
            const QStringList entries = sourceGroup.keyList();
            for (const QString &entry : entries) {
                (Migration{
                     .sourceGroup = sourceGroup,
                     .sourceActionName = entry,
                     .targetGroup = effectiveTargetGroup,
                     .targetActionName = targetAction,
                     .targetDefaultShortcut = targetDefaultShortcut,
                     .targetDisplayName = targetDisplayName,
                 })
                    .execute();
            }
        }
    }

    config.sync();
    return 0;
}
