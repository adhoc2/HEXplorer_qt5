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

class ObdMergeModel : public QAbstractTableModel
{
    public:
        ObdMergeModel(SrecFile *srec, QObject *parent = 0);
        ~ObdMergeModel();

        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        void sort(int column, Qt::SortOrder order=Qt::AscendingOrder);

    private:
        QMap<QString, Data*> listDataInc;
        QMap<QString, Data*> listDataDec;
        QMap<QString, Data*> listDataPreThd;
        QMap<QString, Data*> listDataPrio;
        QMap<QString, Data*> listDataAgiCycIdn;
        QMap<QString, Data*> listDataAgiCycThd;
        QMap<QString, Data*> listDataOperCycIdn;
        QMap<QString, Data*> listDataOperCycThd;
        QMap<QString, Data*> listDataExclsnCdn;

        QMap<QString, Data*> listDataFnR0;
        QMap<QString, Data*> listDataFnR1;
        QMap<QString, Data*> listDataFnR2;
        QMap<QString, Data*> listDataFnR3;
        QMap<QString, Data*> listDataFnR4;
        SrecFile *srec;
        int nColumn;
        int nRow;

};

#endif // OBDMERGEMODEL_H
