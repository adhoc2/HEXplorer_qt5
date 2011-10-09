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
