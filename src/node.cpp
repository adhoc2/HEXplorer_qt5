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
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegExp>
#include <typeinfo>
#include "node.h"
#include "lexer.h"
#include <cstring>
#include <errno.h>


bool compareNode(Node *a, Node *b)
{
   if (QString(a->name) < QString(b->name))
       return true;
   else return false;
}

Node::Node(Node *parent, A2lLexer *lexer, QStringList *error)
{
    stopped = false;
    parentNode = parent;
    lex = lexer;
    errorList = error;
    _pixmap = "";
    name = NULL;
}

Node::Node(char* nodeName)
{
   parentNode = 0;
   stopped = false;
   lex = NULL;
   errorList = NULL;
   _pixmap = "";
   name = nodeName;
}

Node::~Node()
{
    qDeleteAll(childNodes);
    childNodes.clear();
    qDeleteAll(optItems);
    optItems.clear();
}

bool Node::operator < (const Node & node)
{
    int i = strcmp(name, node.name);
    return (i < 0);
}

void Node::addChildNode(Node *child)
{
    childNodes += child;
}

void Node::insertChildNode(Node *child)
{
    //QList<Node*>::iterator i = qLowerBound(childNodes.begin(), childNodes.end(), child, compareNode);
    QList<Node*>::iterator i = std::lower_bound(childNodes.begin(), childNodes.end(), child, compareNode);
    childNodes.insert(i, child);
}

void Node::removeChildNode(Node *child)
{
    childNodes.removeOne(child);
}

void Node::addOptItem(Item *item)
{
    optItems += item;
}

Node *Node::child(int row)
{
    return childNodes.value(row);
}

Node *Node::child(QString str, bool bin)
{
   if (bin)
   {

        Node node;
        node.name = new char[str.length() + 1];
        strcpy(node.name, str.toLocal8Bit().data());
        //QList<Node*>::iterator i =  qBinaryFind(childNodes.begin(), childNodes.end(), &node, nodeLessThan);
        QList<Node*>::iterator i =  std::lower_bound(childNodes.begin(), childNodes.end(), &node, nodeLessThan);
        //QList<Node*>::iterator i =  std::lower_bound(childNodes.begin(), childNodes.end(), &node, compareNode);

        if (i == childNodes.end())
        {
            delete[] node.name;
            return NULL;
        }
        else
        {
            if (strcmp(((Node*)*i)->name, node.name) != 0)
            {
                delete[] node.name;
                return NULL;
            }
            else
            {
                delete[] node.name;
                return *i;
            }
        }
    }
    else
    {
        foreach(Node *child, childNodes)
        {
            if (child->name == str)
            {
                return child;
            }
        }
        return NULL;
    }
}

Node *Node::getNode(QString str)
{
    QStringList list = str.split("/");

    Node *node = child((list.at(0)), true);
    if (node == NULL)
    {
        return NULL;
    }

    int i = 1;
    while (i < list.count())
    {
        node = node->child((list.at(i)), true);
        if (node == NULL)
        {
            return NULL;
        }
        i++;
    }

    return node;
}

Node *Node::getNode(int num)
{
    if (num >0 && num < childCount() )
    {
        return childNodes.at(num);
    }
    else
        return 0;
}

int Node::childCount() const
{
    return childNodes.count();
}

int Node::columnCount() const
{
    return 2;
}

int Node::row() const
{
    if (parentNode)
        return parentNode->childNodes.indexOf(const_cast<Node*>(this));

    return 0;
}

Node *Node::getParentNode()
{
    return parentNode;
}

bool Node::isChild(std::string str)
{
    foreach(Node *node, childNodes)
    {
        if (node->name == str)
            return true;
    }
    return false;
}

bool Node::isChild(Node *data, bool sorted)
{
    if (sorted)
    {
        //QList<Node*>::iterator i = qBinaryFind(childNodes.begin(), childNodes.end(), data, compareNode);
        return std::binary_search(childNodes.begin(), childNodes.end(), data, compareNode);
//        if (i == childNodes.end())
//            return false;
//        else
//            return true;
    }
    else
    {
        return childNodes.contains(data);
    }

}

