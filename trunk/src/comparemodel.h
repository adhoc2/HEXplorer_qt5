#ifndef COMPAREMODEL_H
#define COMPAREMODEL_H

#include <QAbstractTableModel>
#include "hexfile.h"
#include "data.h"
#include "a2ltreemodel.h"

class CompareModel : public QAbstractTableModel
{
    public:
        CompareModel(QObject *parent = 0);
        ~CompareModel();

        QList<Data*> *getList1();
        QList<Data*> *getList2();
        QModelIndex getFirstZindex(Data *data);
        void setList(QList<Data*> *labelList, QList<Data*> *labelList2);
        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool setData(const QModelIndex &index, const QModelIndexList &indexList, const QVariant &value, int role);
        bool resetData(const QModelIndex &index, int role);
        bool resetData(const QModelIndexList &indexList, int role);
        bool undoData(const QModelIndexList &indexList, int role);
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        Data *getLabel(const QModelIndex &index, int role);
        QModelIndexList selectAllX(Data *data);
        QModelIndexList selectAllY(Data *data);
        QModelIndexList selectAllZ(Data *data);

    private:
        A2lTreeModel *treeModel;
        QList<Data*> *listDat1;
        QList<Data*> *listDat2;
        QList<int> listDataIndex;
        int nColumn;
        int nRow;

};

#endif // COMPAREMODEL_H
