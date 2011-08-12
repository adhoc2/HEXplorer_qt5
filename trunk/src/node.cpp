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
    QList<Node*>::iterator i = qLowerBound(childNodes.begin(), childNodes.end(), child, compareNode);
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
        QList<Node*>::iterator i =  qBinaryFind(childNodes.begin(), childNodes.end(), &node, nodeLessThan);
        if (i == childNodes.end())
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
        QList<Node*>::iterator i = qBinaryFind(childNodes.begin(), childNodes.end(), data, compareNode);
        if (i == childNodes.end())
            return false;
        else
            return true;
    }
    else
    {
        return childNodes.contains(data);
    }

}

TokenTyp Node::nextToken(QTextStream &in)
{
    TokenTyp token = lex->getNextToken(in);

    while (token == Comment)
    {
        token = lex->getNextToken(in);
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
    qSort(childNodes.begin(), childNodes.end(), nodeLessThan);
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
    return NULL;
}

std::string Node::fixPar(QString str)
{
    QMap<std::string, std::string> *map = getParameters();
    std::string tt = map->value(str.toLocal8Bit().data());
    delete map;
    return  tt;
}
