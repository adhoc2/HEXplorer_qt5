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

#include "dialogupdate.h"
#include "mdimain.h"
#include <QByteArray>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QTime>
#include <QSysInfo>
#include <QHostInfo>
#ifdef Q_WS_WIN32
    #include "LMCons.h"
    #include "windows.h"
#endif


DialogUpdate::DialogUpdate(const QString &configFile, bool bl, QWidget *mdiMain)
{
    this->mdiMain = (MDImain*)mdiMain;

    //start update process by reading the configFile
    readConfigFile(configFile, bl);
}

DialogUpdate::~DialogUpdate()
{

}

QString DialogUpdate::getUserName()
{
    #ifdef Q_WS_WIN32
        QString userName = "";
        #if defined(UNICODE)
        if ( QSysInfo::windowsVersion () ==  QSysInfo::WV_NT)
        {
            TCHAR winUserName[UNLEN + 1]; // UNLEN is defined in LMCons.h
            DWORD winUserNameSize = sizeof(winUserName);
            GetUserName( winUserName, &winUserNameSize );
            userName = QString::fromUtf16((ushort*)winUserName );
        } else
        #endif
        {
            char winUserName[UNLEN + 1]; // UNLEN is defined in LMCons.h
            DWORD winUserNameSize = sizeof(winUserName);
            GetUserNameA( winUserName, &winUserNameSize );
            userName = QString::fromLocal8Bit( winUserName );
        }
        return userName;
    #endif

}

void DialogUpdate::readConfigFile(QString configFilePath, bool bl)
{
    QFile cfgFile(configFilePath);

    //check if file is open, readable,...
    if(!cfgFile.openMode() == QIODevice::NotOpen)
        cfgFile.close();

    if (!cfgFile.open(QFile::ReadOnly | QFile::Text))
    {
        if (bl)
        {
            QMessageBox::critical(0, "HEXplorer::update", "Unable to open stream for reading.",
                                  QMessageBox::Ok, QMessageBox::Cancel);
        }

        return;
    }

    // parse the file
    QXmlStreamReader reader(&cfgFile);
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

    // close the configFile to release
    cfgFile.close();

    // display error by reading configFile
    if (reader.error())
    {
        QMessageBox::critical(0, "HEXplorer::update", reader.errorString(),
                              QMessageBox::Ok, QMessageBox::Cancel);
        return;
    }

    if (updateFilePath.isEmpty())
    {
        if (bl)
        {
            QMessageBox::information(0, "HEXplorer::update", "You are up to date at version "
                                     + qApp->applicationVersion(),
                                  QMessageBox::Ok, QMessageBox::Cancel);
        }
        return;
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
            downloadUpdate();
        }
        else
            return;
    }
}

void DialogUpdate::downloadUpdate()
{
    // download the file from net into local place
     QFile source(updateFilePath);
     bool bl = source.copy(qApp->applicationDirPath() + "/update_HEXplorer.exe");

     if (bl)
     {

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
            //update Installation_UserList file
            QFileInfo info(updateFilePath);
            QString str = info.absolutePath();
            str = str + "/HEXplorer_UserList.txt";
            QFileInfo fileInfo(str);

            //check if the file exists
            if (fileInfo.isFile())
            {
                //open the file in ReadWrite Mode
                QFile file(str);
                QTime timer;
                timer.start();
                bool isFileOpen = false;
                while( !isFileOpen && (timer.elapsed() < 5000) )
                {
                    isFileOpen = file.open(QIODevice::ReadWrite);
                }

                //once the file is open, do the job
                if (isFileOpen)
                {
                    //read the file into listPc
                    QStringList listPC;
                    while(!file.atEnd())
                    {
                        listPC.append(file.readLine());
                    }

                    //check if the pc is already into the listPc
                    //and if true update the listPc
                    int countPc = listPC.count();
                    bool isPc = false;
                    for (int i = 0; i < countPc; i++)
                    {
                        if (listPC[i].contains(QHostInfo::localHostName()))
                        {
                            isPc = true;
                            listPC[i] = QHostInfo::localHostName() + "\t" +
                                           getUserName() + "\t" +
                                           QDateTime::currentDateTime().toString() + "\t" +
                                           newVersion + "\t" +                                           
                                           QString::number(i + 1) + "\n";
                        }
                    }

                    //if pc is not into the listPc, add a line to the file
                    if (!isPc)
                    {
                        QString line = QHostInfo::localHostName() + "\t" +
                                       getUserName() + "\t" +
                                       QDateTime::currentDateTime().toString() + "\t" +
                                       newVersion + "\t" +                                       
                                       QString::number(countPc + 1) + "\n";
                        listPC.append(line);
                    }

                    //copy the listPc into the file
                    file.seek(0);
                    foreach (QString line, listPC)
                    {
                        file.write(line.toLocal8Bit().data());
                    }

                    // close file for release
                    file.close();
                }
            }

            //unzip the downloaded file into the installation folder
            installUpdate();
        }
        else
            return;
    }

}

void DialogUpdate::installUpdate()
{
    //update Installation_UserList file
    QFileInfo info(updateFilePath);
    QString fileName = info.fileName();
    QString application = qApp->applicationDirPath() + "/" + fileName;

     QProcess *myProcess = new QProcess();
     myProcess->start(application);

     if(myProcess->waitForStarted())
     {
         mdiMain->close();
     }
}
