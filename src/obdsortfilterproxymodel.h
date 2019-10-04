#ifndef OBDSORTFILTERPROXYMODEL_H
#define OBDSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "data.h"


class obdSortFilterProxyModel: public QSortFilterProxyModel
{


public:
    obdSortFilterProxyModel(QObject *parent = 0);

    QString getFilter() const { return filter; }
    void setFilter(const QString &str);

    Data* getData(QModelIndex index);
    QStringList getUniqueValues(int column);


protected:
    //bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    //bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;


private:
    bool dateInRange(const QDate &date) const;

    QString filter;

};
#endif // OBDSORTFILTERPROXYMODEL_H
