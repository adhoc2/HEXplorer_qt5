#ifndef DIALOGCHOOSEEXPORTFORMAT_H
#define DIALOGCHOOSEEXPORTFORMAT_H

#include <QDialog>

namespace Ui {
    class DialogChooseExportFormat;
}

class DialogChooseExportFormat : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChooseExportFormat(QString *format, QWidget *parent = 0);
    ~DialogChooseExportFormat();

protected:
    void changeEvent(QEvent *e);

private:
    QString *_format;
    Ui::DialogChooseExportFormat *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // DIALOGCHOOSEEXPORTFORMAT_H
