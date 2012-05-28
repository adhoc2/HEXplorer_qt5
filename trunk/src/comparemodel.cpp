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

#include <QObject>
#include <QFont>
#include "comparemodel.h"
#include "qdebug.h"

CompareModel::CompareModel(QObject *parent) : QAbstractTableModel(parent)
{
    listDat1 = NULL;
    nRow = 0;
    nColumn = 0;
}

CompareModel::~CompareModel()
{
    delete listDat1;
    delete listDat2;
}

int CompareModel::rowCount(const QModelIndex & /* parent */) const
{
    return nRow;
}

int CompareModel::columnCount(const QModelIndex & /* parent */) const
{    
    return nColumn;
}

QVariant CompareModel::data(const QModelIndex &index, int role) const
{    
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    // binary search
    int ind = 0;
    int starting = 0;
    int ending = listDataIndex.size() - 1;
    int mid = 0;
    int length = 0;
    bool found = false;
    while (!found)
    {
        if (ending < starting)
        {
            ind = -1;
            found = true;
        }

        length = ending - starting;

        if (length == 0)
        {
            ind = ending;
            found = true;
        }

        mid = starting + int(length / 2);

        if (row < listDataIndex.at(mid))
        {
            ending = mid - 1;
        }
        else if (row > listDataIndex.at(mid))
        {
            //if (row < listDataIndex.at(mid + 1))
            if (row < (listDataIndex.at(mid) + listDat1->at(mid)->size + listDat2->at(mid)->size))
            {
                ind = mid;
                found = true;
            }
            else
            {
                starting = mid + 1;
            }
        }
        else
        {
            ind = mid;
            found = true;
        }
    }
    int sum = listDataIndex.at(ind);

    // determine if dat or dat2
    bool bdata1 = true;
    if (row - sum >= listDat1->at(ind)->size)
    {
        sum += listDat1->at(ind)->size;
        bdata1 = false;
    }

    switch (role)
    {
        case Qt::DisplayRole :

            if (bdata1)
            {
                if (row - sum == 0 && column == 0)
                    return (listDat1->at(ind)->getName());
                else if (row - sum == 1 && column == 0)
                    return listDat1->at(ind)->getComment();
                else if (row - sum  == 0 && column == 1)
                    return listDat1->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat1->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat1->at(ind)->getMaxDim();

                if (listDat1->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 2 && column == 1)
                       return listDat1->at(ind)->getZ(0);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                        return listDat1->at(ind)->getX(column - 1);
                    else if (row - sum == 2 && column == 0)
                    {
                        return listDat1->at(ind)->getInputQuantityX();
                    }
                    else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                        return listDat1->at(ind)->getZ(column - 1);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        return listDat1->at(ind)->getX(column - 2);
                    else if (row - sum == 2 && column == 0)
                    {
                        return listDat1->at(ind)->getInputQuantityX();
                    }
                    else if (row - sum == 3 && column == 0)
                    {
                        return listDat1->at(ind)->getInputQuantityY();
                    }
                    else if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                    {
                        if (column == 1)
                        {
                            return listDat1->at(ind)->getY(row - sum - 3);
                        }
                        else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {

                            int dataRow = row - sum - 3;
                            int dataCol = column - 2;
                            return listDat1->at(ind)->getZ(dataRow, dataCol);
                        }
                    }
                }
            }
            else
            {
                if (row - sum  == 0 && column == 1)
                   return listDat2->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat2->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat2->at(ind)->getMaxDim();

                if (listDat2->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 1 && column == 1)
                       return listDat2->at(ind)->getZ(0);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                        return listDat2->at(ind)->getX(column - 1);
                    else if (row - sum == 2 && column == 0)
                    {
                        return listDat2->at(ind)->getInputQuantityX();
                    }
                    else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                        return listDat2->at(ind)->getZ(column - 1);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        return listDat2->at(ind)->getX(column - 2);
                    else if (row - sum == 2 && column == 0)
                    {
                        return listDat2->at(ind)->getInputQuantityX();
                    }
                    else if (row - sum == 3 && column == 0)
                    {
                        return listDat2->at(ind)->getInputQuantityY();
                    }
                    else if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                    {
                        if (column == 1)
                        {
                            return listDat2->at(ind)->getY(row - sum - 2);
                        }
                        else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 2;
                            int dataCol = column - 2;
                            return listDat2->at(ind)->getZ(dataRow, dataCol);
                            //return dat2->at(ind)->getZ((row - sum - 2) + dat2->at(ind)->yCount() * (column - 2));
                        }
                    }
                }
            }

            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
            if (bdata1)
            {
                if (row - sum == 0 && column == 0)
                    return (listDat1->at(ind)->getName());
                else if (row - sum == 1 && column == 0)
                    return listDat1->at(ind)->getComment();
                else if (row - sum  == 0 && column == 1)
                    return listDat1->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat1->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat1->at(ind)->getMaxDim();

                if (listDat1->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 2 && column == 1)
                       return listDat1->at(ind)->getZ(0);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                        return listDat1->at(ind)->getX(column - 1);
                    else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                        return listDat1->at(ind)->getZ(column - 1);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        return listDat1->at(ind)->getX(column - 2);
                    if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                    {
                        if (column == 1)
                        {
                            return listDat1->at(ind)->getY(row - sum - 3);
                        }
                        else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 3;
                            int dataCol = column - 2;
                            return listDat1->at(ind)->getZ(dataRow, dataCol);
                            //return dat1->at(ind)->getZ((row - sum - 3) + dat1->at(ind)->yCount() * (column - 2));
                        }
                    }
                }
            }
            else
            {
                if (row - sum  == 0 && column == 1)
                   return listDat2->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat2->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat2->at(ind)->getMaxDim();

                if (listDat2->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 1 && column == 1)
                       return listDat2->at(ind)->getZ(0);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                        return listDat2->at(ind)->getX(column - 1);
                    else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                        return listDat2->at(ind)->getZ(column - 1);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        return listDat2->at(ind)->getX(column - 2);
                    if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                    {
                        if (column == 1)
                        {
                            return listDat2->at(ind)->getY(row - sum - 2);
                        }
                        else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 2;
                            int dataCol = column - 2;
                            return listDat2->at(ind)->getZ(dataRow, dataCol);
                            //return dat2->at(ind)->getZ((row - sum - 2) + dat2->at(ind)->yCount() * (column - 2));
                        }
                    }
                }
            }

            break;

        case Qt::ToolTipRole:
            if (bdata1)
            {
                if (row - sum == 0 && column == 0)
                    return (listDat1->at(ind)->getName() + " " + listDat1->at(ind)->getUnit());
                else if (row - sum == 1 && column == 0)
                    return listDat1->at(ind)->getComment();
                else if (row - sum  == 0 && column == 1)
                    return listDat1->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat1->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat1->at(ind)->getMaxDim();

                if (listDat1->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 2 && column == 1)
                       return listDat1->at(ind)->getZ(0);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                        return listDat1->at(ind)->getX(column - 1);
                    else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                        return listDat1->at(ind)->getZ(column - 1);
                }
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        return listDat1->at(ind)->getX(column - 2);
                    if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                    {
                        if (column == 1)
                        {
                            return listDat1->at(ind)->getY(row - sum - 3);
                        }
                        else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 3;
                            int dataCol = column - 2;
                            return listDat1->at(ind)->getZ(dataRow, dataCol);
                            //return dat1->at(ind)->getZ((row - sum - 3) + dat1->at(ind)->yCount() * (column - 2));
                        }
                    }
                }
            }
            else
            {
                if (row - sum  == 0 && column == 1)
                   return listDat2->at(ind)->getSubset();
                else if (row - sum == 0 && column == 2)
                    return listDat2->at(ind)->getUnit();
                else if (row - sum == 0 && column == 3)
                    return listDat2->at(ind)->getMaxDim();

                if (listDat2->at(ind)->xCount() == 0) //Value
                {
                    if (row - sum == 1 && column == 1)
                       return listDat2->at(ind)->getZ(0);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                {
                    if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                        return listDat2->at(ind)->getX(column - 1);
                    else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                        return listDat2->at(ind)->getZ(column - 1);
                }
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                {
                    if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        return listDat2->at(ind)->getX(column - 2);
                    if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                    {
                        if (column == 1)
                        {
                            return listDat2->at(ind)->getY(row - sum - 2);
                        }
                        else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 2;
                            int dataCol = column - 2;
                            return listDat2->at(ind)->getZ(dataRow, dataCol);
                            //return dat2->at(ind)->getZ((row - sum - 2) + dat2->at(ind)->yCount() * (column - 2));
                        }
                    }
                }
            }
            break;

        case Qt::StatusTipRole:
            break;

        case Qt::WhatsThisRole:
            break;

        case Qt::SizeHintRole:
            break;

        case Qt::FontRole :
            {
                if (bdata1)
                {
                    QFont font;
                    if (row - sum == 0 && column == 0)
                        font.setBold(true);
                    else if (row - sum == 1 && column == 0)
                        font.setItalic(true);
                    else if (row - sum  == 0 && column == 1)
                        font.setBold(true);

                    if (listDat1->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 2 && column == 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(0).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getZ(0).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                                font.setBold(false);
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount()
                            && column <= listDat2->at(ind)->xCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getX(column - 1).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 1) != listDat2->at(ind)->getX(column - 1))
                                   font.setBold(true);
                            }
                        }
                        else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount()
                            &&  column <= listDat2->at(ind)->zCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getZ(column - 1).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getZ(column - 1) != listDat2->at(ind)->getZ(column - 1))
                                   font.setBold(true);
                            }
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1
                            && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 2).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getX(column - 2).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 2) != listDat2->at(ind)->getX(column - 2))
                                   font.setBold(true);
                            }
                        }
                        if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1
                            && row - sum < listDat2->at(ind)->size - 1)
                        {
                            if (column == 1)
                            {
                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getY(row - sum - 3).toDouble(&bl1);
                                double val2 = listDat2->at(ind)->getY(row - sum - 3).toDouble(&bl2);

                                if (val1 != val2 && bl1 && bl2)
                                    font.setBold(true);
                                else
                                {
                                    if (listDat1->at(ind)->getY(row - sum - 3) != listDat2->at(ind)->getY(row - sum - 3))
                                       font.setBold(true);
                                }

                            }
                            else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1
                                     && column <= listDat2->at(ind)->xCount() + 1)
                            {
                                int dataRow = row - sum - 3;
                                int dataCol = column - 2;

                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getZ(dataRow, dataCol, &bl1);
                                double val2 = listDat2->at(ind)->getZ(dataRow, dataCol, &bl2);

                                if (val1 != val2 && bl1 && bl2)
                                {
                                    font.setBold(true);
                                }
                                else
                                {
                                    if (listDat1->at(ind)->getZ(dataRow, dataCol) != listDat2->at(ind)->getZ(dataRow, dataCol))
                                       font.setBold(true);
                                }
                            }
                        }
                    }
                    return font;
                }
                else
                {
                    QFont font;
                    if (row - sum  == 0 && column == 1)
                        font.setBold(true);

                    if (listDat2->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 1 && column == 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(0).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getZ(0).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                                font.setBold(false);
                        }
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount()
                            && column <= listDat1->at(ind)->xCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getX(column - 1).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 1) != listDat2->at(ind)->getX(column - 1))
                                   font.setBold(true);
                            }
                        }
                        else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount()
                            &&  column <= listDat1->at(ind)->zCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getZ(column - 1).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getZ(column - 1) != listDat2->at(ind)->getZ(column - 1))
                                   font.setBold(true);
                            }
                        }
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1
                            && column <= listDat1->at(ind)->xCount() + 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 2).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getX(column - 2).toDouble(&bl2);

                            if (val1 != val2 && bl1 && bl2)
                                font.setBold(true);
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 2) != listDat2->at(ind)->getX(column - 2))
                                   font.setBold(true);
                            }
                        }
                        if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2
                            && row - sum < listDat1->at(ind)->size - 2)
                        {
                            if (column == 1)
                            {
                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getY(row - sum - 2).toDouble(&bl1);
                                double val2 = listDat2->at(ind)->getY(row - sum - 2).toDouble(&bl2);

                                if (val1 != val2 && bl1 && bl2)
                                    font.setBold(true);
                                else
                                {
                                    if (listDat1->at(ind)->getY(row - sum - 2) != listDat2->at(ind)->getY(row - sum - 2))
                                       font.setBold(true);
                                }

                            }
                            else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1
                                     && column <= listDat1->at(ind)->xCount() + 1)
                            {
                                int dataRow = row - sum - 2;
                                int dataCol = column - 2;

                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getZ(dataRow, dataCol, &bl1);
                                double val2 = listDat2->at(ind)->getZ(dataRow, dataCol, &bl2);

                                if (val1 != val2 && bl1 && bl2)
                                    font.setBold(true);
                                else
                                {
                                    if (listDat1->at(ind)->getZ(dataRow, dataCol) != listDat2->at(ind)->getZ(dataRow, dataCol))
                                       font.setBold(true);
                                }
                            }
                        }
                    }

                    return font;
                }
            }
            break;

        case Qt::TextAlignmentRole:
            break;

        case Qt::BackgroundRole:
            {
                QColor myjaune = QColor::fromHsv(60, 110, 255);
                QColor mybleu = QColor::fromHsv(200, 110, 255);
                QColor myvert = QColor::fromHsv(120, 110, 255);

                if (bdata1)
                {
                    if (row - sum == 0 && column == 3)
                    {
                        if (listDat1->at(ind)->getMaxDim() != "")
                        {
                            return myvert;
                        }
                    }

                    if (listDat1->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 2 && column == 1)
                        {
                           return myjaune;
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                        {
                            return mybleu;
                        }
                        else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                        {
                            return myjaune;
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {
                             return mybleu;
                        }
                        if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                        {
                            if (column == 1)
                            {
                                return mybleu;
                            }
                            else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                            {
                                return myjaune;
                            }
                        }
                    }
                }
                else
                {
                    if (row - sum == 0 && column == 3)
                    {
                        if (listDat2->at(ind)->getMaxDim() != "")
                        {
                            return myvert;
                        }
                    }

                    if (listDat2->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 1 && column == 1)
                           return myjaune;
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                        {
                            return mybleu;
                        }
                        else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                        {
                            return myjaune;
                        }
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                             return mybleu;
                        }
                        if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                        {
                            if (column == 1)
                            {
                                return mybleu;

                            }
                            else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                            {
                                 return myjaune;
                            }
                        }
                    }
                }
            }
            break;

        case Qt::ForegroundRole:
            {
                QColor color1 = Qt::red;
                QColor color2 = Qt::blue;

                if (row - sum == 0 && column == 0)
                    return color2;
                else if (row - sum  == 0 && column == 1)
                    return color1;

                if (bdata1)
                {

                    if (listDat1->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 2 && column == 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(0).toDouble(&bl1);
                            double val2 = listDat1->at(ind)->getOrgZ(0).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat1->at(ind)->getZ(0) != listDat1->at(ind)->getOrgZ(0))
                                    return color1;
                            }
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 1).toDouble(&bl1);
                            double val2 = listDat1->at(ind)->getOrgX(column - 1).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 1) != listDat1->at(ind)->getOrgX(column - 1))
                                    return color1;
                            }
                        }
                        else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                        {

                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getZ(column - 1).toDouble(&bl1);
                            double val2 = listDat1->at(ind)->getOrgZ(column - 1).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat1->at(ind)->getZ(column - 1) != listDat1->at(ind)->getOrgZ(column - 1))
                                    return color1;
                            }
                        }
                    }
                    else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {

                            bool bl1, bl2;
                            double val1 = listDat1->at(ind)->getX(column - 2).toDouble(&bl1);
                            double val2 = listDat1->at(ind)->getOrgX(column - 2).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat1->at(ind)->getX(column - 2) != listDat1->at(ind)->getOrgX(column - 2))
                                    return color1;
                            }
                        }
                        if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                        {
                            if (column == 1)
                            {
                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getY(row - sum - 3).toDouble(&bl1);
                                double val2 = listDat1->at(ind)->getOrgY(row - sum - 3).toDouble(&bl2);

                                if (bl1 && bl2)
                                {
                                    if (val1 > val2)
                                        return color1;
                                    else if (val1 < val2)
                                        return color2;
                                }
                                else
                                {
                                    if (listDat1->at(ind)->getY(row - sum - 3) != listDat1->at(ind)->getOrgY(row - sum - 3))
                                        return color1;
                                }
                            }
                            else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                            {
                                int dataRow = row - sum - 3;
                                int dataCol = column - 2;

                                bool bl1, bl2;
                                double val1 = listDat1->at(ind)->getZ(dataRow, dataCol, &bl1);
                                double val2 = listDat1->at(ind)->getOrgZ(dataRow, dataCol, &bl2);

                                if (bl1 && bl2)
                                {
                                    if (val1 > val2)
                                        return color1;
                                    else if (val1 < val2)
                                        return color2;
                                }
                                else
                                {
                                    if (listDat1->at(ind)->getZ(dataRow, dataCol)!= listDat1->at(ind)->getOrgZ(dataRow, dataCol))
                                        return color1;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (listDat2->at(ind)->xCount() == 0) //Value
                    {
                        if (row - sum == 1 && column == 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat2->at(ind)->getZ(0).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getOrgZ(0).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat2->at(ind)->getZ(0) != listDat2->at(ind)->getOrgZ(0))
                                    return color1;
                            }
                        }
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0) //Curve
                    {
                        if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat2->at(ind)->getX(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getOrgX(column - 1).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat2->at(ind)->getX(column - 1) != listDat2->at(ind)->getOrgX(column - 1))
                                    return color1;
                            }
                        }
                        else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                        {
                            bool bl1, bl2;
                            double val1 = listDat2->at(ind)->getZ(column - 1).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getOrgZ(column - 1).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat2->at(ind)->getZ(column - 1) != listDat2->at(ind)->getOrgZ(column - 1))
                                    return color1;
                            }
                        }
                    }
                    else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0) //Map
                    {
                        if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            bool bl1, bl2;
                            double val1 = listDat2->at(ind)->getX(column - 2).toDouble(&bl1);
                            double val2 = listDat2->at(ind)->getOrgX(column - 2).toDouble(&bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (listDat2->at(ind)->getX(column - 2) != listDat2->at(ind)->getOrgX(column - 2))
                                    return color1;
                            }
                        }
                        if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                        {
                            if (column == 1)
                            {
                                bool bl1, bl2;
                                double val1 = listDat2->at(ind)->getY(row - sum - 2).toDouble(&bl1);
                                double val2 = listDat2->at(ind)->getOrgY(row - sum - 2).toDouble(&bl2);

                                if (bl1 && bl2)
                                {
                                    if (val1 > val2)
                                        return color1;
                                    else if (val1 < val2)
                                        return color2;
                                }
                                else
                                {
                                    if (listDat2->at(ind)->getY(row - sum - 2) != listDat2->at(ind)->getOrgY(row - sum - 2))
                                        return color1;
                                }

                            }
                            else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                            {
                                int dataRow = row - sum - 2;
                                int dataCol = column - 2;

                                bool bl1, bl2;
                                double val1 = listDat2->at(ind)->getZ(dataRow, dataCol, &bl1);
                                double val2 = listDat2->at(ind)->getOrgZ(dataRow, dataCol, &bl2);

                                if (bl1 && bl2)
                                {
                                    if (val1 > val2)
                                        return color1;
                                    else if (val1 < val2)
                                        return color2;
                                }
                                else
                                {
                                    if (listDat2->at(ind)->getZ(dataRow, dataCol)!= listDat2->at(ind)->getOrgZ(dataRow, dataCol))
                                        return color1;
                                }

                            }
                        }
                    }
                }
            }
            break;
    }

    return QVariant();
}

