#ifndef DIALOGDATADIMENSION_H
#define DIALOGDATADIMENSION_H

#include "ui_dialogdatadimension.h"

class DialogDataDimension : public QDialog, private Ui::DialogDataDimension
{
    Q_OBJECT

public:
    explicit DialogDataDimension(int &xAct, int xMax, int &yAct, int yMax, QWidget *parent = 0);

private slots:
    void on_buttonBox_accepted();

private:
    int *x;
    int *y;
};

#endif // DIALOGDATADIMENSION_H
