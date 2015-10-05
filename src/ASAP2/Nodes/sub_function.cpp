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

#include "Nodes/sub_function.h"
#include <QMessageBox>
#include <typeinfo>
#include <QHash>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,SUB_FUNCTION> SUB_FUNCTION::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

SUB_FUNCTION::SUB_FUNCTION( Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->sub_function.namePar;
    typePar = &gram->sub_function.typePar;
    factoryOptNode = &gram->sub_function.factoryOptNode;
    factoryOptItem = &gram->sub_function.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    name = (char*)"SUB_FUNCTION";

    //Parse list Identifiers
    TokenTyp token = parseListChar();

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "SUB_FUNCTION")
        {
            //Sort the childNodes
            //std::sort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //std::sort(this->optItems.begin(), this->optItems.end(), itemLessThan);

            //sort the charList
            std::sort(charList.begin(), charList.end());
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd SUB_FUNCTION\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end SUB_FUNCTION\nfind : " + s1 + " " + s2);
    }
}

SUB_FUNCTION::~SUB_FUNCTION()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

QMap<std::string, std::string> *SUB_FUNCTION::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string  SUB_FUNCTION::pixmap()
{
    return ":/icones/CHAR.bmp";
}

TokenTyp SUB_FUNCTION::parseListChar()
{
    TokenTyp token = lex->getNextToken();

    while (token == Identifier)
    {
        charList.append(lex->getLexem());
        token = lex->getNextToken();
    }

    return token;
}

QStringList SUB_FUNCTION::getCharList()
{
    QStringList list;
    foreach(std::string str, charList)
    {
        list.append(str.c_str());
    }
    return list;
}

char* SUB_FUNCTION::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
