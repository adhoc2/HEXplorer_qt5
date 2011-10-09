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

#ifndef COMPAREMODEL_H
#define COMPAREMODEL_H

#include <QAbstractTableModel>
#include "hexfile.h"
#include "data.h"
#include "a2ltreemodel.h"

class CompareModel : public QAbstractTableModel
{
    public:
        CompareModel(QObject *parent = 0);
        ~CompareModel();

        QList<Data*> *getList1();
        QList<Data*> *getList2();
        QModelIndex getFirstZindex(Data *data);
        void setList(QList<Data*> *labelList, QList<Data*> *labelList2);
        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool setData(const QModelIndex &index, const QModelIndexList &indexList, const QVariant &value, int role);
        bool resetData(const QModelIndex &index, int role);
        bool resetData(const QModelIndexList &indexList, int role);
        bool undoData(const QModelIndexList &indexList, int role);
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        Data *getLabel(const QModelIndex &index, int role);
        QModelIndexList selectAllX(Data *data);
        QModelIndexList selectAllY(Data *data);
        QModelIndexList selectAllZ(Data *data);

    private:
        A2lTreeModel *treeModel;
        QList<Data*> *listDat1;
        QList<Data*> *listDat2;
        QList<int> listDataIndex;
        int nColumn;
        int nRow;

};

#endif // COMPAREMODEL_H
