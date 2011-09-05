#include "format.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,FORMAT> FORMAT::itemFactory;

FORMAT::FORMAT(QTextStream  &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->format.namePar;
    typePar = &gram->format.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    name = (char*)"FORMAT";
}

FORMAT::~FORMAT()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void FORMAT::parseFixPar(QList<TokenTyp> *typePar,
                          QTextStream  &in)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken(in);
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

QMap<std::string, std::string> FORMAT::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* FORMAT::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}