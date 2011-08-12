#include <QObject>
#include <QFont>
#include "graphmodel.h"

GraphModel::GraphModel(QObject *parent) : QAbstractTableModel(parent)
{
    dat = NULL;
    nRow = 0;
    nColumn = 0;
}

GraphModel::~GraphModel()
{    
    delete dat;
}

int GraphModel::rowCount(const QModelIndex & /* parent */) const
{
    return nRow;
}

int GraphModel::columnCount(const QModelIndex & /* parent */) const
{    
    return nColumn;
}

QVariant GraphModel::data(const QModelIndex &index, int role) const
{    
    if (!index.isValid()) return QVariant();

        int row = index.row();
        int column = index.column();
        int sum = -2;

        switch (role)
        {
        case Qt::DisplayRole :

            if (dat->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 0)
                   return dat->getZ(0);
            }
            else if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
            {            
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                    return dat->getX(column);
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                    return dat->getZ(column);
            }
            else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                    return dat->getX(column - 1);
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0)
                    {
                        return dat->getY(row - sum - 3);
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        return dat->getZ(dataRow, dataCol);
                    }
                }
            }

            break;

        case Qt::DecorationRole: // The dat to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:

            if (dat->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 0)
                   return dat->getZ(0);
            }
            else if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                    return dat->getX(column);
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                    return dat->getZ(column);
            }
            else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                    return dat->getX(column - 1);
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0)
                    {
                        return dat->getY(row - sum - 3);
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        return dat->getZ(dataRow, dataCol);
                    }
                }
            }
            break;

        case Qt::ToolTipRole:

            if (dat->xCount() == 0) //Value
            {
                if (row - sum == 2 && column == 0)
                   return dat->getZ(0);
            }
            else if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
            {
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                    return dat->getX(column);
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                    return dat->getZ(column);
            }
            else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                    return dat->getX(column - 1);
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0)
                    {
                        return dat->getY(row - sum - 3);
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        return dat->getZ(dataRow, dataCol);
                    }
                }
            }

            break;

        case Qt::StatusTipRole: // The dat displayed in the status bar.
            break;

        case Qt::WhatsThisRole: // The dat displayed for the item in "What's This?" mode.
            break;

        case Qt::SizeHintRole: // The size hint for the item that will be supplied to views.
            break;

        case Qt::FontRole : // The font used for items rendered with the default delegate.
            {
                QFont font;
                if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 0 && column < dat->xCount())
                         font.setBold(false);
                }
                else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                         font.setBold(false);
                    if (row - sum >= 3 && row - sum < dat->size - 1)
                    {
                        if (column == 0)
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
                QColor myvert = QColor::fromHsv(200, 110, 255);
                if (dat->xCount() == 0) //Value
                {
                    if (row - sum == 2 && column == 0)
                        return myjaune;
                }
                else if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 0 && column < dat->xCount())
                        return myvert;
                    else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                         return myjaune;
                }
                else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                        return myvert;
                    if (row - sum >= 3 && row - sum < dat->size - 1)
                    {
                        if (column == 0)
                        {
                             return myvert;
                        }
                        else if (column >= 1 && column <= dat->xCount())
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

                if (dat->xCount() == 0) //Value
                {
                    if (row - sum == 2 && column == 1)
                    {
                        bool bl;
                        double val1 = dat->getZ(0).toDouble(&bl);
                        double val2 = dat->getOrgZ(0).toDouble(&bl);

                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                    }
                }
                else if (dat->xCount() > 0 && dat->yCount() == 0) //Curve
                {
                    if (row - sum == 2 && column >= 0 && column < dat->xCount())
                    {
                        bool bl;
                        double val1 = dat->getX(column).toDouble(&bl);
                        double val2 = dat->getOrgX(column).toDouble(&bl);

                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                    }
                    else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                    {

                        bool bl;
                        double val1 = dat->getZ(column).toDouble(&bl);
                        double val2 = dat->getOrgZ(column).toDouble(&bl);

                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                    }
                }
                else if (dat->xCount() > 0 && dat->yCount() > 0) //Map
                {
                    if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                    {
                        bool bl;
                        double val1 = dat->getX(column - 1).toDouble(&bl);
                        double val2 = dat->getOrgX(column - 1).toDouble(&bl);

                        if (val1 > val2)
                            return color1;
                        else if (val1 < val2)
                            return color2;
                    }
                    if (row - sum >= 3 && row - sum < dat->size - 1)
                    {
                        if (column == 0)
                        {
                            bool bl;
                            double val1 = dat->getY(row - sum - 3).toDouble(&bl);
                            double val2 = dat->getOrgY(row - sum - 3).toDouble(&bl);

                            if (val1 > val2)
                                return color1;
                            else if (val1 < val2)
                                return color2;
                        }
                        else if (column >= 1 && column <= dat->xCount())
                        {                           
                            int dataRow = row - sum - 3;
                            int dataCol = column - 1;

                            bool bl1, bl2;
                            double val1 = dat->getZ(dataRow, dataCol, &bl1);
                            double val2 = dat->getOrgZ(dataRow, dataCol, &bl2);

                            if (bl1 && bl2)
                            {
                                if (val1 > val2)
                                    return color1;
                                else if (val1 < val2)
                                    return color2;
                            }
                            else
                            {
                                if (dat->getZ(dataRow, dataCol)!= dat->getOrgZ(dataRow, dataCol))
                                    return color1;
                            }
                        }
                    }
                }
            }
            break;
        }
    return QVariant();
}

