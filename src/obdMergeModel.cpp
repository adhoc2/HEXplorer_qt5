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

#include "ObdMergeModel.h"
#include "Nodes/compu_method.h"
#include <QFont>
#include <QList>
#include "qdebug"


ObdMergeModel::ObdMergeModel(SrecFile *srecFile, QObject *parent)
    :QAbstractTableModel(parent)
{
    nRow = 0;
    nColumn = 0;
    dataContainer = srecFile;
    listErrorCodes();

}

ObdMergeModel::ObdMergeModel(CdfxFile *cdfx, QObject *parent)
    :QAbstractTableModel(parent)
{
    nRow = 0;
    nColumn = 0;
    dataContainer = cdfx;
    listErrorCodes();
}

ObdMergeModel::ObdMergeModel(Dcm *dcm, QObject *parent)
    :QAbstractTableModel(parent)
{
    nRow = 0;
    nColumn = 0;
    dataContainer = dcm;
    listErrorCodes();
}

ObdMergeModel::ObdMergeModel(Csv *csv, QObject *parent)
    :QAbstractTableModel(parent)
{
    nRow = 0;
    nColumn = 0;
    dataContainer = csv;
    listErrorCodes();
}


ObdMergeModel::~ObdMergeModel()
{

}

void ObdMergeModel::listErrorCodes()
{
    QStringList listErrorEndings({"Prio", "Inc", "Dec", "PreThd", "DebounceBehavior", "SetTimeMsOrNbOcc",
                                 "ResetTimeMsOrNbOcc", "EventType", "AgiCycIdn", "AgiCycThd", "OperCycIdn",
                                 "OperCycThd", "ExclsnCdn", "FltReactnId", "DelayId","InhbnMask"});

    foreach (Data *data, dataContainer->listData)
    {
        QString nameStr(data->getName());
        if (!listDataNameInView.contains(nameStr))
        {
            QString nameEnd = nameStr.split(".").last();
            //qDebug() << nameEnd;
            if (listErrorEndings.contains(nameEnd))
            {
                QString nameBegin = "";
                if (nameEnd == "FltReactnId" || nameEnd == "DelayId")
                {
                    nameBegin = nameStr.split("Frm_A[").first();
                }
                else
                {
                    nameBegin = nameStr.split("_C.").first();
                }

                //check if errorCode has been previously created
                //if not create a new one
                ErrorCode* error = getErrorCode(nameBegin);
                if (!error)
                {
                    error = new ErrorCode(nameBegin);
                }

                listDataNameInView.append(nameStr);
                error->dtc = data->getComment().remove("DTC-ID: ");
                error->dtc.remove("\"");
                error->dtc.remove("\"");

                Data* _data = dataContainer->getData(nameBegin + "_C.Prio");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.Inc");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.Dec");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.PreThd");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.DebounceBehavior");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.SetTimeMsOrNbOcc");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.ResetTimeMsOrNbOcc");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.EventType");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.AgiCycIdn");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.AgiCycThd");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.OperCycIdn");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.OperCycThd");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.ExclsnCdn");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "_C.InhbnMask");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[0].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[1].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[2].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[3].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[4].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[5].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[6].FltReactnId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[0].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[1].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[2].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[3].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[4].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[5].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }
                _data = dataContainer->getData(nameBegin + "Frm_A[6].DelayId");
                if (_data) {
                    listDataNameInView.append(_data->getName());
                }               

                this->listErrorCode.append(error);
            }            
            else if (nameStr.contains("Inhbn_A["))
            {
                QString nameBegin = nameStr.split("Inhbn_A[").first();
                QString nameEnd = nameStr.split("Inhbn_A[").last().remove("]");

                ErrorCode* error = getErrorCode(nameBegin);
                if (!error)
                {
                    error = new ErrorCode(nameBegin);
                }
            }
        }
    }

    nRow = this->listErrorCode.count();
    nColumn = 53;

}

int ObdMergeModel::rowCount(const QModelIndex &parent) const
{
    return nRow;
}

int ObdMergeModel::columnCount(const QModelIndex &parent) const
{
    return nColumn;
}

