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

#include "sptablemodel.h"

#include <QFont>
#include <QSettings>
#include <qapplication.h>
#include "qdebug.h"

SpTableModel::SpTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    listDataModel = NULL;
    nRow = 0;
    nColumn = 0;
}

SpTableModel::~SpTableModel()
{    
   /* if (dat)
    {
        foreach (Data* data, *dat)
        {
            delete data;
        }
    }
   */

    delete listDataModel;

}

int SpTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return nRow;
}

int SpTableModel::columnCount(const QModelIndex & /* parent */) const
{    
    return nColumn;
}

QVariant SpTableModel::data(const QModelIndex &index, int role) const
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
            if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

    switch (role)
    {
    case Qt::DisplayRole :
        {
            if (row - sum == 0 && column == 0)
                return (listDataModel->at(ind)->getName());
            else if (row - sum == 1 && column == 0)
                return listDataModel->at(ind)->getComment();
            else if (row - sum  == 0 && column == 1)
                return listDataModel->at(ind)->getSubset();
            else if (row - sum == 0 && column == 3)
                return listDataModel->at(ind)->getMaxDim();
            else if (row - sum == 0 && column == 2)
                  return listDataModel->at(ind)->getUnit();

            if (listDataModel->at(ind)->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 1)
                   return listDataModel->at(ind)->getZ(0);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                    return listDataModel->at(ind)->getX(column - 1);                
                else if (row - sum == 2 && column == 0)
                {
                    return listDataModel->at(ind)->getInputQuantityX();
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                    return listDataModel->at(ind)->getZ(column - 1);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                //qDebug()<< "name : " << listDataModel->at(ind)->getName() << " and size : " <<listDataModel->at(ind)->size << " and yCount() : " << listDataModel->at(ind)->yCount();
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    return listDataModel->at(ind)->getX(column - 2);
                else if (row - sum == 2 && column == 0)
                {
                    return listDataModel->at(ind)->getInputQuantityX();
                }
                else if (row - sum == 3 && column == 0)
                {
                    return listDataModel->at(ind)->getInputQuantityY();
                }
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {

                        //qDebug()<< "i : " << row - sum - 3 << " and max : " << listDataModel->at(ind)->yCount();
                        return listDataModel->at(ind)->getY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        return listDataModel->at(ind)->getZ(dataRow, dataCol);
                        //return dat->at(ind)->getZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2));
                    }
                }
            }
        }
        break;

    case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
        break;

    case Qt::EditRole:
        {
            if (row - sum == 0 && column == 0)
                return (listDataModel->at(ind)->getName());
            else if (row - sum == 1 && column == 0)
                return listDataModel->at(ind)->getComment();
            else if (row - sum  == 0 && column == 1)
                return listDataModel->at(ind)->getSubset();
            else if (row - sum == 0 && column == 2)
                  return listDataModel->at(ind)->getUnit();
            else if (row - sum == 0 && column == 3)
                return listDataModel->at(ind)->getMaxDim();

            if (listDataModel->at(ind)->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 1)
                   return listDataModel->at(ind)->getZ(0);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                    return listDataModel->at(ind)->getX(column - 1);
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                    return listDataModel->at(ind)->getZ(column - 1);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    return listDataModel->at(ind)->getX(column - 2);
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        return listDataModel->at(ind)->getY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        return listDataModel->at(ind)->getZ(dataRow, dataCol);
                        //return dat->at(ind)->getZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2));
                    }
                }
            }
        }
        break;

    case Qt::ToolTipRole:
        {
            if (row - sum == 0 && column == 0)
                return (listDataModel->at(ind)->getName() + " " + listDataModel->at(ind)->getUnit());
            else if (row - sum == 1 && column == 0)
                return listDataModel->at(ind)->getComment();
            else if (row - sum  == 0 && column == 1)
                return listDataModel->at(ind)->getSubset();
            else if (row - sum == 0 && column == 2)
                  return listDataModel->at(ind)->getUnit();
            else if (row - sum == 0 && column == 3)
                return listDataModel->at(ind)->getMaxDim();

            if (listDataModel->at(ind)->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 1)
                   return listDataModel->at(ind)->getZ(0);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                    return listDataModel->at(ind)->getX(column - 1);
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                    return listDataModel->at(ind)->getZ(column - 1);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    return listDataModel->at(ind)->getX(column - 2);
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        return listDataModel->at(ind)->getY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        return listDataModel->at(ind)->getZ(dataRow, dataCol);
                        //return dat->at(ind)->getZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2));
                    }
                }
            }
        }
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
            if (row - sum == 0 && column == 0)
                font.setBold(true);
            else if (row - sum == 1 && column == 0)
                font.setItalic(true);
            else if (row - sum  == 0 && column == 1)
                font.setBold(true);

            if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                     font.setBold(false);
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                     font.setBold(false);
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                         font.setBold(false);
                    }
                }
            }

            return font;
        }
        break;

    case Qt::TextAlignmentRole:
        break;

    case Qt::BackgroundRole:
        {
            QColor myjaune = QColor::fromHsv(60, 110, 255);
            QColor mybleu = QColor::fromHsv(200, 110, 255);
            QColor myvert = QColor::fromHsv(120, 110, 255);

            if (row - sum == 0 && column == 3)
            {
                if (listDataModel->at(ind)->getMaxDim() != "")
                {
                    return myvert;
                }
            }

            if (listDataModel->at(ind)->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 1)
                {
                    return myjaune;
                }
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                {
                    return mybleu;
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                {
                    return myjaune;
                }
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    return mybleu;
                }
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        return mybleu;
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    {
                        return myjaune;
                    }
                }
            }
        }
        break;

    case Qt::ForegroundRole:
        {
            QColor color1 = Qt::red;
            QColor color2 = Qt::blue;
            QColor color3 = Qt::darkGreen;
            QColor black = Qt::black;

            if (row - sum == 0 && column == 0)
            {
                QSettings settings(qApp->organizationName(), qApp->applicationName());
                if (settings.value("theme") != "dark")
                {
                    return color2;
                }
                else
                    return color3;
            }
            else if (row - sum  == 0 && column == 1)
                return color1;

            if (listDataModel->at(ind)->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 1)
                {
                    bool bl1;
                    double val1 = listDataModel->at(ind)->getZ(0).toDouble(&bl1);
                    bool bl2;
                    double val2 = listDataModel->at(ind)->getOrgZ(0).toDouble(&bl2);

                    if (bl1 && bl2)
                    {
                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                        else return black;
                    }
                    else
                    {
                        if (listDataModel->at(ind)->getZ(0) != listDataModel->at(ind)->getOrgZ(0))
                            return color1;
                        else return black;
                    }
                }
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                {
                    bool bl1;
                    double val1 = listDataModel->at(ind)->getX(column - 1).toDouble(&bl1);
                    bool bl2;
                    double val2 = listDataModel->at(ind)->getOrgX(column - 1).toDouble(&bl2);

                    if (bl1 && bl2)
                    {
                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                        else return black;
                    }
                    else
                    {
                        if (listDataModel->at(ind)->getX(column - 1) != listDataModel->at(ind)->getOrgX(column - 1))
                            return color1;
                        else return black;
                    }
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                {

                    bool bl1;
                    double val1 = listDataModel->at(ind)->getZ(column - 1).toDouble(&bl1);
                    bool bl2;
                    double val2 = listDataModel->at(ind)->getOrgZ(column - 1).toDouble(&bl2);

                    if (bl1 && bl2)
                    {
                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                        else return black;
                    }
                    else
                    {
                        if (listDataModel->at(ind)->getZ(column - 1) != listDataModel->at(ind)->getOrgZ(column - 1))
                            return color1;
                        else return black;
                    }
                }
            }
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    bool bl1, bl2;
                    double val1 = listDataModel->at(ind)->getX(column - 2).toDouble(&bl1);
                    double val2 = listDataModel->at(ind)->getOrgX(column - 2).toDouble(&bl2);

                    if (bl1 && bl2)
                    {
                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                        else return black;
                    }
                    else
                    {
                        if (listDataModel->at(ind)->getX(column - 2) != listDataModel->at(ind)->getOrgX(column - 2))
                            return color1;
                        else return black;
                    }
                }
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        bool bl1, bl2;
                        double val1 = listDataModel->at(ind)->getY(row - sum - 3).toDouble(&bl1);
                        double val2 = listDataModel->at(ind)->getOrgY(row - sum - 3).toDouble(&bl2);

                        if (bl1 && bl2)
                        {
                            if (val1 > val2)
                                return color1;
                            else if (val1 < val2)
                                return color2;
                            else return black;
                        }
                        else
                        {
                            if (listDataModel->at(ind)->getY(row - sum - 3) != listDataModel->at(ind)->getOrgY(row - sum - 3))
                                return color1;
                            else return black;
                        }
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;

                        bool bl1, bl2;
                        double val1 = listDataModel->at(ind)->getZ(dataRow, dataCol, &bl1);
                        double val2 = listDataModel->at(ind)->getOrgZ(dataRow, dataCol, &bl2);

                        if (bl1 && bl2)
                        {
                            if (val1 > val2)
                                return color1;
                            else if (val1 < val2)
                                return color2;
                            else return black;
                        }
                        else
                        {
                            if (listDataModel->at(ind)->getZ(dataRow, dataCol)!= listDataModel->at(ind)->getOrgZ(dataRow, dataCol))
                                return color1;
                            else return black;
                        }
                    }
                }
            }
        }
        break;
    }

    return QVariant();
}