bool CompareModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();
        int column = index.column();

        // binary search
        int ind = 0;
        int starting = 0;
        int ending = listDataIndex.size() - 1;
        int mid = 0;
        int length = 0;
        bool found = false;
        while (!found)
        {
            if (ending < starting)
            {
                ind = -1;
                found = true;
            }

            length = ending - starting;

            if (length == 0)
            {
                ind = ending;
                found = true;
            }

            mid = starting + int(length / 2);

            if (row < listDataIndex.at(mid))
            {
                ending = mid - 1;
            }
            else if (row > listDataIndex.at(mid))
            {
                //if (row < listDataIndex.at(mid + 1))
                if (row < (listDataIndex.at(mid) + listDat1->at(mid)->size + listDat2->at(mid)->size))
                {
                    ind = mid;
                    found = true;
                }
                else
                {
                    starting = mid + 1;
                }
            }
            else
            {
                ind = mid;
                found = true;
            }
        }
        int sum = listDataIndex.at(ind);

        // determine if dat or dat2
        bool bdata1 = true;
        if (row - sum >= listDat1->at(ind)->size)
        {
            sum += listDat1->at(ind)->size;
            bdata1 = false;
        }


        if (bdata1)
        {
            // value
            if (listDat1->at(ind)->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    listDat1->at(ind)->setZ(0, value.toString());
                }
            }
            // curve
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                {
                    if (value.toString() != listDat1->at(ind)->getX(column - 1))
                    {
                        listDat1->at(ind)->setX(column - 1 , value.toString());
                        listDat1->at(ind)->checkValues();
                    }
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                {
                    if (value.toString() != listDat1->at(ind)->getZ(column - 1))
                    {
                        listDat1->at(ind)->setZ(column - 1, value.toString());
                    }
                }
            }
            // map
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0)
            {
                if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                {
                    if (value.toString() != listDat1->at(ind)->getX(column - 2))
                    {
                        listDat1->at(ind)->setX(column - 2, value.toString());
                    }
                }
                if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                {
                    if (column == 1 && value.toString() != listDat1->at(ind)->getY(row - sum - 3))
                    {
                        listDat1->at(ind)->setY(row - sum - 3, value.toString());
                    }
                    else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        if (value.toString() != listDat1->at(ind)->getZ(dataRow, dataCol))
                        {
                            listDat1->at(ind)->setZ(dataRow, dataCol, value.toString());
                        }
                    }
                }
            }
        }
        else
        {
            // value
            if (listDat2->at(ind)->xCount() == 0)
            {
                if ((row - sum == 1) && (column == 1))
                {
                    listDat2->at(ind)->setZ(0, value.toString());
                }
            }
            // curve
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0)
            {
                if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                {
                    if (value.toString() != listDat2->at(ind)->getX(column - 1))
                    {
                        listDat2->at(ind)->setX(column - 1 , value.toString());
                    }
                }
                else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                {
                    if (value.toString() != listDat2->at(ind)->getZ(column - 1))
                    {
                        listDat2->at(ind)->setZ(column - 1, value.toString());
                    }
                }
            }
            // map
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0)
            {
                if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                {
                    if (value.toString() != listDat2->at(ind)->getX(column - 2))
                    {
                        listDat2->at(ind)->setX(column - 2, value.toString());
                    }
                }
                if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                {
                    if (column == 1 && value.toString() != listDat2->at(ind)->getY(row - sum - 2))
                    {
                        listDat2->at(ind)->setY(row - sum - 2, value.toString());
                    }
                    else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 2;
                        int dataCol = column - 2;
                        if (value.toString() != listDat2->at(ind)->getZ(dataRow, dataCol))
                        {
                            listDat2->at(ind)->setZ(dataRow, dataCol, value.toString());
                        }
                    }
                }
            }

        }

        //update the tableView
        emit dataChanged(index, index);

        return true;
    }
    else
    {
        return false;
    }
}

