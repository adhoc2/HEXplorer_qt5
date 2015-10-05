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

#include "Nodes/calibration_method.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,CALIBRATION_METHOD> CALIBRATION_METHOD::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

CALIBRATION_METHOD::CALIBRATION_METHOD(Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->calibration_method.namePar;
    typePar = &gram->calibration_method.typePar;
    factoryOptNode = &gram->calibration_method.factoryOptNode;
    factoryOptItem = &gram->calibration_method.factoryOptItem;

    //opt parameters
    occOptPar = new QMap<std::string, Occurence>;
    occOptPar->insert("CALIBRATION_HANDLE", ZeroOrMore);

    //Set the line where the Node starts in ASAP file
    this->a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"calibration_method";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar(occOptPar);

    //End
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "CALIBRATION_METHOD")
        {
            //Sort the childNodes
            //std::sort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //std::sort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd CALIBRATION_METHOD\nfind token : " + s );
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end CALIBRATION_METHOD\nfind : " + s1 + " " + s2 );
    }    
}

CALIBRATION_METHOD::~CALIBRATION_METHOD()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
    delete occOptPar;
}

void CALIBRATION_METHOD::parseFixPar(QList<TokenTyp> *typePar)
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
        else
        {
            QString t(lex->toString(typePar->at(i)).c_str());
            QString s(lex->toString(token).c_str());
            this->showError("expected token : " + t +"\nfind token : " + s );
        }
    }
}

TokenTyp CALIBRATION_METHOD::parseOptPar(QMap<std::string, Occurence> *nameOptPar)
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
                        if (this->occOptPar->value(lexem) == ZeroOrOne)
                        {
                            this->occOptPar->insert(lexem, Zero);
                            Node  *instance = factoryOptNode->value(lexem)->createInstance( this);
                            this->addChildNode(instance);
                            token = nextToken();
                        }
                        else if (this->occOptPar->value(lexem) == ZeroOrMore)
                        {
                            Node  *instance = factoryOptNode->value(lexem)->createInstance( this);
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
                    if (this->occOptPar->value(lexem) == ZeroOrOne)
                    {
                        this->occOptPar->insert(lexem, Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else if (this->occOptPar->value(lexem) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
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

QMap<std::string, std::string> *CALIBRATION_METHOD::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* CALIBRATION_METHOD::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
