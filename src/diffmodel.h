#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <QAbstractItemModel>

class Noeud;

class DiffModel : public QAbstractItemModel
{
public:
    DiffModel(QObject *parent = 0);
    ~DiffModel();

    void setRootNode(Noeud *node);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    Noeud *nodeFromIndex(const QModelIndex &index) const;
    Noeud *rootNode;
};

#endif // DIFFMODEL_H
