#ifndef DIALOGHTTPUPDATE_H
#define DIALOGHTTPUPDATE_H

#include <QtCore>
#include <QtNetwork>
class MDImain;
class DialogHttpUpdate;
class QNetworkAccessManager;


class HttpUpdater : public QObject
{
    Q_OBJECT

public:
    HttpUpdater(QWidget *mainApp, DialogHttpUpdate *parent = 0);
    void check(const QUrl& url);
    void downloadFile(const QUrl& url);
    void installUpdate();

public slots:
    void onRequestFinished(int id, bool error);

public:
    QHttp *http;
    QFile *cfgFile;
    QFile *binFile;
    int cfgId;
    int binId;
    QUrl binUrl;
    QString pathBin;
    QString updateReleaseName;
    QString updateDetails;


private:
    DialogHttpUpdate *parent;
    MDImain *mdiMain;

    QString getUserName();
    QString newVersion;

};



class DialogHttpUpdate : public QObject
{
    Q_OBJECT

public:
    explicit DialogHttpUpdate(const QUrl& url, QWidget *parent = 0);
    ~DialogHttpUpdate(void);

private:
    HttpUpdater *d;
};

#endif // DIALOGHTTPUPDATE_H
