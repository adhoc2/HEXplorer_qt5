#include "Nodes/sub_function.h"
#include <QMessageBox>
#include <typeinfo>
#include <QHash>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,SUB_FUNCTION> SUB_FUNCTION::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

SUB_FUNCTION::SUB_FUNCTION(QTextStream &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->sub_function.namePar;
    typePar = &gram->sub_function.typePar;
    factoryOptNode = &gram->sub_function.factoryOptNode;
    factoryOptItem = &gram->sub_function.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    name = (char*)"SUB_FUNCTION";

    //Parse list Identifiers
    TokenTyp token = parseListChar(in);

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken(in);
        if (token == Keyword && lex->getLexem() == "SUB_FUNCTION")
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
            this->showError("expected token : BlockEnd SUB_FUNCTION\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end SUB_FUNCTION\nfind : " + s1 + " " + s2);
    }
}

SUB_FUNCTION::~SUB_FUNCTION()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

QMap<std::string, std::string> *SUB_FUNCTION::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string  SUB_FUNCTION::pixmap()
{
    return ":/icones/CHAR.bmp";
}

TokenTyp SUB_FUNCTION::parseListChar(QTextStream &in)
{
    TokenTyp token = lex->getNextToken(in);

    while (token == Identifier)
    {
        charList.append(lex->getLexem());
        token = lex->getNextToken(in);
    }

    return token;
}

QStringList SUB_FUNCTION::getCharList()
{
    QStringList list;
    foreach(std::string str, charList)
    {
        list.append(str.c_str());
    }
    return list;
}

char* SUB_FUNCTION::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