QVariant ObdMergeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (0 <= row && row < nRow)
    {

        switch (role)
        {
        case Qt::DisplayRole :
            {
               if (column == 0) //DTC
               {
                   return this->listErrorCode.at(row)->dtc;
               }
               else if (column == 1) //Error Name
               {
                   return this->listErrorCode.at(row)->name;
               }
               else
               {
                   Data* data = this->getData(row, column);
                   if (data)
                   {
                       return data->getZ(0);
                   }
                   else
                       return "";
               }


               return QVariant();

            }
            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
        {
            int row = index.row();
            int col = index.column();

            Data* data = this->getData(row, col);
            if (data)
            {
               return data->getZ(0);
            }

            break;
        }

        case Qt::ToolTipRole:
        {
            break;
        }

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
        {
            QColor myjaune = QColor::fromHsv(60, 110, 255);
            QColor mybleu = QColor::fromHsv(200, 110, 255);
            QColor myvert = QColor::fromHsv(120, 110, 255);

            Data* data = this->getData(row, column);

            if (data)
            {
                if (data->getOrgZ(0) != data->getZ(0))
                {
                    return myvert;
                }
            }

            break;
        }

        case Qt::ForegroundRole: // the foreground brush (text color, typically) used for items rendered with the default delegate.
            {
                if (column == 0)
                {
                    //QColor color = Qt::blue;
                    //return color;
                }
                else
                {
                    QColor color1 = Qt::red;
                    QColor color2 = Qt::blue;


                    Data* data = this->getData(row, column);
                    if (data)
                    {
                        bool bl1;
                        double val1 = data->getZ(0).toDouble(&bl1);
                        bool bl2;
                        double val2 = data->getOrgZ(0).toDouble(&bl2);

                        if (bl1 && bl2)
                        {
                            if (val1 > val2)
                                return color1;
                            else if (val1 < val2)
                                return color2;
                        }
                        else
                        {
                            if (data->getZ(0) != data->getOrgZ(0))
                                return color1;
                        }
                    }
                }
            }
            break;
        }
    }

    return QVariant();
}

bool ObdMergeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    if (0 <= row && row < nRow)
    {
        Data* data = this->getData(row, col);
        if (data)
        {

            //set the value
            data->setZ(0, value.toString());


           //update the tableView
           emit dataChanged(index, index);

           return true;
        }
        else
            return false;
    }

emit obdDataChanged();

}

bool ObdMergeModel::resetData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int col = index.column();

            if (0 <= row && row < nRow)
            {
                Data* data = this->getData(row, col);
                if (data)
                {
                   //set the value
                   data->resetValZ(0);


                   //update the tableView
                   emit dataChanged(index, index);
                   //return true;
                }
                else
                    return false;
            }

       }
    }

    //update the tableView
    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    emit dataChanged(topLeft, btmRight);

    return true;
}

bool ObdMergeModel::undoData(const QModelIndexList &indexList, int role)
{
    foreach (QModelIndex index, indexList)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            int row = index.row();
            int col = index.column();

            if (0 <= row && row < nRow)
            {
                Data* data = this->getData(row, col);
                if (data)
                {
                   //set the value
                   data->undoValZ(0);

                   //update the tableView
                   emit dataChanged(index, index);
                   //return true;
                }
                else
                    return false;
            };

          }
    }

    //update the tableView
    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    emit dataChanged(topLeft, btmRight);

    return true;
}

