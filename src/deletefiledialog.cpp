#include "deletefiledialog.h"
#include "ui_deletefiledialog.h"

DeleteFileDialog::DeleteFileDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::DeleteFileDialog)
{
    ui->setupUi(this);
}


DeleteFileDialog::~DeleteFileDialog()
{
    delete ui;
}

bool DeleteFileDialog::deletePermanently()
{
    return ui->checkBox->isChecked();
}

void DeleteFileDialog::setFileNames(QStringList list)
{
    ui->listWidget->addItems(list);
}
