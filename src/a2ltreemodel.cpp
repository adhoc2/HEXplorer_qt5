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

#include <QtCore>
#include <QPixmap>


#include "a2ltreemodel.h"
#include "node.h"
#include "hexfile.h"
#include "srecfile.h"
#include <typeinfo>

using namespace std;

A2lTreeModel::A2lTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    rootNode = 0;
}

A2lTreeModel::~A2lTreeModel()
{
    delete rootNode;
}

void A2lTreeModel::createRootNode()
{
    rootNode = new Node();
}

void A2lTreeModel::addNode2RootNode(Node *node)
{
    // create a new rootNode if NULL
    if (rootNode == 0)
        rootNode = new Node();

    // add the node as childNode
    rootNode->addChildNode(node);
    node->setParentNode(rootNode);
    rootNode->sortChildrensName();

    //update model
    dataInserted(rootNode, rootNode->childNodes.indexOf(node));
}

QModelIndex A2lTreeModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    if (parentIndex.isValid() && parentIndex.column() != 0)
        return QModelIndex();

    Node *parentNode = nodeFromIndex(parentIndex);
    Node *childNode = parentNode->child(row);
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();
}

QModelIndex A2lTreeModel::getIndex(Node *node)
{
    if (node == rootNode)
    {
        return QModelIndex();
    }

    int pos = node->getParentNode()->childNodes.indexOf(node);
    return createIndex(pos, 0, node);
}

QModelIndex A2lTreeModel::parent(const QModelIndex &index) const
{
    Node *node = this->nodeFromIndex(index);
    if (node == rootNode || node == 0)
        return QModelIndex();

    Node *parentNode = node->getParentNode();
    if (parentNode == rootNode || parentNode == 0)
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

int A2lTreeModel::rowCount(const QModelIndex &index) const
{    
    if (index.column() > 0)
        return 0;

    Node *node = this->nodeFromIndex(index);

    return node->childCount();
}

int A2lTreeModel::columnCount(const QModelIndex &index) const
{
    Node *node = this->nodeFromIndex(index);

    return node->columnCount();
}

QVariant A2lTreeModel::data(const QModelIndex &index, int role) const
{
    Node *node = nodeFromIndex(index);
    if (!node)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (index.column() == 0)
        {
            QString str((node->name));
            return str;
        }
    }

    if (role == Qt::DecorationRole && index.column() == 0)
    {
        if (node->pixmap().length() < 1)
            return QVariant();
        else
        {
            QString str = node->pixmap().c_str();
            QPixmap pixmap(str);
            return pixmap.scaled(15, 15);
        }
    }

    return QVariant();
}

QVariant A2lTreeModel::line(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
    if (!node)
        return QVariant();
    else
        return node->a2lLine;
}

QMap<std::string, std::string>  *A2lTreeModel::getPar(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
    if (!node)
    {
        QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
        par->insert("no fix parameters", "");
        return par;
    }
    else
    {
        QMap<std::string, std::string> *par = node->getParameters();
        return par;
    }
}

std::string A2lTreeModel::getNodeName(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
    if (!node)
        return "";
    else
    {
        return node->name;
    }
}

QString A2lTreeModel::getFullNodeName(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
    if (!node)
        return "";
    else
    {
        QString str = node->name;
        while (node->getParentNode() && node->getParentNode() != rootNode)
        {
            str = QString(node->getParentNode()->name) + "/" + str;
            node = node->getParentNode();
        }
        return str;
    }
}

QVariant A2lTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
        {
            return tr("Project");
        }
    }
    return QVariant();
}

Node *A2lTreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<Node *>(index.internalPointer());
    }
    else
    {
        return rootNode;
    }
}

Node *A2lTreeModel::getNode(const QModelIndex &index)
{
    if (index.isValid())
    {
        return static_cast<Node *>(index.internalPointer());
    }
    else
    {
        return rootNode;
    }
}

QString A2lTreeModel::name(const QModelIndex &index)
{
    Node *node = nodeFromIndex(index);
    if (!node)
        return "";
    else
    {
        return node->fullName();
    }
}

