#include "Nodes/def_characteristic.h"
#include <QMessageBox>
#include <typeinfo>
#include <QHash>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,DEF_CHARACTERISTIC> DEF_CHARACTERISTIC::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

DEF_CHARACTERISTIC::DEF_CHARACTERISTIC(QTextStream &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->def_characteristic.namePar;
    typePar = &gram->def_characteristic.typePar;
    factoryOptNode = &gram->def_characteristic.factoryOptNode;
    factoryOptItem = &gram->def_characteristic.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    //parseFixPar(typePar, namePar ,in);
    name = (char*)"DEF_CHARACTERISTIC";

    //Parse optional PARAMETERS
    //TokenTyp token = parseOptPar(in);

    TokenTyp token = parseListChar(in);

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken(in);
        if (token == Keyword && lex->getLexem() == "DEF_CHARACTERISTIC")
        {
            //Sort the childNodes
            //qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);

            //sort the charList
            qSort(charList.begin(), charList.end());
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd DEF_CHARACTERISTIC\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end DEF_CHARACTERISTIC\nfind : " + s1 + " " + s2);
    }
}

DEF_CHARACTERISTIC::~DEF_CHARACTERISTIC()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

QMap<std::string, std::string> *DEF_CHARACTERISTIC::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string  DEF_CHARACTERISTIC::pixmap()
{
    return ":/icones/CHAR.bmp";
}

TokenTyp DEF_CHARACTERISTIC::parseListChar(QTextStream &in)
{
    TokenTyp token = lex->getNextToken(in);

    while (token == Identifier)
    {
        charList.append(lex->getLexem());
        token = lex->getNextToken(in);
    }

    return token;
}

QStringList DEF_CHARACTERISTIC::getCharList()
{
    QStringList list;
    foreach(std::string str, charList)
    {
        list.append(str.c_str());
    }
    return list;
}

char* DEF_CHARACTERISTIC::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
