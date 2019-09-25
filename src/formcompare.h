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

#ifndef FORMCOMPARE_H
#define FORMCOMPARE_H

#include <QWidget>
#include <QModelIndex>
#include <QTreeView>
#include <QStringListModel>

class HexFile;
class SrecFile;
class A2LFILE;
class SpTableModel;
class TreeModelCompleter;
class A2lTreeModel;
class DiffModel;
class Data;
class FUNCTION;
class Csv;
class CdfxFile;
class Dcm;

namespace Ui
{
    class FormCompare;
}

class FormCompare : public QWidget
{
    Q_OBJECT

    public:
        FormCompare(A2lTreeModel* model, QTreeView *tree, QTreeView *tree_2 ,QWidget *parent = 0);
        ~FormCompare();
        void resetModel();
        QStringList charList;
        void setDataset1(QString str);
        void setDataset2(QString str);
        A2lTreeModel *getDiffModel();
        HexFile *getHex1();
        HexFile *getHex2();
        SrecFile *getSrec1();
        SrecFile *getSrec2();
        Csv *getCsv1();
        Csv *getCsv2();
        Dcm *getDcm1();
        Dcm *getDcm2();
        CdfxFile *getCdf1();
        CdfxFile *getCdf2();


    protected:
        void changeEvent(QEvent *e);

    private:
        HexFile *hex1;
        HexFile *hex2;
        SrecFile *srec1;
        SrecFile *srec2;
        Csv *csv1;
        Csv *csv2;
        Dcm *dcm1;
        Dcm *dcm2;
        CdfxFile *cdfx1;
        CdfxFile *cdfx2;
        A2LFILE *a2l1;
        A2LFILE *a2l2;
        Ui::FormCompare *ui;
        A2lTreeModel *treeModel;
        QAbstractTableModel *tableModel;
        A2lTreeModel *diffModel;
        QTreeView *treeView;
        QTreeView *treeView_2;
        QWidget *mainWidget;

    private slots:
        void on_export_labels_clicked();
        void on_lineEdit_2_textChanged(QString );
        void on_lineEdit_textChanged(QString );
        void on_export_subset_clicked();
        void on_copy_clicked();
        void on_checkBoxTrg_clicked();
        void on_checkBoxSrc_clicked();
        void checkDroppedFile(QString );
        void checkDroppedFile_2(QString );
        void on_choose_clicked();        

public slots:
        void on_quicklook_clicked();
        void on_compare_clicked();

    signals:
            void incCompare(int i = 0);

};

#endif // FORMCOMPARE_H
