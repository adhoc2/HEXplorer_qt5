#ifndef ITEM_H
#define ITEM_H

#include <QMap>
#include "lexer.h"
#include "QMutex"

class Node;

class Item
{
    public:
        Item(Node *parentNod);
        virtual ~Item();

        virtual QMap<std::string, std::string> getParameters()
        {
            QMap<std::string, std::string> map;
            return map;
        }
        TokenTyp nextToken();
        std::string getName() const
        {
            std::string str = name;
            return str;
        }
        char* name;

    protected:
        Node *parentNode;
        void showError(QString str);
    };

#endif // ITEM_H