bool CompareModel::setData(const QModelIndex &index, const QModelIndexList &indexList, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();
        int column = index.column();

        // binary search
        int ind = 0;
        int starting = 0;
        int ending = listDataIndex.size() - 1;
        int mid = 0;
        int length = 0;
        bool found = false;
        while (!found)
        {
            if (ending < starting)
            {
                ind = -1;
                found = true;
            }

            length = ending - starting;

            if (length == 0)
            {
                ind = ending;
                found = true;
            }

            mid = starting + int(length / 2);

            if (row < listDataIndex.at(mid))
            {
                ending = mid - 1;
            }
            else if (row > listDataIndex.at(mid))
            {
                //if (row < listDataIndex.at(mid + 1))
                if (row < (listDataIndex.at(mid) + listDat1->at(mid)->size + listDat2->at(mid)->size))
                {
                    ind = mid;
                    found = true;
                }
                else
                {
                    starting = mid + 1;
                }
            }
            else
            {
                ind = mid;
                found = true;
            }
        }
        int sum = listDataIndex.at(ind);

        // determine if dat or dat2
        bool bdata1 = true;
        if (row - sum >= listDat1->at(ind)->size)
        {
            sum += listDat1->at(ind)->size;
            bdata1 = false;
        }


        if (bdata1)
        {
            // value
            if (listDat1->at(ind)->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    listDat1->at(ind)->setZ(0, value.toString());
                }
            }
            // curve
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                {
                    if (value.toString() != listDat1->at(ind)->getX(column - 1))
                    {
                        listDat1->at(ind)->setX(column - 1 , value.toString());
                    }
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                {
                    if (value.toString() != listDat1->at(ind)->getZ(column - 1))
                    {
                        listDat1->at(ind)->setZ(column - 1, value.toString());
                    }
                }
            }
            // map
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0)
            {
                if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                {
                    if (value.toString() != listDat1->at(ind)->getX(column - 2))
                    {
                        listDat1->at(ind)->setX(column - 2, value.toString());
                    }
                }
                if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                {
                    if (column == 1 && value.toString() != listDat1->at(ind)->getY(row - sum - 3))
                    {
                        listDat1->at(ind)->setY(row - sum - 3, value.toString());
                    }
                    else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        if (value.toString() != listDat1->at(ind)->getZ(dataRow, dataCol))
                        {
                            listDat1->at(ind)->setZ(dataRow, dataCol, value.toString());
                        }
                    }
                }
            }
        }
        else
        {
            // value
            if (listDat2->at(ind)->xCount() == 0)
            {
                if ((row - sum == 1) && (column == 1))
                {
                    listDat2->at(ind)->setZ(0, value.toString());
                }
            }
            // curve
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0)
            {
                if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                {
                    if (value.toString() != listDat2->at(ind)->getX(column - 1))
                    {
                        listDat2->at(ind)->setX(column - 1 , value.toString());
                    }
                }
                else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                {
                    if (value.toString() != listDat2->at(ind)->getZ(column - 1))
                    {
                        listDat2->at(ind)->setZ(column - 1, value.toString());
                    }
                }
            }
            // map
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0)
            {
                if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                {
                    if (value.toString() != listDat2->at(ind)->getX(column - 2))
                    {
                        listDat2->at(ind)->setX(column - 2, value.toString());
                    }
                }
                if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                {
                    if (column == 1 && value.toString() != listDat2->at(ind)->getY(row - sum - 2))
                    {
                        listDat2->at(ind)->setY(row - sum - 2, value.toString());
                    }
                    else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 2;
                        int dataCol = column - 2;
                        if (value.toString() != listDat2->at(ind)->getZ(dataRow, dataCol))
                        {
                            listDat2->at(ind)->setZ(dataRow, dataCol, value.toString());
                        }
                    }
                }
            }

        }

        //update the tableView
        QModelIndex topLeft = indexList.at(0);
        QModelIndex btmRight = indexList.at(indexList.count() - 1);
        if (index == btmRight)
        {
            //emit dataChanged(topLeft, btmRight);
            //create trick variable to evidence change when ctrl+v in formCompare (see point 265 into list)
            QModelIndex trick = createIndex(btmRight.row(), btmRight.column() + 1);
            emit dataChanged(topLeft, trick);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool CompareModel::resetData(const QModelIndex &index, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();
        int column = index.column();

        // binary search
        int ind = 0;
        int starting = 0;
        int ending = listDataIndex.size() - 1;
        int mid = 0;
        int length = 0;
        bool found = false;
        while (!found)
        {
            if (ending < starting)
            {
                ind = -1;
                found = true;
            }

            length = ending - starting;

            if (length == 0)
            {
                ind = ending;
                found = true;
            }

            mid = starting + int(length / 2);

            if (row < listDataIndex.at(mid))
            {
                ending = mid - 1;
            }
            else if (row > listDataIndex.at(mid))
            {
                //if (row < listDataIndex.at(mid + 1))
                if (row < (listDataIndex.at(mid) + listDat1->at(mid)->size + listDat2->at(mid)->size))
                {
                    ind = mid;
                    found = true;
                }
                else
                {
                    starting = mid + 1;
                }
            }
            else
            {
                ind = mid;
                found = true;
            }
        }
        int sum = listDataIndex.at(ind);

        // determine if dat or dat2
        bool bdata1 = true;
        if (row - sum >= listDat1->at(ind)->size)
        {
            sum += listDat1->at(ind)->size;
            bdata1 = false;
        }


        if (bdata1)
        {
            // value
            if (listDat1->at(ind)->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    listDat1->at(ind)->resetValZ(0);
                }
            }
            // curve
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                {
                    listDat1->at(ind)->resetValX(column - 1);
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                {
                    listDat1->at(ind)->resetValZ(column - 1);
                }
            }
            // map
            else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0)
            {
                if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                {
                    listDat1->at(ind)->resetValX(column - 2);
                }
                if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        listDat1->at(ind)->resetValY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        listDat1->at(ind)->resetValZ(dataRow, dataCol);
                    }
                }
            }
        }
        else
        {
            // value
            if (listDat2->at(ind)->xCount() == 0)
            {
                listDat2->at(ind)->resetValZ(0);;
            }
            // curve
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0)
            {
                if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                {
                    listDat2->at(ind)->resetValX(column - 1);
                }
                else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                {
                    listDat2->at(ind)->resetValZ(column - 1);
                }
            }
            // map
            else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0)
            {
                if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                {
                    listDat2->at(ind)->resetValX(column - 2);
                }
                if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                {
                    if (column == 1)
                    {
                        listDat2->at(ind)->resetValY(row - sum - 2);
                    }
                    else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 2;
                        int dataCol = column - 2;
                        listDat2->at(ind)->resetValZ(dataRow, dataCol);
                    }
                }
            }

        }

        //update the tableView
        emit dataChanged(index, index);

        return true;
    }
    else
    {
        return false;
    }
}