Qt::DropActions A2lTreeModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags A2lTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QMimeData *A2lTreeModel::mimeData(const QModelIndexList &indexes) const
 {
     QMimeData *mimeData = new QMimeData();

     foreach (QModelIndex index, indexes)
     {
         if (index.isValid())
         {

             QString path = QString(getNodeName(index).c_str());
             index = parent(index);
             while (index.isValid())
             {
                 path = QString(getNodeName(index).c_str()) + "/" + path;
                 index = parent(index);
             }
             mimeData->setText(path);
         }
     }
     return mimeData;
 }

Node *A2lTreeModel::getRootNode()
{
    return rootNode;
}

void A2lTreeModel::dataInserted(Node *parentNode, int position)
{
    // Create an index of the parent node
    QModelIndex indexParent = getIndex(parentNode);

    // insert a row into parent node at position position
    insertRows(position, 1, indexParent);
}

void A2lTreeModel::dataRemoved(Node *nodeParent, int position, int rows)
{
    if (nodeParent == rootNode)
    {
        removeRows(position, rows, QModelIndex());
        return;
    }

    // Create an index of the parent node
    QModelIndex indexParent = getIndex(nodeParent);

    // create rows into parent node at position position
    removeRows(position, rows, indexParent);
}

bool A2lTreeModel::insertRows(int position, int rows, const QModelIndex &parentIndex)
{
    beginInsertRows(parentIndex, position, position + rows - 1);
    endInsertRows();
    return true;
}

bool A2lTreeModel::removeRows(int position, int rows, const QModelIndex &parentIndex)
{
    Node *nodeParent = getNode(parentIndex);

    // list the node to be deleted
    QList<Node*> list;
    for (int i = 0; i < rows; i++)
    {
        Node *node = nodeParent->childNodes.at(position + i);
        list.append(node);
    }

    //important to prevent view error (like freeze view)
    beginRemoveRows(parentIndex, position, position + rows - 1);

    foreach (Node *node, list)
    {
        nodeParent->removeChildNode(node);
        //node->setParentNode(NULL);
    }

    //enable view
    endRemoveRows();

    return true;
}

void A2lTreeModel::renameNode(QModelIndex index, QString newName)
{
    Node *node = nodeFromIndex(index);

    if (node)
    {
        Node *nodeParent = node->getParentNode();

        // remove node from view
        dataRemoved(nodeParent, nodeParent->childNodes.indexOf(node));

        // change node's name
        node->name = new char[newName.toLocal8Bit().count() + 1];
        strcpy(node->name, newName.toLocal8Bit().data());

        // add node to view with new name
        nodeParent->addChildNode(node);
        node->setParentNode(nodeParent);
        nodeParent->sortChildrensName();
        dataInserted(nodeParent, nodeParent->childNodes.indexOf(node));
    }
}

void A2lTreeModel::setListDataName(QStringList list)
{
    listDataName = list;
}

QStringList A2lTreeModel::getListDataName()
{
    return listDataName;
}

bool A2lTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        QString oldFullName = this->name(index);
        QString oldSuffix = QFileInfo(oldFullName).suffix();

        Node *node = getNode(index);
        QString newName = value.toString();
        QString newSuffix = QFileInfo(newName).suffix();
        QString newFullName = "";
        if (newSuffix == oldSuffix)
        {
            newFullName = QFileInfo(oldFullName).absolutePath() + "/" + newName;
        }
        else
        {
            newFullName = QFileInfo(oldFullName).absolutePath() +  "/" + newName + "." + oldSuffix;
            newName += "." + oldSuffix;
        }

        QString name = typeid(*node).name();

        if (name.toLower().endsWith("hexfile"))
        {
            HexFile* hex = dynamic_cast<HexFile*>(node);

            if (QFile::rename(oldFullName, newFullName))
            {
                renameNode(index, newName);
                hex->setFullName(newFullName);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if(name.toLower().endsWith("srecfile"))
        {
            SrecFile* srec = dynamic_cast<SrecFile*>(node);

            if (QFile::rename(oldFullName, newFullName))
            {
                renameNode(index, newName);
                srec->setFullName(newFullName);
                return true;
            }
            else
            {
                return false;
            }

        }
        else
            return false;
    }
    else
    {
        return false;
    }
}
