#include "charmodel.h"
#include "Nodes/compu_method.h"
#include <QFont>

CharModel::CharModel(QObject *parent)
{
    nRow = 0;
    nColumn = 0;
}

CharModel::~CharModel()
{

}

void CharModel::setList(QList<Node *> labelList)
{

    listMeas = labelList;

    nRow = labelList.count() + 1;
    nColumn = 10;

    reset();
}

int CharModel::rowCount(const QModelIndex &parent) const
{
    return nRow;
}

int CharModel::columnCount(const QModelIndex &parent) const
{
    return nColumn;
}

QVariant CharModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();


    if (row == 0)
    {
        switch (role)
        {
        case Qt::DisplayRole :
            {

                if (column == 0)
                {
                    return "name";
                }
                else if (column == 1)
                {
                    return "description";
                }
                else if (column == 2)
                {
                    return "unit";
                }
                else if (column == 3)
                {
                    return "subset";
                }
                else if (column == 4)
                {
                    return "ECU address";
                }
                else if (column == 5)
                {
                    return "data type";
                }
                else if (column == 6)
                {
                    return "computation method";
                }
                else if (column == 7)
                {
                    return "Deposit";
                }
                else if (column == 8)
                {
                    return "MaxDiff";
                }
                else if (column == 9)
                {
                    return "LowerLimit";
                }
                else if (column == 10)
                {
                    return "UpperLimit";
                }
                else
                    return QVariant();


            }
            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
            {
            }
            break;

        case Qt::ToolTipRole:
            {
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
                font.setBold(true);
                return font;
            }
            break;

        case Qt::TextAlignmentRole:
            break;

        case Qt::BackgroundRole:
            {

            }
            break;

        case Qt::ForegroundRole: // the foreground brush (text color, typically) used for items rendered with the default delegate.
            {
                return Qt::red;
            }
            break;
        }

    }
    else if (0 < row && row <= nRow)
    {
        CHARACTERISTIC *label = (CHARACTERISTIC*)listMeas.at(row - 1);

        switch (role)
        {
        case Qt::DisplayRole :
            {
                QString compu_method = label->getPar("Conversion");

                Node * node = label->getParentNode()->getParentNode();
                COMPU_METHOD *cmp = (COMPU_METHOD*)node->getNode("COMPU_METHOD/" + compu_method);


                if (column == 0)
                {
                    return label->name;
                }
                else if (column == 1)
                {
                    QString str = label->fixPar("LongIdentifier").c_str();
                    return str;
                }
                else if (column == 2)
                {
                    return cmp->getPar("Unit");
                }
                else if (column == 3)
                {
                    return ((CHARACTERISTIC*)label)->getSubsetName();
                }

                else if (column == 4)
                {
                    QString str = label->fixPar("Adress").c_str();
                    return str;
                }
                else if (column == 5)
                {
                    QString str = label->fixPar("Type").c_str();
                    return str;
                }
                else if (column == 6)
                {
                    QString str = label->fixPar("Conversion").c_str();
                    return str;
                }
                else if (column == 7)
                {
                    QString str = label->fixPar("Deposit").c_str();
                    return str;
                }
                else if (column == 8)
                {
                    QString str = label->fixPar("MaxDiff").c_str();
                    return str;
                }
                else if (column == 9)
                {
                    QString str = label->fixPar("LowerLimit").c_str();
                    return str;
                }
                else if (column == 10)
                {
                    QString str = label->fixPar("UpperLimit").c_str();
                    return str;
                }
                else
                    return QVariant();


            }
            break;

        case Qt::DecorationRole: // The data to be rendered as a decoration in the form of an icon.
            break;

        case Qt::EditRole:
            {
            }
            break;

        case Qt::ToolTipRole:
            {
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

            }
            break;

        case Qt::ForegroundRole: // the foreground brush (text color, typically) used for items rendered with the default delegate.
            {
                if (column == 0) {
                    return Qt::blue;
                }
            }
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags CharModel::flags(const QModelIndex &index) const
 {
     Qt::ItemFlags flags = QAbstractItemModel::flags(index);
     flags |= Qt::ItemIsEditable;
     return flags;
 }

QVariant CharModel::headerData(int section, Qt::Orientation /* orientation */, int role) const
{

    if (role != Qt::DisplayRole)
        return QVariant();

    return QString::number(section);
}
