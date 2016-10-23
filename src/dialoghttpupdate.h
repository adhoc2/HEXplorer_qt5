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

#ifndef DIALOGHTTPUPDATE_H
#define DIALOGHTTPUPDATE_H

#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include "dialogprogressdownload.h"

class MDImain;
class DialogHttpUpdate;
class QNetworkAccessManager;


class HttpUpdater : public QObject
{
    Q_OBJECT

public:
    HttpUpdater(QWidget *mainApp, bool display, DialogHttpUpdate *parent = 0);
    void getXml(const QUrl& url);
    void downloadInstaller(const QUrl& url);
    void launchInstaller();
    QString saveFileName(const QUrl &url);

    QNetworkProxy proxy;
    QNetworkAccessManager manager;
    QNetworkReply *requestXml;
    QNetworkReply *requestInstaller;
    QFile binFile;
    int cfgId;
    int binId;
    QUrl binUrl;
    QString updateFilePath;
    QString updateDetails;

public slots:
    void getXmlFinished(QNetworkReply *reply);
    void downloadReadyRead();
    void downloadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    bool displayUptoDate;
    DialogProgressDownload progressBar;
    DialogHttpUpdate *parent;
    MDImain *mdiMain;
    QString newVersion;
    QTime downloadTime;

private slots:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);

};



class DialogHttpUpdate : public QObject
{
    Q_OBJECT

public:
    explicit DialogHttpUpdate(const QUrl& url, bool display, QWidget *parent = 0);
    ~DialogHttpUpdate(void);

private:
    HttpUpdater *updater;

};

#endif // DIALOGHTTPUPDATE_H
