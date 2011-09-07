#include "alignment_byte.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,ALIGNMENT_BYTE> ALIGNMENT_BYTE::itemFactory;

ALIGNMENT_BYTE::ALIGNMENT_BYTE(Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->alignment_byte.namePar;
    typePar = &gram->alignment_byte.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    name = (char*)"alignment_byte";
}

ALIGNMENT_BYTE::~ALIGNMENT_BYTE()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void ALIGNMENT_BYTE::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> ALIGNMENT_BYTE::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* ALIGNMENT_BYTE::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
