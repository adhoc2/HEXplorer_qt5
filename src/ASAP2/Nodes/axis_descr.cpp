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

#include "Nodes/axis_descr.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,AXIS_DESCR> AXIS_DESCR::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

AXIS_DESCR::AXIS_DESCR( Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->axis_descr.namePar;
    typePar = &gram->axis_descr.typePar;
    factoryOptNode = &gram->axis_descr.factoryOptNode;
    factoryOptItem = &gram->axis_descr.factoryOptItem;

    //optional Parameters
    occOptPar = new QHash<QString, Occurence>;
    occOptPar->insert("FORMAT", ZeroOrOne);
    occOptPar->insert("EXTENDED_LIMITS", ZeroOrOne);  
    occOptPar->insert("DEPOSIT", ZeroOrOne);
    occOptPar->insert("AXIS_PTS_REF", ZeroOrOne);
    occOptPar->insert("FIX_AXIS_PAR", ZeroOrOne);
    occOptPar->insert("READ_ONLY", ZeroOrOne);
    occOptPar->insert("FIX_AXIS_PAR_DIST", ZeroOrOne);
    occOptPar->insert("BYTE_ORDER", ZeroOrOne);

    //Set the line where the Node starts in ASAP file
    this->a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"axis_descr";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar(occOptPar);

    //End
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "AXIS_DESCR")
        {
            //Sort the childNodes
            //std::sort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //std::sort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd AXIS_DESCR\nfind token : " + s );
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end AXIS_DESCR\nfind : " + s1 + " " + s2 );
    }    
}

AXIS_DESCR::~AXIS_DESCR()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
    delete occOptPar;
}

void AXIS_DESCR::parseFixPar(QList<TokenTyp> *typePar)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken();
        if (token == typePar->at(i))
        {
            //parameters.insert(namePar->at(i), lex->getLexem());
            char *c = new char[lex->getLexem().length()+1];
            strcpy(c, lex->getLexem().c_str());
            parameters.append(c);
        }
        else if(typePar->at(i) == Float && token == Integer)
        {
            char *c = new char[parentNode->lex->getLexem().length()+1];
            strcpy(c, parentNode->lex->getLexem().c_str());
            parameters.append(c);
        }
        else
        {
            QString t(lex->toString(typePar->at(i)).c_str());
            QString s(lex->toString(token).c_str());
            this->showError("expected token : " + t +"\nfind token : " + s );
        }
    }
}

TokenTyp AXIS_DESCR::parseOptPar(QHash<QString, Occurence> *nameOptPar)
{

    if (nameOptPar->isEmpty())
        return nextToken();
    else
    {
        TokenTyp token = nextToken();
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = this->nextToken();
                if (token == Keyword)
                {
                    std::string lexem = lex->getLexem();
                    if (factoryOptNode->contains(lexem))
                    {
                        if (this->occOptPar->value(lexem.c_str()) == ZeroOrOne)
                        {
                            this->occOptPar->insert(lexem.c_str(), Zero);
                            Node  *instance = factoryOptNode->value(lexem.c_str())->createInstance(this);
                            this->addChildNode(instance);
                            token = nextToken();
                        }
                        else if (this->occOptPar->value(lexem.c_str()) == ZeroOrMore)
                        {
                            Node  *instance = factoryOptNode->value(lexem.c_str())->createInstance( this);
                            this->addChildNode(instance);
                            token = nextToken();
                        }
                        else
                        {
                            QString s(lexem.c_str());
                            this->showError(" Keyword : " + s + " can only be once declared");
                            return token;
                        }
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        this->showError("unknown Keyword : " + s );
                        return token;
                    }
                }
                else
                {
                    QString s(lex->toString(token).c_str());
                    this->showError("expected token : BlockBegin or Keyword\nfind token : " + s );
                    return token;
                }
            }
            //Items
            else if (token == Keyword)
            {
                std::string lexem = lex->getLexem();
                if (factoryOptItem->contains(lexem))
                {
                    if (this->occOptPar->value(lexem.c_str()) == ZeroOrOne)
                    {
                        this->occOptPar->insert(lexem.c_str(), Zero);
                        Item  *instance = factoryOptItem->value(lexem.c_str())->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else if (this->occOptPar->value(lexem.c_str()) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem.c_str())->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        this->showError(" Keyword : " + s + " can only be once declared");
                        return token;
                    }
                }
                else
                {
                    QString s(lexem.c_str());
                    this->showError("unknown Keyword : " + s );
                    return token;
                }
            }
        }
        return token;
    }
}

QMap<std::string, std::string> *AXIS_DESCR::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* AXIS_DESCR::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
