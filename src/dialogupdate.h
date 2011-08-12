#ifndef DIALOGUPDATE_H
#define DIALOGUPDATE_H

#include <QtCore>
#include <QtNetwork>
class MDImain;

// based on dtkUpdaterPrivate from Inria

class DialogUpdate : public QObject
{
    Q_OBJECT

public:
    explicit DialogUpdate(const QString &path, bool bl, QWidget *parent = 0);
    ~DialogUpdate();

private:
    MDImain *mdiMain;
    QString updateFilePath;
    QString updateDetails;
    void readConfigFile(QString path, bool bl);
    void downloadUpdate();
    void installUpdate();
    QString getUserName();
    QString newVersion;
};

#endif // DIALOGUPDATE_H


