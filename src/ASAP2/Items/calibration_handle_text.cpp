#include "calibration_handle_text.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,CALIBRATION_HANDLE_TEXT> CALIBRATION_HANDLE_TEXT::itemFactory;

CALIBRATION_HANDLE_TEXT::CALIBRATION_HANDLE_TEXT(QTextStream &in, Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->calibration_handle_text.namePar;
    typePar = &gram->calibration_handle_text.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"calibration_handle_text";
}

CALIBRATION_HANDLE_TEXT::~CALIBRATION_HANDLE_TEXT()
{

    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void CALIBRATION_HANDLE_TEXT::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
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

QMap<std::string, std::string> CALIBRATION_HANDLE_TEXT::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* CALIBRATION_HANDLE_TEXT::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}

