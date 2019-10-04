#include "obdsortfilterproxymodel.h"
#include "obdMergeModel.h"
#include "qdebug.h"

obdSortFilterProxyModel::obdSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

/*bool obdSortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
    bool  bl = sourceModel()->data(index1).toString().contains(filterRegExp());
    return bl;
}*/


Data* obdSortFilterProxyModel::getData(QModelIndex indexProxy)
{
    QModelIndex indexSourceModel = mapToSource(indexProxy);
    Data* data = ((ObdMergeModel*)sourceModel())->getData(indexSourceModel.row(), indexSourceModel.column());
    return data;
}

