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
#include "Nodes/dbfile.h"
#include <QMessageBox>
#include "qdebug.h"

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

DBFILE::DBFILE(Node *parentNode, QString sqlConnection, QString fullFileName)
    : Node()
{
    //initialize
    optParameters = new QMap<std::string, std::string>;
    fullA2lName = fullFileName;
    this->sqlConnection = sqlConnection;


}

DBFILE::~DBFILE()
{
    delete optParameters;
    delete[] name;
    delete errorList;
    delete lex;
}

void DBFILE::getAsap2Version()
{        
    if (optParameters->contains("ASAP2_VERSION"))
    {
        showError("ASAP2 parser ASAP2_VERSION already defined");
        return;
    }

    std::string str;
    TokenTyp token1 = nextToken();

    if (token1 == Integer)
    {
        str += lex->getLexem();
        TokenTyp token2 =nextToken();
        if (token2 == Integer)
        {
            str += " ";
            str += lex->getLexem();
            optParameters->insert("ASAP2_VERSION", str);
        }
        else
        {
            this->showError("ASAP2 parser : wrong ASAP2 file format at line ???");
        }

    }
    else
    {
        this->showError("ASAP2 parser : wrong ASAP2 file format at line ???");
    }
}

void DBFILE::getA2mlVersion()
{
    if (this->optParameters->contains("A2ML_VERSION"))
    {
        this->showError("ASAP2 parser : A2ML_VERSION already defined");
        return;
    }

    std::string str;
    TokenTyp token1 = nextToken();

    if (token1 == Integer)
    {
        str += lex->getLexem();
        TokenTyp token2 = nextToken();
        if (token2 == Integer)
        {
            str += " ";
            str += lex->getLexem();
            this->optParameters->insert("A2ML_VERSION", str);
        }
        else
        {
            this->showError("ASAP2 parser : wrong ASAP2 file format at line ???");
        }
    }
    else
    {
        this->showError("ASAP2 parser : wrong ASAP2 file format at line ???");
    }

}

std::string DBFILE::pixmap()
{
    return ":/icones/milky_classeur.png";
}

QString DBFILE::fullName()
{
    return fullA2lName;
}

QString DBFILE::getSqlConnection()
{
    return sqlConnection;
}
