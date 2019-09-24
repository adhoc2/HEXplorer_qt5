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

#ifndef OBDMERGEMODEL_H
#define OBDMERGEMODEL_H

#include <QAbstractTableModel>
#include <Nodes/characteristic.h>
#include "srecfile.h"
#include "errorcode.h"


class ObdMergeModel : public QAbstractTableModel
{
    public:
        ObdMergeModel(SrecFile *srec, QObject *parent = 0);
        ~ObdMergeModel();

        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        int getRow(const int col, const QString text) const;
        Data* getData(const int row, const int col) const;
        QVariant data(const QModelIndex &index, int role) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool resetData(const QModelIndexList &indexList, int role);
        bool undoData(const QModelIndexList &indexList, int role);
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        void sort(int column, Qt::SortOrder order=Qt::AscendingOrder);

    private:
        QList<ErrorCode*> listErrorCode;
        SrecFile *srec;
        int nColumn;
        int nRow;

    signals:
        void obdDataChanged() {return;}


};

#endif // OBDMERGEMODEL_H
