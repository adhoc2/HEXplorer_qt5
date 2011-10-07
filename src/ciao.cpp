#include "ciao.h"

Ciao::Ciao(bool &bl, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    _bool = &bl;
}

void Ciao::on_checkBox_clicked()
{

}

void Ciao::on_buttonBox_accepted()
{
    if (checkBox->isChecked())
    {
        *_bool = false;
    }
    else
    {
        *_bool = true;
    }
}