bool SpTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
                if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

        // value
        if (listDataModel->at(ind)->xCount() == 0)
        {
            if ((row - sum == 2) && (column == 1))
            {
                if (value.toString() != listDataModel->at(ind)->getZ(0))
                {
                    listDataModel->at(ind)->setZ(0, value.toString());
                }
            }
        }
        // curve
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0)
        {
            if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
            {
                if (value.toString() != listDataModel->at(ind)->getX(column - 1))
                {
                    listDataModel->at(ind)->setX(column - 1 , value.toString());
                }
            }
            else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
            {
                if (value.toString() != listDataModel->at(ind)->getZ(column - 1))
                {
                    listDataModel->at(ind)->setZ(column - 1, value.toString());
                }
            }
        }
        // map
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0)
        {

            if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
            {
                if (value.toString() != listDataModel->at(ind)->getX(column - 2))
                {
                    listDataModel->at(ind)->setX(column - 2, value.toString());
                }
            }
            if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
            {
                if (column == 1 && value.toString() != listDataModel->at(ind)->getY(row - sum - 3))
                {
                    listDataModel->at(ind)->setY(row - sum - 3, value.toString());
                }
                else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    int dataRow = row - sum - 3;
                    int dataCol = column - 2;
                    if (value.toString() != listDataModel->at(ind)->getZ(dataRow, dataCol))
                    {
                        listDataModel->at(ind)->setZ(dataRow, dataCol, value.toString());
                        //dat->at(ind)->setZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2), value.toString());
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

bool SpTableModel::setData(const QModelIndex &index, const QModelIndexList &indexList ,const QVariant &value, int role)
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
                if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

        // value
        if (listDataModel->at(ind)->xCount() == 0)
        {
            if ((row - sum == 2) && (column == 1))
            {
                if (value.toString() != listDataModel->at(ind)->getZ(0))
                {
                    listDataModel->at(ind)->setZ(0, value.toString());
                }
            }
        }
        // curve
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0)
        {
            if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
            {
                if (value.toString() != listDataModel->at(ind)->getX(column - 1))
                {
                    listDataModel->at(ind)->setX(column - 1 , value.toString());
                }
            }
            else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
            {
                if (value.toString() != listDataModel->at(ind)->getZ(column - 1))
                {
                    listDataModel->at(ind)->setZ(column - 1, value.toString());
                }
            }
        }
        // map
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0)
        {
            if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
            {
                if (value.toString() != listDataModel->at(ind)->getX(column - 2))
                {
                    listDataModel->at(ind)->setX(column - 2, value.toString());
                }
            }
            if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
            {
                if (column == 1 && value.toString() != listDataModel->at(ind)->getY(row - sum - 3))
                {
                    listDataModel->at(ind)->setY(row - sum - 3, value.toString());
                }
                else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    int dataRow = row - sum - 3;
                    int dataCol = column - 2;
                    if (value.toString() != listDataModel->at(ind)->getZ(dataRow, dataCol))
                    {
                        listDataModel->at(ind)->setZ(dataRow, dataCol, value.toString());
                        //dat->at(ind)->setZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2), value.toString());
                    }
                }
            }
        }

        //update the tableView
        QModelIndex topLeft = indexList.at(0);
        QModelIndex btmRight = indexList.at(indexList.count() - 1);
        if (index == btmRight)
            emit dataChanged(topLeft, btmRight);

        return true;
    }
    else
    {
        return false;
    }
}

