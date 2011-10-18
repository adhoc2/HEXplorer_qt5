#ifndef DIALOGPROGRESSDOWNLOAD_H
#define DIALOGPROGRESSDOWNLOAD_H

#include "ui_dialogprogressdownload.h"

class DialogProgressDownload : public QDialog, private Ui::DialogProgressDownload
{
    Q_OBJECT

public:
    explicit DialogProgressDownload(QWidget *parent = 0);

    void clear();
    void update();
    void setStatus(qint64 bytesReceived, qint64 bytesTotal);
    void setMessage(const QString &m);

private:
    QString message;
    qint64 value;
    qint64 maximum;
};

#endif // DIALOGPROGRESSDOWNLOAD_H
