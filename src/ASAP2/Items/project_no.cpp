#include "project_no.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,PROJECT_NO> PROJECT_NO::itemFactory;

PROJECT_NO::PROJECT_NO(QTextStream &in, Node *parentNode) : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->project_no.namePar;
    typePar = &gram->project_no.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"project_no";
}


PROJECT_NO::~PROJECT_NO()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void PROJECT_NO::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
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

QMap<std::string, std::string> PROJECT_NO::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* PROJECT_NO::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
