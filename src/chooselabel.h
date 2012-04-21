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

#ifndef CHOOSELABEL_H
#define CHOOSELABEL_H

#include <QTimer>
#include <QDialog>
#include "a2ltreemodel.h"
#include <QListWidgetItem>
#include "treemodelcompleter.h"
class QTreeView;
class A2LFILE;
class HexFile;
class SrecFile;
class Csv;
class ProjectListWidget;
class CdfxFile;

namespace Ui {
    class ChooseLabel;
}

class ChooseLabel : public QDialog
{
    Q_OBJECT

    public:
        ChooseLabel(A2LFILE *_a2l, HexFile *_hex, QWidget *parent = 0);
        ChooseLabel(A2LFILE *_a2l, SrecFile *_srec, QWidget *parent = 0);
        ChooseLabel(A2LFILE *_a2l, Csv *_csv, QWidget *parent = 0);
        ChooseLabel(A2LFILE *_a2l, CdfxFile *_cdfx, QWidget *parent = 0);
        ~ChooseLabel();

    protected:
        void changeEvent(QEvent *e);

    private:
        QTimer timer;
        QAction *deleteRight;
        QAction *leftSelect;
        QAction *rightSelect;
        QWidget *mainWidget;
        TreeModelCompleter *completer;
        Ui::ChooseLabel *ui;
        A2LFILE *a2l;
        HexFile *hex;
        SrecFile *srec;
        Csv *csv;
        CdfxFile *cdfx;

        void createActions();

    private slots:
        void searchItem();
        void selectedItem(QListWidgetItem*,QListWidgetItem*);
        void on_export_2_clicked();
        void on_import_2_clicked();
        void on_leftButton_clicked();
        void on_add_all_clicked();
        void on_remove_all_clicked();
        void on_buttonBox_accepted();
        void on_lineEdit_textChanged(QString str = "");
        void on_rightButton_clicked();
};

#endif // CHOOSELABEL_H
