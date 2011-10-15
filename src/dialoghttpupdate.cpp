#include "dialoghttpupdate.h"

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

DialogHttpUpdate::DialogHttpUpdate(const QUrl& cfgUrl, QWidget *mdiMain)
{

    QFile::remove("cfg");
    QFile::remove("bin");

    d = new HttpUpdater(mdiMain);
    d->http = new QHttp;
    d->cfgFile = new QFile("cfg");
    d->binFile = new QFile("bin");

    QObject::connect(d->http, SIGNAL(requestFinished(int, bool)), d, SLOT(onRequestFinished(int, bool)));

    d->check(cfgUrl);
}

DialogHttpUpdate::~DialogHttpUpdate(void)
{
    QFile::remove("/HEXplorer.zip");
}

////////////////////////////////////////////////////////////////////////////
//                        Class HTTPUPDATER                               //
////////////////////////////////////////////////////////////////////////////

HttpUpdater::HttpUpdater(QWidget *mainApp, DialogHttpUpdate *par)
{
    parent = par;
    mdiMain = (MDImain*)mainApp;
}

QString HttpUpdater::getUserName()
{
    #ifdef Q_WS_WIN32
        QString userName;
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

void HttpUpdater::onRequestFinished(int id, bool error)
{
    Q_UNUSED(error);

    if (id == cfgId)
    {
        if(!cfgFile->openMode() == QIODevice::NotOpen)
            cfgFile->close();

        if (!cfgFile->open(QFile::ReadOnly | QFile::Text))
            qDebug() << "Unable to open stream for reading.";

        QXmlStreamReader reader(cfgFile);

//        while (!reader.atEnd())
//        {
//            reader.readNext();

//            if (reader.isStartElement() && reader.attributes().hasAttribute("version") && reader.attributes().value("version").toString() > qApp->applicationVersion())
//            {
//                reader.readNext();
//                if(reader.isCharacters()) {
//                    binUrl.setUrl(reader.text().toString());
//                }
//            }
//        }

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

        if (reader.error())
        {
            QMessageBox::critical(0, "HEXplorer::update", reader.errorString(),
                                  QMessageBox::Ok, QMessageBox::Cancel);
            return;
        }

        if(updateFilePath.isEmpty())
        {
            QMessageBox::information(0, "HEXplorer::update", "You are up to date at version "
                                     + qApp->applicationVersion(),
                                  QMessageBox::Ok, QMessageBox::Cancel);
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
                downloadFile(binUrl);;
            }
            else
                return;
        }
    }
    else if (id == binId)
    {

//        if(!binFile->openMode() == QIODevice::NotOpen)
//            binFile->close();

//        qDebug() << "Download completed, would you like to install ?";

//        char c = getchar(); getchar();

//        if(c == 'y')
//            extract();
    }
}

void HttpUpdater::check(const QUrl& url)
{
    if(!cfgFile->open(QFile::ReadWrite))
        qDebug() << "Unable to open config file for parsing";

    http->setHost(url.host(), url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());

    if (!url.userName().isEmpty())
        http->setUser(url.userName(), url.password());

    QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
    if (path.isEmpty())
        path = "/";

    cfgId = http->get(path, cfgFile);
}

void HttpUpdater::downloadFile(const QUrl& url)
{
//    if(!binFile->open(QFile::ReadWrite))
//        qDebug() << "Unable to open binary file for saving";

//    http->setHost(url.host(), url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());

//    if (!url.userName().isEmpty())
//        http->setUser(url.userName(), url.password());

//    QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
//    if (path.isEmpty())
//        path = "/";

//    binId = http->get(path, binFile);

    // download the file from net into local place
     QFile source(updateFilePath);
     if (source.exists())
        qDebug() << updateFilePath;
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
            //execute the downloaded file
            installUpdate();
        }
        else
            return;
    }
}

void HttpUpdater::installUpdate()
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
