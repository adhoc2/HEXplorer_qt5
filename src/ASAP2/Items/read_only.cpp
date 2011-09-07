#include "read_only.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,READ_ONLY> READ_ONLY::itemFactory;

READ_ONLY::READ_ONLY( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->read_only.namePar;
    typePar = &gram->read_only.typePar;

    //Parse Mandatory PARAMETERS
    //parseFixPar(typePar, namePar ,in, parentWidget);
    //name = parameters->value("READ_ONLY");
    name = (char*)"";
}

READ_ONLY::~READ_ONLY()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void READ_ONLY::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> READ_ONLY::getParameters()
{    
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* READ_ONLY::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
