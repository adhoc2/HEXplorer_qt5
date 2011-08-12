#ifndef CHARMODEL_H
#define CHARMODEL_H

#include <QAbstractTableModel>
#include <Nodes/characteristic.h>

class CharModel : public QAbstractTableModel
{
    public:
        CharModel(QObject *parent = 0);
        ~CharModel();

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

#endif // CHARMODEL_H
