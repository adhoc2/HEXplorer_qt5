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
        Node(Node *parent, A2lLexer *lexer, QStringList *error);
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
        std::string fixPar(QString str);


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
        Item *getItem(QString str);
        TokenTyp nextToken(QTextStream &in);

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

};

#endif