bool CompareModel::resetData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int ind = 0;
            int sum = 0;

            while (row >= sum)
            {
                sum += listDat1->at(ind)->size + listDat2->at(ind)->size;
                ind++;
            }
            ind--;
            sum -= listDat1->at(ind)->size + listDat2->at(ind)->size;

            // determine if dat or dat2
            bool bdata1 = true;
            if (row - sum >= listDat1->at(ind)->size)
            {
                sum += listDat1->at(ind)->size;
                bdata1 = false;
            }

            if (bdata1)
            {
                // value
                if (listDat1->at(ind)->xCount() == 0)
                {
                    if ((row - sum == 2) && (column == 1))
                    {
                        listDat1->at(ind)->resetValZ(0);
                    }
                }
                // curve
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0)
                {
                    if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                    {
                        listDat1->at(ind)->resetValX(column - 1);
                    }
                    else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                    {
                        listDat1->at(ind)->resetValZ(column - 1);
                    }
                }
                // map
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0)
                {
                    if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                    {
                        listDat1->at(ind)->resetValX(column - 2);
                    }
                    if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                    {
                        if (column == 1)
                        {
                            listDat1->at(ind)->resetValY(row - sum - 3);
                        }
                        else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 3;
                            int dataCol = column - 2;
                            listDat1->at(ind)->resetValZ(dataRow, dataCol);
                        }
                    }
                }
            }
            else
            {
                // value
                if (listDat2->at(ind)->xCount() == 0)
                {
                    listDat2->at(ind)->resetValZ(0);
                }
                // curve
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0)
                {
                    if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                    {
                        listDat2->at(ind)->resetValX(column - 1);
                    }
                    else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                    {
                        listDat2->at(ind)->resetValZ(column - 1);
                    }
                }
                // map
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0)
                {
                    if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                    {
                        listDat2->at(ind)->resetValX(column - 2);
                    }
                    if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                    {
                        if (column == 1)
                        {
                            listDat2->at(ind)->resetValY(row - sum - 2);
                        }
                        else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 2;
                            int dataCol = column - 2;
                            listDat2->at(ind)->resetValZ(dataRow, dataCol);
                        }
                    }
                }

            }
        }
    }

    //update the tableView
    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    emit dataChanged(topLeft, btmRight);

    return true;

}

