#include "dialogdatadimension.h"

DialogDataDimension::DialogDataDimension(int &xAct, int xMax, int &yAct, int yMax, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    spinBox_2->setValue(xAct);
    spinBox_2->setMinimum(1);
    spinBox_2->setMaximum(xMax);

    spinBox->setValue(yAct);
    spinBox->setMinimum(1);
    spinBox->setMaximum(yMax);

    x = &xAct;
    y = &yAct;
}

void DialogDataDimension::on_buttonBox_accepted()
{
    *x = spinBox_2->value();
    *y = spinBox->value();
}
