#ifndef DIALOGCHOOSEMODULE_H
#define DIALOGCHOOSEMODULE_H

#include "ui_dialogchoosemodule.h"

class DialogChooseModule : public QDialog, private Ui::DialogChooseModule
{
    Q_OBJECT

public:
    explicit DialogChooseModule(QString *str, QWidget *parent = 0);
    void setList(QStringList list);

private:
    QString *module;

private slots:
    void on_buttonBox_accepted();
};

#endif // DIALOGCHOOSEMODULE_H