bool  CompareModel::undoData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int ind = 0;
            int sum = 0;

            while (row >= sum)
            {
                sum += listDat1->at(ind)->size + listDat2->at(ind)->size;
                ind++;
            }
            ind--;
            sum -= listDat1->at(ind)->size + listDat2->at(ind)->size;

            // determine if dat or dat2
            bool bdata1 = true;
            if (row - sum >= listDat1->at(ind)->size)
            {
                sum += listDat1->at(ind)->size;
                bdata1 = false;
            }

            if (bdata1)
            {
                // value
                if (listDat1->at(ind)->xCount() == 0)
                {
                    if ((row - sum == 2) && (column == 1))
                    {
                        listDat1->at(ind)->undoValZ(0);
                    }
                }
                // curve
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() == 0)
                {
                    if (row - sum == 2 && column >= 1 && column <= listDat1->at(ind)->xCount())
                    {
                        listDat1->at(ind)->undoValX(column - 1);
                    }
                    else if (row - sum == 3 && column >= 1 &&  column <= listDat1->at(ind)->zCount())
                    {
                        listDat1->at(ind)->undoValZ(column - 1);
                    }
                }
                // map
                else if (listDat1->at(ind)->xCount() > 0 && listDat1->at(ind)->yCount() > 0)
                {
                    if (row - sum == 2 && column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                    {
                        listDat1->at(ind)->undoValX(column - 2);
                    }
                    if (row - sum >= 3 && row - sum < listDat1->at(ind)->size - 1)
                    {
                        if (column == 1)
                        {
                            listDat1->at(ind)->undoValY(row - sum - 3);
                        }
                        else if (column >= 2 && column <= listDat1->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 3;
                            int dataCol = column - 2;
                            listDat1->at(ind)->undoValZ(dataRow, dataCol);
                        }
                    }
                }
            }
            else
            {
                // value
                if (listDat2->at(ind)->xCount() == 0)
                {
                    listDat2->at(ind)->undoValZ(0);
                }
                // curve
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() == 0)
                {
                    if (row - sum == 1 && column >= 1 && column <= listDat2->at(ind)->xCount())
                    {
                        listDat2->at(ind)->undoValX(column - 1);
                    }
                    else if (row - sum == 2 && column >= 1 &&  column <= listDat2->at(ind)->zCount())
                    {
                        listDat2->at(ind)->undoValZ(column - 1);
                    }
                }
                // map
                else if (listDat2->at(ind)->xCount() > 0 && listDat2->at(ind)->yCount() > 0)
                {
                    if (row - sum == 1 && column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                    {
                        listDat2->at(ind)->undoValX(column - 2);
                    }
                    if (row - sum >= 2 && row - sum < listDat2->at(ind)->size - 2)
                    {
                        if (column == 1)
                        {
                            listDat2->at(ind)->undoValY(row - sum - 2);
                        }
                        else if (column >= 2 && column <= listDat2->at(ind)->xCount() + 1)
                        {
                            int dataRow = row - sum - 2;
                            int dataCol = column - 2;
                            listDat2->at(ind)->undoValZ(dataRow, dataCol);
                        }
                    }
                }

            }
        }
    }

    //update the tableView
    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    emit dataChanged(topLeft, btmRight);

    return true;
}