TokenTyp Node::nextToken()
{
    TokenTyp token = lex->getNextToken();

    while (token == Comment)
    {
        token = lex->getNextToken();
    }

    return token;
}

void Node::showError(QString str)
{
    QString s;
    s.setNum(lex->getLine());
    QString name = typeid(*this).name();

    *errorList << (name + " parser detected an error at line " + s);
    *errorList << str;
}

void Node::setParentNode(Node *node)
{
    parentNode = node;
}

void Node::sortChildrensName()
{
    std::sort(childNodes.begin(), childNodes.end(), nodeLessThan);
    //std::sort(childNodes.begin(), childNodes.end());
}

Item *Node::getItem(QString str)
{
    foreach(Item *item, optItems)
    {
        if (item->name == str)
        {
            return item;
        }
    }
    return nullptr;
}

std::string Node::fixPar(QString str)
{
    QMap<std::string, std::string> *map = getParameters();
    std::string tt = map->value(str.toLocal8Bit().data());
    delete map;
    return  tt;
}

QString Node::getFullTreePath()
{
    QString str = this->name;
    Node* node = this;
    while (node->getParentNode()&& node->getParentNode()->getParentNode() != NULL)
    {
        str = QString(node->getParentNode()->name) + "/" + str;
        node = node->getParentNode();
    }
    return str;
}

Node* Node::interpolationSearch(QList<Node *> sortedArray, QString str)
{
    int low = 0;
    int high = sortedArray.length() - 1;
    int mid;


    while (QString(sortedArray.at(low)->name) <= str && QString(sortedArray.at(high)->name) >= str)
    {

        mid = low +
                (distance(str.toStdString().c_str() , sortedArray.at(low)->name) * (high - low)) /
                (distance(sortedArray.at(high)->name, sortedArray.at(low)->name));

        if (QString(sortedArray.at(mid)->name) < str)
            low = mid + 1;
        else if (QString(sortedArray.at(mid)->name) > str)
            high = mid - 1;
        else
            return sortedArray.at(mid);
    }

    if (QString(sortedArray.at(low)->name) == str)
        return sortedArray.at(low);
    else
        return 0;
}

int Node::distance(QString source, QString target)
{
    // Step 1

      const int n = source.length();
      const int m = target.length();
      if (n == 0) {
        return m;
      }
      if (m == 0) {
        return n;
      }

      // Good form to declare a TYPEDEF

      typedef std::vector< std::vector<int> > Tmatrix;

      Tmatrix matrix(n+1);

      // Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
      // allow for allocation on declaration of 2.nd dimension of vec of vec

      for (int i = 0; i <= n; i++) {
        matrix[i].resize(m+1);
      }

      // Step 2

      for (int i = 0; i <= n; i++) {
        matrix[i][0]=i;
      }

      for (int j = 0; j <= m; j++) {
        matrix[0][j]=j;
      }

      // Step 3

      for (int i = 1; i <= n; i++) {

        const char s_i = source.at(i-1).toLatin1();

        // Step 4

        for (int j = 1; j <= m; j++) {

          const char t_j = target.at(j-1).toLatin1();

          // Step 5

          int cost;
          if (s_i == t_j) {
            cost = 0;
          }
          else {
            cost = 1;
          }

          // Step 6

          int above = matrix[i-1][j];
          int left = matrix[i][j-1];
          int diag = matrix[i-1][j-1];
          int cell = std::min( above + 1, std::min(left + 1, diag + cost));

          // Step 6A: Cover transposition, in addition to deletion,
          // insertion and substitution. This step is taken from:
          // Berghel, Hal ; Roach, David : "An Extension of Ukkonen's
          // Enhanced Dynamic Programming ASM Algorithm"
          // (http://www.acm.org/~hlb/publications/asm/asm.html)

          if (i>2 && j>2) {
            int trans=matrix[i-2][j-2]+1;
            if (source[i-2]!=t_j) trans++;
            if (s_i!=target[j-2]) trans++;
            if (cell>trans) cell=trans;
          }

          matrix[i][j]=cell;
        }
      }

      // Step 7

      return matrix[n][m];
}
