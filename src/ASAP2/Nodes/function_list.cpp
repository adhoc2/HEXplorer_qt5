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

#include "Nodes/function_list.h".h"
#include <QMessageBox>
#include <typeinfo>
#include <QHash>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,FUNCTION_LIST> FUNCTION_LIST::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

FUNCTION_LIST::FUNCTION_LIST(Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->function_list.namePar;
    typePar = &gram->function_list.typePar;
    factoryOptNode = &gram->function_list.factoryOptNode;
    factoryOptItem = &gram->function_list.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    //parseFixPar(typePar, namePar ,in);
    name = (char*)"FUNCTION_LIST";

    //Parse optional PARAMETERS
    //TokenTyp token = parseOptPar(in);

    TokenTyp token = parseListChar();

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "FUNCTION_LIST")
        {
            //Sort the childNodes
            //std::sort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //std::sort(this->optItems.begin(), this->optItems.end(), itemLessThan);

            //sort the charList
            std::sort(subsetList.begin(), subsetList.end());
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd FUNCTION_LIST\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end FUNCTION_LIST\nfind : " + s1 + " " + s2);
    }
}

FUNCTION_LIST::~FUNCTION_LIST()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

QMap<std::string, std::string> *FUNCTION_LIST::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string  FUNCTION_LIST::pixmap()
{
    return ":/icones/CHAR.bmp";
}

TokenTyp FUNCTION_LIST::parseListChar()
{
    TokenTyp token = lex->getNextToken();

    while (token == Identifier)
    {
        subsetList.append(lex->getLexem());
        token = lex->getNextToken();
    }

    return token;
}

QStringList FUNCTION_LIST::getCharList()
{
    QStringList list;
    foreach(std::string str, subsetList)
    {
        list.append(str.c_str());
    }
    return list;
}

char* FUNCTION_LIST::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
