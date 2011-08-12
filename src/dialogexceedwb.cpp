#include "dialogexceedwb.h"

DialogExceedWB::DialogExceedWB(bool *bl, QWidget *parent) :  QDialog(parent)
{
    again = bl;
    setupUi(this);
}

void DialogExceedWB::on_buttonBox_accepted()
{
    if (checkBox->isChecked())
    {
        *again = false;
    }
    else
    {
        *again = true;
    }
}

void DialogExceedWB::on_buttonBox_rejected()
{
    if (checkBox->isChecked())
    {
        *again = false;
    }
    else
    {
        *again = true;
    }
}
