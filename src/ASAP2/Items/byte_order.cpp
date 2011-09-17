#include "byte_order.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,Byte_Order> Byte_Order::itemFactory;

Byte_Order::Byte_Order( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->byte_order.namePar;
    typePar = &gram->byte_order.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    name = (char*)"BYTE_ORDER";
}

Byte_Order::~Byte_Order()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void Byte_Order::parseFixPar(QList<TokenTyp> *typePar)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken();
        if (token == typePar->at(i))
        {
            char *c = new char[parentNode->lex->getLexem().length()+1];
            strcpy(c, parentNode->lex->getLexem().c_str());
            parameters.append(c);
        }
        else
        {
            QString t(this->parentNode->lex->toString(typePar->at(i)).c_str());
            QString s(this->parentNode->lex->toString(token).c_str());
            this->showError("expected token : " + t +"\nfind token : " + s);
        }
    }
}

QMap<std::string, std::string> Byte_Order::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* Byte_Order::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}