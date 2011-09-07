#include "item.h"
#include "node.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

Item::Item(Node *parentNod)
{
    parentNode = parentNod;
}

Item::~Item()
{
}


TokenTyp Item::nextToken()
{
    TokenTyp token = parentNode->lex->getNextToken();

    while (token == Comment)
    {
        token = parentNode->lex->getNextToken();
    }

    return token;
}


void Item::showError(QString str)
{
    QString s;
    s.setNum(this->parentNode->lex->getLine());
    QString name = typeid(*this).name();

    parentNode->errorList->append(name + " parser : " + "wrong ASAP2 file format at line " + s + " :");
    parentNode->errorList->append(str);
}

