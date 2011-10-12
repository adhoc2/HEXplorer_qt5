// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

#include "Nodes/if_data.h"
#include "lexer.h"

Factory<Node,IF_DATA> IF_DATA::nodeFactory;

IF_DATA::IF_DATA(Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    parse();
}

IF_DATA::~IF_DATA()
{
    delete[] name;
}

void IF_DATA::parse()
{

    TokenTyp token = lex->getNextToken();
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

    token = lex->getNextToken();

    while (lex->getLexem() != "IF_DATA")
    {
        token = lex->getNextToken();
    }
}
