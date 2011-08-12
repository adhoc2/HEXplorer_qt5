#ifndef DIALOGEDITASTEXT_H
#define DIALOGEDITASTEXT_H

#include "ui_dialogeditastext.h"
#include "data.h"

class DialogEditAsText : public QDialog, private Ui::DialogEditAsText
{
    Q_OBJECT

public:
    explicit DialogEditAsText(Data *data, QWidget *parent = 0);

private:
    QString convertData2Text();
    Data *myData;

private slots:
    void on_buttonBox_accepted();
};

#endif // DIALOGEDITASTEXT_H
