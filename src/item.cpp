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

#include "item.h"
#include "node.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

Item::Item(Node *parentNod)
{
    parentNode = parentNod;
}

Item::~Item()
{
}


TokenTyp Item::nextToken()
{
    TokenTyp token = parentNode->lex->getNextToken();

    while (token == Comment)
    {
        token = parentNode->lex->getNextToken();
    }

    return token;
}


void Item::showError(QString str)
{
    QString s;
    s.setNum(this->parentNode->lex->getLine());
    QString name = typeid(*this).name();

    parentNode->errorList->append(name + " parser : " + "wrong ASAP2 file format at line " + s + " :");
    parentNode->errorList->append(str);
}

