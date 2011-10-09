// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

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
