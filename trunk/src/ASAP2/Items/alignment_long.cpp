#include "alignment_long.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,ALIGNMENT_LONG> ALIGNMENT_LONG::itemFactory;

ALIGNMENT_LONG::ALIGNMENT_LONG(QTextStream &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->alignment_long.namePar;
    typePar = &gram->alignment_long.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    name = (char*)"alignment_long";
}

ALIGNMENT_LONG::~ALIGNMENT_LONG()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void ALIGNMENT_LONG::parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in)
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

QMap<std::string, std::string> ALIGNMENT_LONG::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* ALIGNMENT_LONG::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
