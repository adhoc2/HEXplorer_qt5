#include "obdsortfilterproxymodel.h"
#include "obdMergeModel.h"
#include "qdebug.h"

obdSortFilterProxyModel::obdSortFilterProxyModel(QObject *parent, ObdMergeModel *obdModel)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(obdModel);
    //connect(obdModel, SIGNAL(dataChanged()), this , SLOT(setFilterRegExp("")));
}

bool obdSortFilterProxyModel::filterAcceptsRow(int sourceRow,const QModelIndex &sourceParent) const
{

    bool bl = true;
    QList<int> listCol({11, 13, 15, 17, 19, 21, 23});
    bool flagReaction = false;

    foreach (int col, filtersMap.keys())
    {
        if (!listCol.contains(col))
        {
            bool bl_col = false;
            QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
            QList<QString> values = filtersMap.values(col);
            for (int i = 0; i < values.size(); ++i)
            {
                QRegExp regExp("^" + values.at(i) + "$");
                QString str = sourceModel()->data(index).toString();
                bl_col = bl_col ||  str.contains(regExp);
            }
            bl = bl && bl_col;
        }
        else
            flagReaction = true;
    }

    if (!flagReaction) return bl;
    else
    {
        bool bl_reaction = false;
        foreach (int col, filtersMap.keys())
        {
            if (listCol.contains(col))
            {
                bool bl_col = false;
                QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
                QList<QString> values = filtersMap.values(col);
                for (int i = 0; i < values.size(); ++i)
                {
                    QRegExp regExp("^" + values.at(i) + "$");
                    QString str = sourceModel()->data(index).toString();
                    bl_col = bl_col ||  str.contains(regExp);
                }
                bl_reaction = bl_reaction || bl_col;
            }
         }
        return bl && bl_reaction;
    }
}

Data* obdSortFilterProxyModel::getData(QModelIndex indexProxy)
{
    QModelIndex indexSourceModel = mapToSource(indexProxy);
    Data* data = ((ObdMergeModel*)sourceModel())->getData(indexSourceModel.row(), indexSourceModel.column());
    return data;
}

QStringList obdSortFilterProxyModel::getUniqueValues(int column)
{
    QStringList list;
    int nrow = sourceModel()->rowCount();
    for (int i=0; i < nrow; i++)
    {
        QModelIndex index1 = sourceModel()->index(i, column, QModelIndex());
        QString value = sourceModel()->data(index1).toString();
        if (!list.contains(value))
        {
            list.append(value);
        }
    }
    return list;
}

void obdSortFilterProxyModel::addFilter(int column, QString filter)
{
    this->filtersMap.insert(column,filter);

    QRegExp regExp("");
    this->setFilterRegExp(regExp);
}

void obdSortFilterProxyModel::addFilters(QList<int> list, QString filter)
{
    foreach (int i, list)
    {
        this->filtersMap.insert(i,filter);
    }

    QRegExp regExp("");
    this->setFilterRegExp(regExp);
}

void obdSortFilterProxyModel::removeFilter(int column, QString filter)
{
    this->filtersMap.remove(column, filter);

    QRegExp regExp("");
    this->setFilterRegExp(regExp);
}

void obdSortFilterProxyModel::removeFilters(QList<int> list, QString filter)
{
    foreach (int i, list)
    {
        this->filtersMap.remove(i,filter);
    }

    QRegExp regExp("");
    this->setFilterRegExp(regExp);
}

void obdSortFilterProxyModel::resetAllFilters()
{
    this->filtersMap.clear();

    QRegExp regExp("");
    this->setFilterRegExp(regExp);
}
