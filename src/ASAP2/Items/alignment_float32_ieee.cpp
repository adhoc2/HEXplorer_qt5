#include "alignment_float32_ieee.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,ALIGNMENT_FLOAT32_IEEE> ALIGNMENT_FLOAT32_IEEE::itemFactory;

ALIGNMENT_FLOAT32_IEEE::ALIGNMENT_FLOAT32_IEEE( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->alignment_float32_ieee.namePar;
    typePar = &gram->alignment_float32_ieee.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    name = (char*)"alignment_float32_ieee";
}

ALIGNMENT_FLOAT32_IEEE::~ALIGNMENT_FLOAT32_IEEE()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void ALIGNMENT_FLOAT32_IEEE::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> ALIGNMENT_FLOAT32_IEEE::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* ALIGNMENT_FLOAT32_IEEE::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
