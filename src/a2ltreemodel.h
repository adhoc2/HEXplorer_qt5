// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

#ifndef A2LTREEMODEL_H
#define A2LTREEMODEL_H

#include <QAbstractItemModel>

class Node;

class A2lTreeModel : public QAbstractItemModel
{
public:
    A2lTreeModel(QObject *parent = 0);
    ~A2lTreeModel();

    void createRootNode();
    void addNode2RootNode(Node *node);
    void dataInserted(Node *parentNode, int position);
    void startInsertData(Node *parentNode, int position, int count = 1);
    void stopInsertData(Node *parentNode, int position, int count = 1);
    void dataRemoved(Node *nodeParent, int position, int count = 1);
    void renameNode(QModelIndex index, QString name);
    void resetModel();
    QModelIndexList getPersistentIndexList();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant line(const QModelIndex &index) const;
    QMap<std::string, std::string> *getPar(const QModelIndex &index) const;
    std::string getNodeName(const QModelIndex &index) const;
    QString getFullNodeName(const QModelIndex &index) const;
    QString name(const QModelIndex &index);
    Node *getNode(const QModelIndex &index);
    Node *getRootNode();
    bool insertRows(int position, int rows,
                    const QModelIndex &parentIndex = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parentIndex = QModelIndex());
    QModelIndex getIndex(Node *node);
    void setListDataName(QStringList list);
    QStringList getListDataName();


private:
    Node *nodeFromIndex(const QModelIndex &index) const;
    Node *rootNode;
    QStringList listDataName;
};

#endif // A2LTREEMODEL_H
