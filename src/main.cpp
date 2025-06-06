/*
 * Melodeon - LMS Controller
 *
 * Copyright (c) 2022-2023 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "config.h"
#include "debug.h"
#include "mainwindow.h"
#include "settings.h"
#include "startup.h"
#include "singleapplication.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <stdlib.h>

int main(int argc, char *argv[]) {
#ifdef Q_OS_LINUX
    QList<QByteArray> xdgDesktop = qgetenv("XDG_CURRENT_DESKTOP").toLower().split(':');
    QSet<QByteArray> de = QSet<QByteArray>(xdgDesktop.cbegin(), xdgDesktop.cend());
    if (de.contains("gnome")) {
        setenv("QT_QPA_PLATFORM", "xcb", 1);
    }
#endif
    QCoreApplication::setOrganizationName(PROJECT_NAME);
    QCoreApplication::setApplicationName(PROJECT_NAME);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    SingleApplication app(argc, argv, true, SingleApplication::User, SingleApplication::SecondaryNotification);

    if (app.isSecondary()) {
        if (app.arguments().length()==2) {
            app.sendMessage(app.arguments().at(1).toLower().toUtf8());
        }
        return 0;
    } else {
        Debug::init(app.arguments());
        if (Settings::self()->getAddress().isEmpty()) {
            Startup *start = new Startup();
            QObject::connect(&app, &SingleApplication::instanceStarted, [ start ]() {
                start->raise();
                start->activateWindow();
            });
            start->show();
        } else {
            MainWindow *mw = new MainWindow();
            QObject::connect( &app, &SingleApplication::instanceStarted, [ mw ]() {
                mw->raise();
                mw->activateWindow();
            });
            QObject::connect(&app, &SingleApplication::receivedMessage, mw, &MainWindow::receivedMessage);

            bool maximised = false;
            for (const auto &arg: app.arguments()) {
                if (arg.startsWith("--startmax")) {
                    maximised = true;
                }
            }
            if (maximised || Settings::self()->getMaximized()) {
                mw->showNormal();
                QTimer::singleShot(0, mw, SLOT(showMaximized()));
            } else {
                mw->show();
            }
        }

        return app.exec();
    }
}
