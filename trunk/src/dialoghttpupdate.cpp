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

#include "dialoghttpupdate.h"

#include "mdimain.h"
#include <QByteArray>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QTime>
#include <QSysInfo>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>

DialogHttpUpdate::DialogHttpUpdate(const QUrl& cfgUrl, bool display, QWidget *mdiMain)
{
    //Save Proxy settings with QSettings
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    if (!settings.contains("Update/automatic"))
        settings.setValue("Update/automatic", true);
    if (!settings.contains("Proxy/behindProxy"))
        settings.setValue("Proxy/behindProxy", false);
    if (!settings.contains("Proxy/HostName"))
        settings.setValue("Proxy/HostName", "");
    if (!settings.contains("Proxy/Port"))
        settings.setValue("Proxy/Port", "");
    if (!settings.contains("Proxy/Password"))
        settings.setValue("Proxy/Password", "");
    if (!settings.contains("Proxy/User"))
        settings.setValue("Proxy/User", "");

     //QNetworkManager
    d = new HttpUpdater(mdiMain, display, this);

    //configure proxy
    bool bl = settings.value("Proxy/behindProxy").toBool();
    if (bl)
    {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpCachingProxy);
        proxy.setHostName(settings.value("Proxy/HostName").toString());
        proxy.setPort(settings.value("Proxy/Port").toInt());
        d->manager.setProxy(proxy);
    }

    //check if updates are available
    d->getXml(cfgUrl);
}

DialogHttpUpdate::~DialogHttpUpdate(void)
{

}

////////////////////////////////////////////////////////////////////////////
//                        Class HTTPUPDATER                               //
////////////////////////////////////////////////////////////////////////////

HttpUpdater::HttpUpdater(QWidget *mainApp, bool display, DialogHttpUpdate *par)
{
    displayUptoDate = display;
    parent = par;
    mdiMain = (MDImain*)mainApp;

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(getXmlFinished(QNetworkReply*)));
    connect(&manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(&manager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
           this, SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
}

void HttpUpdater::getXml(const QUrl& url)
{
    QNetworkRequest request(url);
    requestXml = manager.get(request);
}

QString HttpUpdater::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = qApp->applicationDirPath() + "/" + QFileInfo(path).fileName();
    //QString basename = qApp->applicationDirPath() + "/update_HEXplorer.exe";

    if (basename.isEmpty())
        basename = "download";

    return basename;
}

void HttpUpdater::downloadInstaller(const QUrl& url)
{
    //download the file from net into local place
    QString filename = saveFileName(url);
    binFile.setFileName(filename);
    if (!binFile.open(QIODevice::WriteOnly))
    {

        QMessageBox::warning(0, "HEXplorer::update", "Problem opening file " +
                           filename + " for download at " + url.toEncoded().constData() +
                           " : " + url.toEncoded().constData() + "\n",
                              QMessageBox::Ok, QMessageBox::Cancel);

        return;

        return;
    }

    QNetworkRequest request(url);
    requestInstaller = manager.get(request);

    connect(requestInstaller, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(requestInstaller, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(requestInstaller, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));

    downloadTime.start();
}

void HttpUpdater::launchInstaller()
{
    //update Installation_UserList file
    QFileInfo info(updateFilePath);
    QString fileName = info.fileName();
    QString application = qApp->applicationDirPath() + "/" + fileName;

     QProcess *myProcess = new QProcess();
     myProcess->start(application);

     qDebug() << application;

     if(myProcess->waitForStarted())
     {
         mdiMain->close();
     }
}

//// SLOTS ////

void HttpUpdater::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    auth->setUser(settings.value("Proxy/User").toString());
    auth->setPassword(settings.value("Proxy/Password").toString());
}

void HttpUpdater::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    auth->setUser(settings.value("Proxy/User").toString());
    auth->setPassword(settings.value("Proxy/Password").toString());
}

