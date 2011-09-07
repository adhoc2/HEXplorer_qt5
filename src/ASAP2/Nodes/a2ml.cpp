#include "Nodes/a2ml.h"
#include "lexer.h"

Factory<Node, A2ML> A2ML::nodeFactory;

A2ML::A2ML( Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    this->parse();
}

void A2ML::parse()
{

    this->name = (char*)"A2ML";
    this->a2lLine = lex->getLine();

    TokenTyp token = lex->getNextToken();

    while (lex->getLexem() != "A2ML")
    {
        token = lex->getNextToken();
    }
}

