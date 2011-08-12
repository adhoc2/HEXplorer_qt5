#include "fix_axis_par.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,FIX_AXIS_PAR> FIX_AXIS_PAR::itemFactory;

FIX_AXIS_PAR::FIX_AXIS_PAR(QTextStream &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->fix_axis_par.namePar;
    typePar = &gram->fix_axis_par.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    name =(char*)"FIX_AXIS_PAR";
}

FIX_AXIS_PAR::~FIX_AXIS_PAR()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void FIX_AXIS_PAR::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
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

QMap<std::string, std::string> FIX_AXIS_PAR::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* FIX_AXIS_PAR::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
