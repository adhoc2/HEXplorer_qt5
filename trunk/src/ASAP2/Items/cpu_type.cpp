#include "cpu_type.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,CPU_TYPE> CPU_TYPE::itemFactory;

CPU_TYPE::CPU_TYPE( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->cpu_type.namePar;
    typePar = &gram->cpu_type.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"cpu_type";
}

CPU_TYPE::~CPU_TYPE()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void CPU_TYPE::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> CPU_TYPE::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* CPU_TYPE::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