Qt::ItemFlags CompareModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant CompareModel::headerData(int section, Qt::Orientation /* orientation */, int role) const
{

    if (role != Qt::DisplayRole)
        return QVariant();

    return QString::number(section);
}

void CompareModel::setList(QList<Data *> *labelList, QList<Data *> *labelList2)
{
    //set data
    listDat1 = labelList;
    listDat2 = labelList2;

    //calculate the listDataIndex
    listDataIndex.clear();
    listDataIndex.append(0);
    for (int i = 1; i < listDat1->count(); i++)
    {
        listDataIndex.append(listDataIndex.at(i - 1) + listDat1->at(i - 1)->size
                             + listDat2->at(i - 1)->size);
    }

    //calculate nRow / nColumn
    nRow = 0;
    nColumn = 0;
    for (int i = 0; i < listDat1->count(); i++)
    {
        nRow += listDat1->at(i)->size;
        nRow += listDat2->at(i)->size;
    }

    for (int i = 0; i < listDat1->count(); i++)
    {
        if (listDat1->at(i)->xCount() + 2 > nColumn)
            nColumn = listDat1->at(i)->xCount() + 2;
        if (listDat2->at(i)->xCount() + 2 > nColumn)
            nColumn = listDat2->at(i)->xCount() + 2;
    }

    //refresh model
    reset();
}

