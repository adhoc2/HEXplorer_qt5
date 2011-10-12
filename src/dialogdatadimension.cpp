// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
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