void HttpUpdater::getXmlFinished(QNetworkReply *reply)
{
    if (reply == requestXml)
    {
        //if xml file could not be downloaded
        if (reply->error())
        {
            QMessageBox::warning(0, "HEXplorer::update", QString(reply->errorString()) +
                                  "\n\n" +
                                  "HEXplorer could not read update.xml file for available updates.\n\n" +
                                  "Please control or configure your internet connection \n" +
                                  "setting properly the proxy parameters if you are behind a proxy\n" +
                                  "or simply disabling the automatic updates into Edit/Settings.\n",
                                  QMessageBox::Ok, QMessageBox::Cancel);

            return; //exit download
        }

        if (!reply->open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(0, "HEXplorer::update", "Unable to open update.xml file",
                                  QMessageBox::Ok, QMessageBox::Cancel);

            return;
        }
        else
        {
            QByteArray data = reply->readAll();
            QXmlStreamReader reader(data);
            while (!reader.atEnd())
            {
                reader.readNext();
                if (reader.isStartElement() && reader.attributes().hasAttribute("version"))
                {
                    newVersion = reader.attributes().value("version").toString();
                    QString old_Version = qApp->applicationVersion();
                    QStringList oldList = old_Version.split(".");
                    QStringList newList = newVersion.split(".");
                    if (newList.at(0).toDouble() > oldList.at(0).toDouble())
                    {
                        reader.readNext();
                        if(reader.isCharacters())
                        {
                            updateFilePath = reader.text().toString();
                        }
                    }
                    else if (newList.at(0).toDouble() == oldList.at(0).toDouble())
                    {
                        if (newList.at(1).toDouble() > oldList.at(1).toDouble())
                        {
                            reader.readNext();
                            if(reader.isCharacters())
                            {
                                updateFilePath = reader.text().toString();
                            }
                        }
                        else if (newList.at(1).toDouble() == oldList.at(1).toDouble())
                        {
                            if (newList.at(2).toDouble() > oldList.at(2).toDouble())
                            {
                                reader.readNext();
                                if(reader.isCharacters())
                                {
                                    updateFilePath = reader.text().toString();
                                }
                            }
                        }
                    }
                }
                else if (reader.isStartElement() && reader.qualifiedName() == "details")
                {
                    reader.readNext();
                    if(reader.isCharacters())
                    {
                        updateDetails = reader.text().toString();
                    }
                }
            }

            //display streamreader error
            if (reader.error())
            {
                QMessageBox::critical(0, "HEXplorer::update", reader.errorString(),
                                      QMessageBox::Ok, QMessageBox::Cancel);
                return;
            }

            //if update is or not available, do:
            if(updateFilePath.isEmpty())
            {
                if (displayUptoDate)
                {
                    QMessageBox::information(0, "HEXplorer::update", "You are up to date at version "
                                             + qApp->applicationVersion(),
                                             QMessageBox::Ok, QMessageBox::Cancel);
                }
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setIconPixmap(QPixmap(":/icones/updates.png").scaled(80,80));
                msgBox.setText("A new update is available :\n - current version : " + qApp->applicationVersion() + "\n - new version : " + newVersion);
                msgBox.setInformativeText("would you like to download it?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                msgBox.setDetailedText(updateDetails);
                int ret = msgBox.exec();

                if (ret == QMessageBox::Yes)
                {
                    binUrl.setUrl(updateFilePath);
                    downloadInstaller(binUrl);;
                }
                else
                    return;
            }
        }
    }
}

void HttpUpdater::downloadReadyRead()
{
    binFile.write(requestInstaller->readAll());
}

void HttpUpdater::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    progressBar.show();
    progressBar.setStatus(bytesReceived, bytesTotal);

    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    progressBar.setMessage(QFileInfo(updateFilePath).fileName() + " " + QString::fromLatin1("%1 %2")
                          .arg(speed, 3, 'f', 1).arg(unit));

}

void HttpUpdater::downloadFinished()
{
    progressBar.hide();
    binFile.close();

    if (requestInstaller->error())
    {
        QMessageBox::warning(0, "HEXplorer::update", QString(requestInstaller->errorString()) +
                              "\n\n" +
                              "Please control your internet connection \n" +
                              "or set the proxy parameters properly into Edit/Settings",
                              QMessageBox::Ok, QMessageBox::Cancel);

        return; //exit
    }

    QMessageBox msgBox;
    msgBox.setIconPixmap(QPixmap(":/icones/updates.png").scaled(80,80));
    msgBox.setText("Download completed.");
    msgBox.setInformativeText("Would you like to install ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    //update user_List
    if (ret == QMessageBox::Yes)
    {
        //execute the downloaded file
        launchInstaller();
    }
    else
        return;
}
