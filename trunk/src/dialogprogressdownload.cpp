#include "dialogprogressdownload.h"

DialogProgressDownload::DialogProgressDownload(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowTitle("HEXplorer :: download update");
}

void DialogProgressDownload::setStatus(qint64 bytesReceived, qint64 bytesTotal)
{
    progressBar->setMaximum(bytesTotal);
    progressBar->setValue(bytesReceived);

}

void DialogProgressDownload::setMessage(const QString &m)
{
    QString str = "Download in progress : ";
    label->setText(str + m);
    message = m;
}
