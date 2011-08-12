#include "ref_unit.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,REF_UNIT> REF_UNIT::itemFactory;

REF_UNIT::REF_UNIT(QTextStream &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->ref_unit.namePar;
    typePar = &gram->ref_unit.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"ref_unit";
}

REF_UNIT::~REF_UNIT()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void REF_UNIT::parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in)
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

QMap<std::string, std::string> REF_UNIT::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* REF_UNIT::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