bool SpTableModel::resetData(const QModelIndex &index, int role)
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
                if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

        // value
        if (listDataModel->at(ind)->xCount() == 0)
        {
            if ((row - sum == 2) && (column == 1))
            {
                listDataModel->at(ind)->resetValZ(0);
            }
        }
        // curve
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0)
        {
            if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
            {
                listDataModel->at(ind)->resetValX(column - 1);
            }
            else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
            {
                listDataModel->at(ind)->resetValZ(column - 1);
            }
        }
        // map
        else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0)
        {
            if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
            {
                listDataModel->at(ind)->resetValX(column - 2);
            }
            if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
            {
                if (column == 1)
                {
                    listDataModel->at(ind)->resetValY(row - sum - 3);
                }
                else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1 )
                {
                    int dataRow = row - sum - 3;
                    int dataCol = column - 2;
                    listDataModel->at(ind)->resetValZ(dataRow, dataCol);
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

bool SpTableModel::resetData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
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
                    if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

            // value
            if (listDataModel->at(ind)->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    listDataModel->at(ind)->resetValZ(0);
                }
            }
            // curve
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                {
                    listDataModel->at(ind)->resetValX(column - 1);
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                {
                    listDataModel->at(ind)->resetValZ(column - 1);
                }
            }
            // map
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0)
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    listDataModel->at(ind)->resetValX(column - 2);
                }
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        listDataModel->at(ind)->resetValY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1 )
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        listDataModel->at(ind)->resetValZ(dataRow, dataCol);
                        //dat->at(ind)->resetValZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2));
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

