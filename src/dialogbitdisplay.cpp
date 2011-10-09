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

#include "dialogbitdisplay.h"

using namespace std;

DialogBitDisplay::DialogBitDisplay(ulong & value, int length, QWidget *parent) : QDialog(parent)
{
    setupUi(this);   

    myValue = &value;
    bitset<32> myBitset(static_cast<unsigned long long>(value));
    listRadio.append(radioButton_1);
    listRadio.append(radioButton_2);
    listRadio.append(radioButton_3);
    listRadio.append(radioButton_4);
    listRadio.append(radioButton_5);
    listRadio.append(radioButton_6);
    listRadio.append(radioButton_7);
    listRadio.append(radioButton_8);
    listRadio.append(radioButton_9);
    listRadio.append(radioButton_10);
    listRadio.append(radioButton_11);
    listRadio.append(radioButton_12);
    listRadio.append(radioButton_13);
    listRadio.append(radioButton_14);
    listRadio.append(radioButton_15);
    listRadio.append(radioButton_16);
    listRadio.append(radioButton_17);
    listRadio.append(radioButton_18);
    listRadio.append(radioButton_19);
    listRadio.append(radioButton_20);
    listRadio.append(radioButton_21);
    listRadio.append(radioButton_22);
    listRadio.append(radioButton_23);
    listRadio.append(radioButton_24);
    listRadio.append(radioButton_25);
    listRadio.append(radioButton_26);
    listRadio.append(radioButton_27);
    listRadio.append(radioButton_28);
    listRadio.append(radioButton_29);
    listRadio.append(radioButton_30);
    listRadio.append(radioButton_31);
    listRadio.append(radioButton_32);

    for (int i = 0; i < listRadio.length();i++)
    {

        if (i < length)
        {
            listRadio.at(i)->setChecked(myBitset.test(i));
            listRadio.at(i)->setEnabled(true);
        }
        else
        {
            listRadio.at(i)->setEnabled(false);
        }
    }
}

void DialogBitDisplay::on_buttonBox_accepted()
{
    for (int i = 0; i < listRadio.length();i++)
    {
        myBitset.set(i, listRadio.at(i)->isChecked());
    }

     *myValue = myBitset.to_ulong();
}
