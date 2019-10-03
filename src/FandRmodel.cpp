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

#include "FandRModel.h"
#include "Nodes/compu_method.h"
#include <QFont>
#include "qdebug"

FandRModel::FandRModel(SrecFile *srecFile, QObject *parent)
{
    nRow = 0;
    nColumn = 0;
    srec = srecFile;

    //fill-in listData
    foreach (Data *data, srec->listData)
    {
        QString nameStr(data->name);
        if (nameStr.toLower().endsWith(".prio"))
        {
            listDataPrio.insert(data->getComment(), data);
        }
        else if (nameStr.toLower().endsWith("a[0].fltreactnid"))
        {
            listDataFnR0.insert(data->getComment(), data);
        }       
        else if (nameStr.toLower().endsWith("a[1].fltreactnid"))
        {
            listDataFnR1.insert(data->getComment(), data);
        }
        else if (nameStr.toLower().endsWith("a[2].fltreactnid"))
        {
            listDataFnR2.insert(data->getComment(), data);
        }
        else if (nameStr.toLower().endsWith("a[3].fltreactnid"))
        {
            listDataFnR3.insert(data->getComment(), data);
        }
        else if (nameStr.toLower().endsWith("a[4].fltreactnid"))
        {
            listDataFnR4.insert(data->getComment(), data);
        }
    }

    nRow = listDataFnR0.count();
    nColumn = 8;

}

FandRModel::~FandRModel()
{

}

int FandRModel::rowCount(const QModelIndex &parent) const
{
    return nRow;
}

int FandRModel::columnCount(const QModelIndex &parent) const
{
    return nColumn;
}

QVariant FandRModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (0 <= row && row < nRow)
    {

        QString key = listDataFnR0.keys().at(row);
        Data *label0 = listDataFnR0.value(key, nullptr);

        switch (role)
        {
        case Qt::DisplayRole :
            {
                if (column == 0)
                {
                    return key.remove("DTC-ID: ");
                }
                else if (column == 1)
                {
                    if (label0)
                        return label0->name;
                }
                else if (column == 3)
                {
                    if (label0)
                    {
                        QString str = label0->getZ(0);
                        return str;
                    }
                }
                else if (column == 2)
                {
                    Data *labelPrio = listDataPrio.value(key, nullptr);
                    if (labelPrio)
                    {
                        QString str = labelPrio->getZ(0);
                        return str;
                    }
                }
                else if (column == 4)
                {
                    Data *label1 = listDataFnR1.value(key, nullptr);
                    if (label1)
                    {
                        QString str = label1->getZ(0);
                        return str;
                    }
                }
                else if (column == 5)
                {

                    Data *label2 = listDataFnR2.value(key, nullptr);
                    if (label2)
                    {
                        QString str = label2->getZ(0);
                        return str;
                    }
                }
                else if (column == 6)
                {

                    Data *label3 = listDataFnR3.value(key, nullptr);
                    if (label3)
                    {
                        QString str = label3->getZ(0);
                        return str;
                    }
                }
                else if (column == 7)
                {

                    Data *label4 = listDataFnR4.value(key, nullptr);
                    if (label4)
                    {
                        QString str = label4->getZ(0);
                        return str;
                    }
                }
                else
                    return QVariant();

            }
            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
            break;

        case Qt::ToolTipRole:
            break;

        case Qt::StatusTipRole: // The data displayed in the status bar.
            break;

        case Qt::WhatsThisRole: // The data displayed for the item in "What's This?" mode.
            break;

        case Qt::SizeHintRole: // The size hint for the item that will be supplied to views.
            break;

        case Qt::FontRole : // The font used for items rendered with the default delegate.
            {
                if (column == 0)
                {
                   QFont font;
                   font.setBold(true);
                   return font;
                }
            }
            break;

        case Qt::TextAlignmentRole:
            break;

        case Qt::BackgroundRole:
            break;

        case Qt::ForegroundRole: // the foreground brush (text color, typically) used for items rendered with the default delegate.
            {
                if (column == 0)
                {
                    //QColor color = Qt::blue;
                    //return color;
                }
            }
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags FandRModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant FandRModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole :
            {
            if (orientation == Qt::Horizontal)
            {
                if (section == 1)
                {
                    return "Error name";
                }
                else if (section == 0)
                {
                    return "DTC Id";
                }
                else if (section == 2)
                {
                    return "Prio";
                }
                else if (section == 3)
                {
                    return "reaction #1";
                }
                else if (section == 4)
                {
                    return "reaction #2";
                }
                else if (section == 5)
                {
                    return "reaction #3";
                }
                else if (section == 6)
                {
                    return "reaction #4";
                }
                else if (section == 7)
                {
                    return "reaction #5";
                }
            }
                else
                    return QVariant();

            }
            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
            break;

        case Qt::ToolTipRole:
            break;

        case Qt::StatusTipRole: // The data displayed in the status bar.
            break;

        case Qt::WhatsThisRole: // The data displayed for the item in "What's This?" mode.
            break;

        case Qt::SizeHintRole: // The size hint for the item that will be supplied to views.
            break;

        case Qt::FontRole : // The font used for items rendered with the default delegate.
            {
               QFont font;
               font.setBold(true);
               return font;
            }
            break;

        case Qt::TextAlignmentRole:
            break;

        case Qt::BackgroundRole:
            break;

        case Qt::ForegroundRole: // the foreground brush (text color, typically) used for items rendered with the default delegate.
            {
                QColor color = Qt::red;
                return color;
            }
            break;
    }

    return QVariant();
}
