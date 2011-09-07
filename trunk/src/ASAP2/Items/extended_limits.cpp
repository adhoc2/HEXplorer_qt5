#include "extended_limits.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,EXTENDED_LIMITS> EXTENDED_LIMITS::itemFactory;

EXTENDED_LIMITS::EXTENDED_LIMITS(Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->extended_limits.namePar;
    typePar = &gram->extended_limits.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    name = (char*)"EXTENDED_LIMITS";
}

EXTENDED_LIMITS::~EXTENDED_LIMITS()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void EXTENDED_LIMITS::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> EXTENDED_LIMITS::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* EXTENDED_LIMITS::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
