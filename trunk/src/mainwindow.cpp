// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

#include <QtGui>

#include "locationdialog.h"
#include "mainwindow.h"
#include "settingstree.h"

MainWindow::MainWindow()
{
    settingsTree = new SettingsTree;
    setCentralWidget(settingsTree);

    locationDialog = 0;

    createActions();
    createMenus();

    autoRefreshAct->setChecked(true);
    fallbacksAct->setChecked(true);

    setWindowTitle(tr("Settings Editor"));


    QSettings *settings = new QSettings(QSettings::NativeFormat,
                                        QSettings::UserScope,
                                        "Ch.Ho�l",
                                        "HEXplorer");
    setSettingsObject(settings);
    fallbacksAct->setEnabled(true);

    resize(500, 600);
}

void MainWindow::openSettings()
{
    if (!locationDialog)
        locationDialog = new LocationDialog(this);

    if (locationDialog->exec()) {
        QSettings *settings = new QSettings(locationDialog->format(),
                                            locationDialog->scope(),
                                            locationDialog->organization(),
                                            locationDialog->application());
        setSettingsObject(settings);
        fallbacksAct->setEnabled(true);
    }
}

void MainWindow::openIniFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open INI File"),
                               "", tr("INI Files (*.ini *.conf)"));
    if (!fileName.isEmpty()) {
        QSettings *settings = new QSettings(fileName, QSettings::IniFormat);
        setSettingsObject(settings);
        fallbacksAct->setEnabled(false);
    }
}

void MainWindow::openPropertyList()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                               tr("Open Property List"),
                               "", tr("Property List Files (*.plist)"));
    if (!fileName.isEmpty()) {
        QSettings *settings = new QSettings(fileName, QSettings::NativeFormat);
        setSettingsObject(settings);
        fallbacksAct->setEnabled(false);
    }
}

void MainWindow::openRegistryPath()
{
    QString path = QInputDialog::getText(this, tr("Open Registry Path"),
                           tr("Enter the path in the Windows registry:"),
                           QLineEdit::Normal, "HKEY_CURRENT_USER\\");
    if (!path.isEmpty()) {
        QSettings *settings = new QSettings(path, QSettings::NativeFormat);
        setSettingsObject(settings);
        fallbacksAct->setEnabled(false);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Settings Editor"),
            tr("The <b>Settings Editor</b> example shows how to access "
               "application settings using Qt."));
}

void MainWindow::createActions()
{
    openSettingsAct = new QAction(tr("&Open Application Settings..."), this);
    openSettingsAct->setShortcuts(QKeySequence::Open);
    connect(openSettingsAct, SIGNAL(triggered()), this, SLOT(openSettings()));

    openIniFileAct = new QAction(tr("Open I&NI File..."), this);
    openIniFileAct->setShortcut(tr("Ctrl+N"));
    connect(openIniFileAct, SIGNAL(triggered()), this, SLOT(openIniFile()));

    openPropertyListAct = new QAction(tr("Open Mac &Property List..."), this);
    openPropertyListAct->setShortcut(tr("Ctrl+P"));
    connect(openPropertyListAct, SIGNAL(triggered()),
            this, SLOT(openPropertyList()));

    openRegistryPathAct = new QAction(tr("Open Windows &Registry Path..."),
                                      this);
    openRegistryPathAct->setShortcut(tr("Ctrl+G"));
    connect(openRegistryPathAct, SIGNAL(triggered()),
            this, SLOT(openRegistryPath()));

    refreshAct = new QAction(tr("&Refresh"), this);
    refreshAct->setShortcut(tr("Ctrl+R"));
    refreshAct->setEnabled(false);
    connect(refreshAct, SIGNAL(triggered()), settingsTree, SLOT(refresh()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    autoRefreshAct = new QAction(tr("&Auto-Refresh"), this);
    autoRefreshAct->setShortcut(tr("Ctrl+A"));
    autoRefreshAct->setCheckable(true);
    autoRefreshAct->setEnabled(false);
    connect(autoRefreshAct, SIGNAL(triggered(bool)),
            settingsTree, SLOT(setAutoRefresh(bool)));
    connect(autoRefreshAct, SIGNAL(triggered(bool)),
            refreshAct, SLOT(setDisabled(bool)));

    fallbacksAct = new QAction(tr("&Fallbacks"), this);
    fallbacksAct->setShortcut(tr("Ctrl+F"));
    fallbacksAct->setCheckable(true);
    fallbacksAct->setEnabled(false);
    connect(fallbacksAct, SIGNAL(triggered(bool)),
            settingsTree, SLOT(setFallbacksEnabled(bool)));

#ifndef Q_WS_MAC
    openPropertyListAct->setEnabled(false);
#endif
#ifndef Q_WS_WIN
    openRegistryPathAct->setEnabled(false);
#endif
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openSettingsAct);
    fileMenu->addAction(openIniFileAct);
    fileMenu->addAction(openPropertyListAct);
    fileMenu->addAction(openRegistryPathAct);
    fileMenu->addSeparator();
    fileMenu->addAction(refreshAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(autoRefreshAct);
    optionsMenu->addAction(fallbacksAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::setSettingsObject(QSettings *settings)
{
    settings->setFallbacksEnabled(fallbacksAct->isChecked());
    settingsTree->setSettingsObject(settings);

    refreshAct->setEnabled(true);
    autoRefreshAct->setEnabled(true);

    QString niceName = settings->fileName();
    niceName.replace("\\", "/");
    int pos = niceName.lastIndexOf("/");
    if (pos != -1)
        niceName.remove(0, pos + 1);

    if (!settings->isWritable())
        niceName = tr("%1 (read only)").arg(niceName);

    setWindowTitle(tr("%1 - %2").arg(niceName).arg(tr("Settings Editor")));
}
