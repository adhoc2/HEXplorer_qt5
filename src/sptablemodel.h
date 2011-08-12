#ifndef SPTABLEMODEL_H
#define SPTABLEMODEL_H

#include <QAbstractTableModel>
#include "hexfile.h"
#include "data.h"

class SpTableModel : public QAbstractTableModel
{
    public:
        SpTableModel(QObject *parent = 0);
        ~SpTableModel();

        void setList(QList<Data*> *labelList);
        QList<Data*> *getList();
        QModelIndex getFirstZindex(Data *data);

        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool setData(const QModelIndex &index, const QModelIndexList &indexList ,const QVariant &value, int role);
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool resetData(const QModelIndex &index, int role);
        bool resetData(const QModelIndexList &indexList, int role);
        bool undoData(const QModelIndexList &indexList, int role);
        Data *getLabel(const QModelIndex &index, int role);
        QModelIndex getIndex(int row, int col);
        QModelIndexList selectAllX(Data *data);
        QModelIndexList selectAllY(Data *data);
        QModelIndexList selectAllZ(Data *data);

    private:
        QList<Data*> *listDataModel;
        QList<int> listDataIndex;
        int nColumn;
        int nRow;

};

#endif // SPTABLEMODEL_H
