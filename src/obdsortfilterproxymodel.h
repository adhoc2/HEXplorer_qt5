#ifndef OBDSORTFILTERPROXYMODEL_H
#define OBDSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "obdMergeModel.h"
#include "data.h"


class obdSortFilterProxyModel: public QSortFilterProxyModel
{


public:
    obdSortFilterProxyModel(QObject *parent = 0, ObdMergeModel *obdModel = 0);

    QString getFilter() const { return filter; }
    void setFilter(const QString &str);
    Data* getData(QModelIndex index);
    QStringList getUniqueValues(int column);
    void addFilter(int column, QString filter);
    void addFilters(QList<int> list, QString filter);
    void removeFilter(int column, QString filter);
    void removeFilters(QList<int> list, QString filter);
    void resetAllFilters();
    QMultiMap<int,QString> getfiltersMap() {return this->filtersMap;}


protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    //bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;


private:
    bool dateInRange(const QDate &date) const;

    QString filter;
    QMultiMap<int,QString> filtersMap;

};
#endif // OBDSORTFILTERPROXYMODEL_H