QVariant GraphModel::data(int row, int col, int role)
{
     QModelIndex index = getIndex(row, col);
     return data(index, role);
}

bool GraphModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int sum = -2;

            // value
            if (dat->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 0))
                {
                    if (value.toString() != dat->getZ(0))
                    {
                        dat->setZ(0, value.toString());
                    }
                }
            }
            // curve
            else if (dat->xCount() > 0 && dat->yCount() == 0)
            {
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                {
                    if (value.toString() != dat->getX(column))
                    {
                        dat->setX(column , value.toString());
                    }
                }
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                {
                    if (value.toString() != dat->getZ(column))
                    {
                        dat->setZ(column, value.toString());
                    }
                }
            }
            // map
            else if (dat->xCount() > 0 && dat->yCount() > 0)
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                {
                    if (value.toString() != dat->getX(column - 1))
                    {
                        dat->setX(column - 1, value.toString());
                    }
                }
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0 && value.toString() != dat->getY(row - sum - 3))
                    {
                        dat->setY(row - sum - 3, value.toString());
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        if ( value.toString() != dat->getZ(dataRow, dataCol))
                        {
                            dat->setZ(dataRow, dataCol, value.toString());
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

bool GraphModel::setData(const QModelIndex &index, const QModelIndexList &indexList, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int sum = -2;

            // value
            if (dat->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 0))
                {
                    if (value.toString() != dat->getZ(0))
                    {
                        dat->setZ(0, value.toString());
                    }
                }
            }
            // curve
            else if (dat->xCount() > 0 && dat->yCount() == 0)
            {
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                {
                    if (value.toString() != dat->getX(column))
                    {
                        dat->setX(column , value.toString());
                    }
                }
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                {
                    if (value.toString() != dat->getZ(column))
                    {
                        dat->setZ(column, value.toString());
                    }
                }
            }
            // map
            else if (dat->xCount() > 0 && dat->yCount() > 0)
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                {
                    if (value.toString() != dat->getX(column - 1))
                    {
                        dat->setX(column - 1, value.toString());
                    }
                }
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0 && value.toString() != dat->getY(row - sum - 3))
                    {
                        dat->setY(row - sum - 3, value.toString());
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        if ( value.toString() != dat->getZ(dataRow, dataCol))
                        {
                            dat->setZ(dataRow, dataCol, value.toString());
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

bool GraphModel::setData(int row, int col, const QVariant &value, int role)
{
    QModelIndex index = getIndex(row, col);

    return setData(index, value, role);
}

bool GraphModel::resetData(const QModelIndex &index, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();
        int column = index.column();

        // determine the label in function of the row
        int sum = -2;

        // value
        if (dat->xCount() == 0)
        {
            if ((row - sum == 2) && (column == 1))
            {
                dat->resetValZ(0);
            }
        }
        // curve
        else if (dat->xCount() > 0 && dat->yCount() == 0)
        {
            if (row - sum == 2 && column >= 0 && column < dat->xCount())
            {
                dat->resetValX(column);
            }
            else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
            {
                dat->resetValZ(column);
            }
        }
        // map
        else if (dat->xCount() > 0 && dat->yCount() > 0)
        {
            if (row - sum == 2 && column >= 1 && column < dat->xCount() + 1)
            {
                dat->resetValX(column - 1);
            }
            if (row - sum >= 3 && row - sum < dat->size - 1)
            {
                if (column == 0)
                {
                    dat->resetValY(row - sum - 3);
                }
                else if (column >= 1 && column <= dat->xCount() )
                {
                    int dataRow = row - sum - 3;
                    int dataCol = column - 1;
                    dat->resetValZ(dataRow, dataCol);
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

bool GraphModel::resetData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int sum = -2;

            // value
            if (dat->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 1))
                {
                    dat->resetValZ(0);
                }
            }
            // curve
            else if (dat->xCount() > 0 && dat->yCount() == 0)
            {
                if (row - sum == 2 && column >= 0 && column < dat->xCount())
                {
                    dat->resetValX(column);
                }
                else if (row - sum == 3 && column >= 0 &&  column < dat->zCount())
                {
                    dat->resetValZ(column);
                }
            }
            // map
            else if (dat->xCount() > 0 && dat->yCount() > 0)
            {
                if (row - sum == 2 && column >= 1 && column < dat->xCount() + 1)
                {
                    dat->resetValX(column - 1);
                }
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0)
                    {
                        dat->resetValY(row - sum - 3);
                    }
                    else if (column >= 1 && column <= dat->xCount() )
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        dat->resetValZ(dataRow, dataCol);
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

bool GraphModel::undoData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int column = index.column();

            // determine the label in function of the row
            int sum = -2;

            // value
            if (dat->xCount() == 0)
            {
                if ((row - sum == 2) && (column == 0))
                {
                    dat->undoValZ(0);
                }
            }
            // curve
            else if (dat->xCount() > 0 && dat->yCount() == 0)
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                {
                    dat->undoValX(column - 1);
                }
                else if (row - sum == 3 && column >= 1 &&  column <= dat->zCount())
                {
                    dat->undoValZ(column - 1);
                }
            }
            // map
            else if (dat->xCount() > 0 && dat->yCount() > 0)
            {
                if (row - sum == 2 && column >= 1 && column <= dat->xCount())
                {
                    dat->undoValX(column - 1);
                }
                if (row - sum >= 3 && row - sum < dat->size - 1)
                {
                    if (column == 0)
                    {
                        dat->undoValY(row - sum - 3);
                    }
                    else if (column >= 1 && column <= dat->xCount())
                    {
                        int dataRow = row - sum - 3;
                        int dataCol = column - 1;
                        dat->undoValZ(dataRow, dataCol);
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

Qt::ItemFlags GraphModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant GraphModel::headerData(int section, Qt::Orientation /* orientation */, int role) const
{

    if (role != Qt::DisplayRole)
        return QVariant();

    return QString::number(section);
}

void GraphModel::setVal(Data *data)
{
    dat = data;
    nRow = dat->size;
    nColumn = dat->xCount() + 2;
    reset();
}

Data *GraphModel::getLabel(const QModelIndex &index, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        return dat;
    }
    else
    {
        return 0;
    }
}

Data *GraphModel::getLabel()
{
    return dat;
}

QModelIndex GraphModel::getIndex(int row, int col)
{
    if (dat->yCount() == 0)
        return createIndex(row + 1, col);
    else
        return createIndex(row + 1, col + 1);
}

QModelIndex GraphModel::getFirstZindex(Data *data)
{
    int row = 0;


    if (data->xCount() == 0)
    {
        return createIndex(row, 0);
    }
    else if (data->yCount() == 0)
    {
        return createIndex(row + 1, 0);
    }
    else
    {
        return createIndex(row + 1, 1);
    }
}

QModelIndexList GraphModel::selectAllX(Data *data)
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

QModelIndexList GraphModel::selectAllY(Data *data)
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

QModelIndexList GraphModel::selectAllZ(Data *data)
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
