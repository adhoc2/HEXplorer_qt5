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

#ifndef DIALOGCSV_H
#define DIALOGCSV_H

#include "ui_dialogcsv.h"
class A2lTreeModel;


class DialogCsv : public QDialog, private Ui::DialogCsv
{
    Q_OBJECT

public:
    explicit DialogCsv(QWidget *parent = 0, QStringList *list = 0);
    void setModel(A2lTreeModel *mod);

private:
    QStringList *listlabel;
    A2lTreeModel *model;

private slots:
    void on_add_all_clicked();
    void on_remove_all_clicked();
    void on_buttonBox_accepted();
};

#endif // DIALOGCSV_H
