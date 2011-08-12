#ifndef MEASMODEL_H
#define MEASMODEL_H

#include <QAbstractTableModel>
#include <Nodes/measurement.h>

class MeasModel : public QAbstractTableModel
{
    public:
        MeasModel(QObject *parent = 0);
        ~MeasModel();

        void setList(QList<Node*> labelList);

        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;

    private:
        QList<Node*> listMeas;
        int nColumn;
        int nRow;

};

#endif // MEASMODEL_H