Data *CompareModel::getLabel(const QModelIndex &index, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();

        // binary search
        int ind = 0;
        int starting = 0;
        int ending = listDataIndex.size() - 1;
        int mid = 0;
        int length = 0;
        bool found = false;
        while (!found)
        {
            if (ending < starting)
            {
                ind = -1;
                found = true;
            }

            length = ending - starting;

            if (length == 0)
            {
                ind = ending;
                found = true;
            }

            mid = starting + int(length / 2);

            if (row < listDataIndex.at(mid))
            {
                ending = mid - 1;
            }
            else if (row > listDataIndex.at(mid))
            {
                //if (row < listDataIndex.at(mid + 1))
                if (row < (listDataIndex.at(mid) + listDat1->at(mid)->size + listDat2->at(mid)->size))
                {
                    ind = mid;
                    found = true;
                }
                else
                {
                    starting = mid + 1;
                }
            }
            else
            {
                ind = mid;
                found = true;
            }
        }
        int sum = listDataIndex.at(ind);

        // determine if dat or dat2
        bool bdata1 = true;
        if (row - sum >= listDat1->at(ind)->size)
        {
            sum += listDat1->at(ind)->size;
            bdata1 = false;
        }

        if (bdata1)
            return listDat1->at(ind);
        else
            return listDat2->at(ind);
    }
    else
    {
        return 0;
    }
}