bool SpTableModel::undoData(const QModelIndexList &indexList, int role)
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
                sum += listDataModel->at(ind)->size;
                ind++;
            }
            ind--;
            sum -= listDataModel->at(ind)->size;

            // value
            if (listDataModel->at(ind)->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    listDataModel->at(ind)->undoValZ(0);
                }
            }
            // curve
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= listDataModel->at(ind)->xCount())
                {
                    listDataModel->at(ind)->undoValX(column - 1);
                }
                else if (row - sum == 3 && column >= 1 &&  column <= listDataModel->at(ind)->zCount())
                {
                    listDataModel->at(ind)->undoValZ(column - 1);
                }
            }
            // map
            else if (listDataModel->at(ind)->xCount() > 0 && listDataModel->at(ind)->yCount() > 0)
            {
                if (row - sum == 2 && column >= 2 && column <= listDataModel->at(ind)->xCount() + 1)
                {
                    listDataModel->at(ind)->undoValX(column - 2);
                }
                if (row - sum >= 3 && row - sum < listDataModel->at(ind)->size - 1)
                {
                    if (column == 1)
                    {
                        listDataModel->at(ind)->undoValY(row - sum - 3);
                    }
                    else if (column >= 2 && column <= listDataModel->at(ind)->xCount() + 1 )
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 2;
                        listDataModel->at(ind)->undoValZ(dataRow, dataCol);
                        //dat->at(ind)->resetValZ((row - sum - 3) + dat->at(ind)->yCount() * (column - 2));
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

Qt::ItemFlags SpTableModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant SpTableModel::headerData(int section, Qt::Orientation /* orientation */, int role) const
{

    if (role != Qt::DisplayRole)
        return QVariant();

    return QString::number(section);
}

void SpTableModel::setList(QList<Data *> *labelList)
{
    //set the list
    listDataModel = labelList;

    //calculate the listDataIndex
    listDataIndex.clear();
    listDataIndex.append(0);
    for (int i = 1; i < listDataModel->count(); i++)
    {
        listDataIndex.append(listDataIndex.at(i - 1) + listDataModel->at(i - 1)->size);
    }

    //calculate nRow;
    nRow = 0;
    for (int i = 0; i < listDataModel->count(); i++)
        nRow += listDataModel->at(i)->size;

    //calculate nCol
    nColumn = 3;
    for (int i = 0; i < listDataModel->count(); i++)
        if (listDataModel->at(i)->xCount() + 2 > nColumn)
            nColumn = listDataModel->at(i)->xCount() + 2;

    beginResetModel();
    endResetModel();
}

QList<Data*> *SpTableModel::getList()
{
    return listDataModel;
}

Data *SpTableModel::getLabel(const QModelIndex &index, int role)
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
                if (row < (listDataIndex.at(mid) + listDataModel->at(mid)->size))
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

        if (found)
            return listDataModel->at(ind);
        else
            return 0;

    }
    else
    {
        return 0;
    }
}

QModelIndex SpTableModel::getIndex(int row, int col)
{
   return createIndex(row, col);
}

QModelIndex SpTableModel::getFirstZindex(Data *data)
{
    int i = 0;
    int row = 0;
    while (listDataModel->at(i) != data)
    {
        row += listDataModel->at(i)->size;
        i++;
    }

    if (data->xCount() == 0)
    {
        return createIndex(row + 2, 1);
    }
    else if (data->yCount() == 0)
    {
        return createIndex(row + 3, 1);
    }
    else
    {
        return createIndex(row + 3, 2);
    }
}

QModelIndexList SpTableModel::selectAllX(Data *data)
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

QModelIndexList SpTableModel::selectAllY(Data *data)
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

QModelIndexList SpTableModel::selectAllZ(Data *data)
{
    QModelIndexList listIndex;
    QModelIndex index = getFirstZindex(data);

    if (data->xCount() == 0)
    {
        QModelIndex ind = createIndex(index.row(), index.column());
        listIndex.append(ind);
        return listIndex;
    }

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
