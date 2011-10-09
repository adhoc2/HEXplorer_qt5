// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
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

#include "Nodes/compu_vtab.h"
#include "lexer.h"
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,COMPU_VTAB> COMPU_VTAB::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

COMPU_VTAB::COMPU_VTAB(Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->compuVtab.namePar;
    typePar = &gram->compuVtab.typePar;
    factoryOptNode = &gram->compuVtab.factoryOptNode;
    factoryOptItem = &gram->compuVtab.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"compu_vtab";

    //special for compu_vtab
    parsePairs();

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar();

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "COMPU_VTAB")
        {
            //Sort the childNodes
            //qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd COMPU_VTAB\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end COMPU_VTAB\nfind : " + s1 + " " + s2);
    }
}

COMPU_VTAB::~COMPU_VTAB()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void COMPU_VTAB::parseFixPar(QList<TokenTyp> *typePar)
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
            this->showError("expected token : " + t +"\nfind token : " + s);
        }
    }
}

TokenTyp COMPU_VTAB::parseOptPar()
{
    //opt parameters
    QMap<std::string, Occurence> nameOptPar;
    nameOptPar.insert("DEFAULT_VALUE", ZeroOrOne);

    if (nameOptPar.isEmpty())
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
                    if (nameOptPar.contains(lexem))
                    {
                        if (nameOptPar.value(lexem) == ZeroOrOne)
                        {
                           nameOptPar.insert(lexem, Zero);
                            Node  *instance = factoryOptNode->value(lexem)->createInstance(this);
                            this->addChildNode(instance);
                            token = nextToken();
                        }
                        else if (nameOptPar.value(lexem) == ZeroOrMore)
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
                if (nameOptPar.contains(lexem))
                {
                    if (nameOptPar.value(lexem) == ZeroOrOne)
                    {
                        nameOptPar.insert(lexem, Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else if (nameOptPar.value(lexem) == ZeroOrMore)
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

QMap<std::string, std::string> *COMPU_VTAB::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }

    for (int i = 0; i < valuePairs.count(); i++)
    {
        QString str;
        str.setNum(valuePairs.keys().at(i));
		par->insert(str.toLocal8Bit().data(), valuePairs.values().at(i));
    }
    return par;
}

std::string  COMPU_VTAB::pixmap()
{
    return ":/icones/CHAR.bmp";
}

void COMPU_VTAB::parsePairs()
{
    QString str = parameters.at(3);
    bool bl = false;
    int count = str.toInt(&bl, 10);

    TokenTyp token;
    int key = 0;
    for (int i = 0; i < count; i++)
    {
        token = lex->getNextToken();
        if (token == Integer)
        {
           QString str = lex->getLexem().c_str();
           key = str.toInt(&bl,10);
           listKeyPairs.append(key);
           token = lex->getNextToken();
           if (token == String)
           {
               QString value = lex->getLexem().c_str();
               value.remove("\"");
              //valuePairs.insert(key, lex->getLexem());
              //listValuePairs.append(lex->getLexem());
               valuePairs.insert(key, value.toStdString());
               listValuePairs.append(value.toStdString());
           }
           else
           {
                QString s(lex->toString(token).c_str());
                showError("expected token : String \nfind token : " + s);
           }
        }
        else
        {
            QString s(lex->toString(token).c_str());
            showError("expected token : Ineger \nfind token : " + s);
        }
    }
}

QString COMPU_VTAB::getValue(int i)
{
    /*
    if (valuePairs.keys().contains(i))
    {
        QString str = "";
        #pragma omp critical
        str = valuePairs.value(i).c_str();

        return str;
    }
    else
    {
        return "ERROR";
    }
    */

    int ind = listKeyPairs.indexOf(i);
    if (ind >= 0)
        return listValuePairs.at(ind).c_str();
    else
        return "ERROR";

}

int COMPU_VTAB::getPos(QString str)
{
   int ind =  listValuePairs.indexOf(str.toLocal8Bit().data());
   if (ind >= 0)
       return listKeyPairs.at(ind);
   else
       return -1;
}

char* COMPU_VTAB::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}

QStringList COMPU_VTAB::getValueList()
{
    QStringList list;
    foreach (std::string str, listValuePairs)
    {
        list.append(str.c_str());
    }

    return list;
}