QList<Data*> *CompareModel::getList1()
{
    return listDat1;
}

QList<Data*> *CompareModel::getList2()
{
    return listDat2;
}

QModelIndex CompareModel::getFirstZindex(Data *data)
{
    // determine the label in function of the row
    int ind = 0;
    int sum = 0;

    while (listDat1->at(ind) != data && listDat2->at(ind) != data)
    {
        sum += listDat1->at(ind)->size + listDat2->at(ind)->size;
        ind++;
    }

//    if (ind > 0)
//    {
//        ind--;
//        sum -= dat1->at(ind)->size + dat2->at(ind)->size;
//    }


    // determine if dat1 or dat2
    if (listDat2->at(ind) == data)
    {
        sum += listDat1->at(ind)->size - 1;
    }

    if (data->xCount() == 0)
    {
        return createIndex(sum + 2, 1);
    }
    else if (data->yCount() == 0)
    {
        return createIndex(sum + 3, 1);
    }
    else
    {
        return createIndex(sum + 3, 2);
    }

}

QModelIndexList CompareModel::selectAllX(Data *data)
{
    QModelIndexList listIndex;
    listIndex.clear();

    if (data->xCount() == 0)
        return listIndex;

    QModelIndex index = getFirstZindex(data);

    for (int i = 0; i < data->xCount(); i++)
    {
        QModelIndex ind = createIndex(index.row() - 1, index.column() + i);
        listIndex.append(ind);

    }
    return listIndex;
}

QModelIndexList CompareModel::selectAllY(Data *data)
{
    QModelIndexList listIndex;
    listIndex.clear();

    if (data->yCount() == 0)
        return listIndex;

    QModelIndex index = getFirstZindex(data);

    for (int i = 0; i < data->yCount(); i++)
    {
        QModelIndex ind = createIndex(index.row() + i, index.column() - 1);
        listIndex.append(ind);

    }
    return listIndex;
}

QModelIndexList CompareModel::selectAllZ(Data *data)
{
    QModelIndexList listIndex;
    QModelIndex index = getFirstZindex(data);

    int line = 0;
    if (data->yCount() == 0)
        line = 1;
    else
        line = data->yCount();

    for (int i = 0; i < line; i++)
    {
        for (int j = 0; j < data->xCount(); j++)
        {
            QModelIndex ind = createIndex(index.row() + i, index.column() + j);
            listIndex.append(ind);
        }
    }
    return listIndex;
}
