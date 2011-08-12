#include "diffmodel.h"
#include "noeud.h"

DiffModel::DiffModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootNode = 0;
}

DiffModel::~DiffModel()
{
    delete rootNode;
}

void DiffModel::setRootNode(Noeud *node)
{
    delete rootNode;
    rootNode = node;
    reset();
}

QModelIndex DiffModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!rootNode)
        return QModelIndex();
    Noeud *parentNode = nodeFromIndex(parent);
    return createIndex(row, column, parentNode->children[row]);
}

QModelIndex DiffModel::parent(const QModelIndex &child) const
{
    Noeud *node = nodeFromIndex(child);
    if (!node)
        return QModelIndex();
    Noeud *parentNode = node->parent;
    if (!parentNode)
        return QModelIndex();
    Noeud *grandparentNode = parentNode->parent;
    if (!grandparentNode)
        return QModelIndex();

    int row = grandparentNode->children.indexOf(parentNode);
    return createIndex(row, child.column(), parentNode);
}

int DiffModel::rowCount(const QModelIndex &parent) const
{
    Noeud *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return 0;
    return parentNode->children.count();
}

int DiffModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

QVariant DiffModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    Noeud *node = nodeFromIndex(index);
    if (!node)
        return QVariant();

     if (index.column() == 0) {
        return node->str;
    }
    return QVariant();
}

QVariant DiffModel::headerData(int section,
                                 Qt::Orientation orientation,
                                 int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Label");
        }
    }
    return QVariant();
}

Noeud *DiffModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<Noeud *>(index.internalPointer());
    } else {
        return rootNode;
    }
}

