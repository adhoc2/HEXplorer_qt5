#include "dialogchoosemodule.h"

DialogChooseModule::DialogChooseModule(QString *str, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    module = str;
}

void DialogChooseModule::setList(QStringList list)
{
    listWidget->addItems(list);
}



void DialogChooseModule::on_buttonBox_accepted()
{
    *module = listWidget->selectedItems().at(0)->text();
}