Qt::ItemFlags ObdMergeModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant ObdMergeModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                    return "Inc";
                }
                else if (section == 3)
                {
                    return "Dec";
                }
                else if (section == 4)
                {
                    return "PreThd";
                }
                else if (section == 5)
                {
                    return "Debounce";
                }
                else if (section == 6)
                {
                    return "SetTime";
                }
                else if (section == 7)
                {
                    return "ResetTime";
                }
                else if (section == 8)
                {
                    return "EventType";
                }
                else if (section == 9)
                {
                    return "Prio";
                }
                else if (section == 10)
                {
                    return "AgiCycIdn";
                }
                else if (section == 11)
                {
                    return "AgiCycThd";
                }
                else if (section == 12)
                {
                    return "OperCycIdn";
                }
                else if (section == 13)
                {
                    return "OperCycThd";
                }
                else if (section == 14)
                {
                    return "ExclsnCdn";
                }
                else if (section == 15)
                {
                    return "reaction_0";
                }
                else if (section == 16)
                {
                    return "delay_0";
                }
                else if (section == 17)
                {
                    return "reaction_1";
                }
                else if (section == 18)
                {
                    return "delay_1";
                }
                else if (section == 19)
                {
                    return "reaction_2";
                }
                else if (section == 20)
                {
                    return "delay_2";
                }
                else if (section == 21)
                {
                    return "reaction_3";
                }
                else if (section == 22)
                {
                    return "delay_3";
                }
                else if (section == 23)
                {
                    return "reaction_4";
                }
                else if (section == 24)
                {
                    return "delay_4";
                }
                else if (section == 25)
                {
                    return "reaction_5";
                }
                else if (section == 26)
                {
                    return "delay_5";
                }
                else if (section == 27)
                {
                    return "reaction_6";
                }
                else if (section == 28)
                {
                    return "delay_6";
                }
                else if (section == 29)
                {
                    return "InhbnMask";
                }
                else if (section >= 30)
                {
                    return "Inhbn_A[" + QString::number(section - 30) + "]";
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

int ObdMergeModel::getRow(const int col, const QString text) const
{
    return 0;
}

Data* ObdMergeModel::getData(const int row, const int col) const
{
    QString dataName = "";
    dataName += listErrorCode.at(row)->name;
    if (col == 2) {  dataName += "_C.Inc";  }
    else if (col == 3)   {  dataName += "_C.Dec";   }
    else if (col == 4)  {  dataName += "_C.PreThd";   }
    else if (col == 5)  {  dataName += "_C.DebounceBehavior";   }
    else if (col == 6)  {  dataName += "_C.SetTimeMsOrNbOcc";   }
    else if (col == 7)  {  dataName += "_C.ResetTimeMsOrNbOcc";   }
    else if (col == 8)  {  dataName += "_C.EventType";   }
    else if (col == 9)  {  dataName += "_C.Prio"; }
    else if (col == 10)   {  dataName += "_C.AgiCycIdn";   }
    else if (col == 11)  {  dataName += "_C.AgiCycThd";   }
    else if (col == 12)  {  dataName += "_C.OperCycIdn"; }
    else if (col == 13)   {  dataName += "_C.OperCycThd";   }
    else if (col == 14)  {  dataName += "_C.ExclsnCdn";   }
    else if (col == 15)  {  dataName += "Frm_A[0].FltReactnId"; }
    else if (col == 16)  {  dataName += "Frm_A[0].DelayId"; }
    else if (col == 17)  {  dataName += "Frm_A[1].FltReactnId"; }
    else if (col == 18)  {  dataName += "Frm_A[1].DelayId"; }
    else if (col == 19)  {  dataName += "Frm_A[2].FltReactnId"; }
    else if (col == 20)  {  dataName += "Frm_A[2].DelayId"; }
    else if (col == 21)  {  dataName += "Frm_A[3].FltReactnId"; }
    else if (col == 22)  {  dataName += "Frm_A[3].DelayId"; }
    else if (col == 23)  {  dataName += "Frm_A[4].FltReactnId"; }
    else if (col == 24)  {  dataName += "Frm_A[4].DelayId"; }
    else if (col == 25)  {  dataName += "Frm_A[5].FltReactnId"; }
    else if (col == 26)  {  dataName += "Frm_A[5].DelayId"; }
    else if (col == 27)  {  dataName += "Frm_A[6].FltReactnId"; }
    else if (col == 28)  {  dataName += "Frm_A[6].DelayId"; }
    else if (col == 29)  {  dataName += "_C.InhbnMask"; }
    else if (col >= 30)
    {
        dataName += "Inhbn_A[" + QString::number(col - 30) + "]";
    }

    Data *data = dataContainer->getData(dataName);
    return data;
}

bool ObdMergeModel::exportAs(QString format, QString filename)
{
    if (format.endsWith("cdfx"))
    {
        return dataContainer->exportDataList2Cdf(listDataNameInView);
    }
    else if (format.endsWith("csv"))
    {
        return dataContainer->exportDataList2Csv(listDataNameInView);
    }
}

ErrorCode* ObdMergeModel::getErrorCode(QString errorName)
{
    foreach (ErrorCode* errorCode, listErrorCode)
    {
        if (errorCode->name == errorName)
            return errorCode;
    }

    return nullptr;
}
