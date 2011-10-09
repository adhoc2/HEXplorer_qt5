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

#ifndef CHOOSESUBSET_H
#define CHOOSESUBSET_H

#include <QDialog>
#include <QStringList>
class A2LFILE;
class HexFile;

namespace Ui {
    class ChooseSubset;
}

class ChooseSubset : public QDialog {
    Q_OBJECT
public:
    ChooseSubset(A2LFILE *_a2l, HexFile *_hex, QStringList &list,QWidget *parent = 0);
    ~ChooseSubset();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ChooseSubset *ui;
    A2LFILE *a2l;
    HexFile *hex;
    QWidget *mainWidget;
    QStringList *subsetList;

private slots:
    void on_export_2_clicked();
    void on_import_2_clicked();
    void on_lineEdit_textChanged(QString );
    void on_leftButton_clicked();
    void on_add_all_clicked();
    void on_remove_all_clicked();
    void on_buttonBox_accepted();
    void on_rightButton_clicked();
};

#endif // CHOOSESUBSET_H
