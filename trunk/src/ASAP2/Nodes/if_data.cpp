#include "Nodes/if_data.h"
#include "lexer.h"

Factory<Node,IF_DATA> IF_DATA::nodeFactory;

IF_DATA::IF_DATA(QTextStream  &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    parse(in);
}

IF_DATA::~IF_DATA()
{
    delete[] name;
}

void IF_DATA::parse(QTextStream  &in)
{

    TokenTyp token = lex->getNextToken(in);
    if (token == Identifier)
    {
        name = new char[lex->getLexem().length() + 1];
        strcpy(name, lex->getLexem().c_str());
    }
    else
    {
        QString s(lex->toString(token).c_str());
        this->showError("expected token : Identifier\nfind token : " + s);
    }
    this->a2lLine = lex->getLine();

    token = lex->getNextToken(in);

    while (lex->getLexem() != "IF_DATA")
    {
        token = lex->getNextToken(in);
    }
}
