#include "dialogchooseexportformat.h"
#include "ui_dialogchooseexportformat.h"

DialogChooseExportFormat::DialogChooseExportFormat(QString *format, QWidget *parent) :  QDialog(parent),   ui(new Ui::DialogChooseExportFormat)
{
    ui->setupUi(this);
    _format = format;
}

DialogChooseExportFormat::~DialogChooseExportFormat()
{
    delete ui;
}

void DialogChooseExportFormat::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DialogChooseExportFormat::on_buttonBox_accepted()
{
    if (ui->radioButton->isChecked())
        *_format = "csv";
    else if (ui->radioButton_2->isChecked())
        *_format = "cdf";
}
