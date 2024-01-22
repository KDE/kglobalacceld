/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>

#include "move.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.addPositionalArgument(QStringLiteral("subcommand"), QStringLiteral("move"));

    parser.parse(app.arguments());
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return -1;
    }

    const QString subcommand = parser.positionalArguments().first();
    if (subcommand == QLatin1String("move")) {
        return handleMove(parser, app.arguments());
    } else {
        parser.showHelp();
        return -1;
    }
}
