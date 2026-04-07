/*
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dbusconnection_p.h"

#include <QString>

QDBusConnection kglobalaccelDBusConnection()
{
    static QDBusConnection connection =
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, QStringLiteral("kglobalacceld"));
    return connection;
}
