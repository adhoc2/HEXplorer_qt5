#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include <QAbstractTableModel>
#include "hexfile.h"
#include "data.h"

class GraphModel : public QAbstractTableModel
{
    public:
        GraphModel(QObject *parent = 0);
        ~GraphModel();

        void setVal(Data *data);
        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant data(int row, int col, int role);
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool setData(const QModelIndex &index, const QModelIndexList &indexList, const QVariant &value, int role);
        bool setData(int row, int col, const QVariant &value, int role);
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool resetData(const QModelIndex &index, int role);
        bool resetData(const QModelIndexList &indexList, int role);
        bool undoData(const QModelIndexList &indexList, int role);
        Data *getLabel(const QModelIndex &index, int role);
        Data *getLabel();
        QModelIndex getIndex(int row, int col);
        QModelIndex getFirstZindex(Data *data);
        QModelIndexList selectAllX(Data *data);
        QModelIndexList selectAllY(Data *data);
        QModelIndexList selectAllZ(Data *data);

    private:
        Data *dat;
        int nColumn;
        int nRow;

};

#endif // GRAPHMODEL_H
