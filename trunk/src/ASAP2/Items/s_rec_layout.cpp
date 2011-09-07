#include "s_rec_layout.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,S_REC_LAYOUT> S_REC_LAYOUT::itemFactory;

S_REC_LAYOUT::S_REC_LAYOUT( Node *parentNode) : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->s_rec_layout.namePar;
    typePar = &gram->s_rec_layout.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"s_rec_layout";
}


S_REC_LAYOUT::~S_REC_LAYOUT()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void S_REC_LAYOUT::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> S_REC_LAYOUT::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* S_REC_LAYOUT::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
