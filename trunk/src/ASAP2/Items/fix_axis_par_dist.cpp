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

#include "fix_axis_par_dist.h"
#include <QMessageBox>
#include "a2lgrammar.h"

//initialise static variables
Factory<Item,FIX_AXIS_PAR_DIST> FIX_AXIS_PAR_DIST::itemFactory;

FIX_AXIS_PAR_DIST::FIX_AXIS_PAR_DIST( Node *parentNode)  : Item(parentNode)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->fix_axis_par_dist.namePar;
    typePar = &gram->fix_axis_par_dist.typePar;

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    name =(char*)"FIX_AXIS_PAR_DIST";
}

FIX_AXIS_PAR_DIST::~FIX_AXIS_PAR_DIST()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void FIX_AXIS_PAR_DIST::parseFixPar(QList<TokenTyp> *typePar)
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

QMap<std::string, std::string> FIX_AXIS_PAR_DIST::getParameters()
{
    QMap<std::string, std::string> par;
    for (int i = 0; i < namePar->count(); i++)
    {
        par.insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* FIX_AXIS_PAR_DIST::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
