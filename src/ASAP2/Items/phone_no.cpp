#include "phone_no.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,PHONE_NO> PHONE_NO::itemFactory;

PHONE_NO::PHONE_NO(QTextStream &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->phone_no.namePar;
    typePar = &gram->phone_no.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"phone_no";
}

PHONE_NO::~PHONE_NO()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void PHONE_NO::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in )
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

QMap<std::string, std::string> PHONE_NO::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* PHONE_NO::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
