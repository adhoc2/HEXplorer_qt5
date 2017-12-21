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

#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>
#include <QMap>
#include <QTextStream>
#include <QStringList>
#include <QPixmap>
#include "lexer.h"
#include "item.h"
#include "QMutex"


class A2lGrammar;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

class Node
{
    public:
        Node(Node *parent, A2lLexer *lexer = 0, QStringList *error = 0);
        Node(char* nodeName = NULL);
        bool operator < (const Node & node);
        virtual ~Node();

        virtual QString fullName()
        {
            return "";
        }
        virtual std::string pixmap()
        {
            return _pixmap;
        }       
        virtual QMap<std::string, std::string> *getParameters()
        {
            QMap<std::string, std::string> *map = new QMap<std::string, std::string>;
            return map;
        }
        virtual QMap<QString, QString> *getOptItems(){
            QMap<QString, QString> *map = new QMap<QString, QString>;
            return map;
        }

        std::string fixPar(QString str);
        QString getFullTreePath();
        void addChildNode (Node *child);
        void insertChildNode (Node *child);
        void removeChildNode(Node *child);
        void addOptItem (Item *item);
        void setParentNode(Node *node);
        void sortChildrensName();
        bool isChild(std::string str);
        bool isChild(Node *data, bool bl = false);
        int childCount() const;
        int columnCount() const;
        int row() const;
        Node *getParentNode();
        Node *child(int row);
        Node *child(QString str, bool bin);
        Node *getNode(QString str);
        Node *getNode(int num);
        Item *getItem(QString str);
        TokenTyp nextToken();

        A2lLexer *lex;
        int a2lLine;
        std::string _pixmap;
        char *name;
        QList<Node*> childNodes;
        QList<Item*> optItems;
        QStringList *errorList;
        bool stopped;

    protected:
        Node* parentNode;
        void showError(QString str);

private:
        Node* interpolationSearch(QList<Node*> sortedArray, QString str);
        int distance(QString source, QString target);

};

#endif
