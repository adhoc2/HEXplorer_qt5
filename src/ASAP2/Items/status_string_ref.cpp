#include "status_string_ref.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,STATUS_STRING_REF> STATUS_STRING_REF::itemFactory;

STATUS_STRING_REF::STATUS_STRING_REF( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->status_string_ref.namePar;
    typePar = &gram->status_string_ref.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"status_string_ref";
}

STATUS_STRING_REF::~STATUS_STRING_REF()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void STATUS_STRING_REF::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> STATUS_STRING_REF::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* STATUS_STRING_REF::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
