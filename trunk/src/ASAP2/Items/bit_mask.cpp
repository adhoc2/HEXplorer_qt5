#include "bit_mask.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,BIT_MASK> BIT_MASK::itemFactory;

BIT_MASK::BIT_MASK( Node *parentNode) : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->bit_mask.namePar;
    typePar = &gram->bit_mask.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"bit_mask";
}


BIT_MASK::~BIT_MASK()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void BIT_MASK::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> BIT_MASK::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* BIT_MASK::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
