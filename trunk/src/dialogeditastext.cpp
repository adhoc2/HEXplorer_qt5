#include "dialogeditastext.h"

DialogEditAsText::DialogEditAsText(Data *data, QWidget *parent) :   QDialog(parent)
{
    setupUi(this);
    setWindowTitle("HEXplorer :: edit as Text");

    myData = data;

    // convert data into Text
    lineEdit->setText(convertData2Text());

}

QString DialogEditAsText::convertData2Text()
{
    QString str = "";

    double c;
    for (int i = 0; i < myData->xCount(); i++)
    {
        QString val = myData->getZ(i);
        c = val.toDouble();
        if (32 <= c && c < 127)
            str.append((unsigned char)c);
    }

    return str;
}

void DialogEditAsText::on_buttonBox_accepted()
{
    QString text = lineEdit->text();

    int asciiVal;
    for(int i = 0; i < myData->xCount(); i++)
    {
        if (i < text.length())
        {
            asciiVal = text.at(i).toAscii();
            myData->setZ(i, QString::number(asciiVal));
        }
        else
        {
             myData->setZ(i, "0");
        }
    }
}
