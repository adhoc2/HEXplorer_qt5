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
#include "Nodes/a2lfile.h"
#include "workproject.h"
#include <QMessageBox>
#include "a2lgrammar.h"
#include "qdebug.h"

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

A2LFILE::A2LFILE(Node *parentNode, A2lLexer *lexer, QStringList *errorList, QString fullFileName)
    : Node(parentNode, lexer, errorList)
{
    //initialize
    project = NULL;
    optParameters = new QMap<std::string, std::string>;
    fullA2lName = fullFileName;

    //call the parser
    parser();
}

A2LFILE::~A2LFILE()
{
    delete optParameters;
    delete[] name;
    delete errorList;
    delete lex;
}

void A2LFILE::parser()
{
    TokenTyp token = nextToken();
    while (token == Keyword)
    {
        if (lex->getLexem() == "ASAP2_VERSION")
            getAsap2Version();
        else if (lex->getLexem() == "A2ML_VERSION")
            getA2mlVersion();
        else
        {
            QString s1(lex->toString(token).c_str());
            QString s2(lex->getLexem().c_str());
            QString s3 = QString::number(lex->getLine());

            showError("ASAP2 parser : wrong ASAP2 file format at line " + s3 + "\n"
                                    "expected token : Keyword (ASAP2_VERSION or A2ML_VERSION)\n"
                                    "find token : " + s1 + " (" + s2 + ")");
            return;
        }
        token = nextToken();
    }

    if (token == BlockBegin)
    {
        TokenTyp token1 = nextToken();

        if (token1 == Keyword && lex->getLexem() == "PROJECT")
        {
            PROJECT *child = new PROJECT(this, lex);
            addChildNode(child);
            project = child;

           // token = nextToken();
        }
        else
        {
            QString s1(lex->toString(token1).c_str());
            QString s2(lex->getLexem().c_str());

            showError("ASAP2 parser : wrong ASAP2 file format at line ???\n"
                                    "expected token : Identifier (PROJECT)\n"
                                    "find token : " + s1 + " (" + s2 + ")");
        }
    }
    else
    {
        QString s(lex->toString(token).c_str());

        this->showError("ASAP2 parser : wrong ASAP2 file format at line ???\n"
                                "expected token : BlockBegin or Keyword\n"
                                 "find token : " + s);
    }
}

void A2LFILE::getAsap2Version()
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

void A2LFILE::getA2mlVersion()
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

bool A2LFILE::isConform()
{
    if (project)
    {
        QList<MODULE*> list = project->listModule();
        if (list.isEmpty())
        {
            return false;
        }
        else
            return true;
    }
    else
    {
        return false;
    }
}

std::string A2LFILE::pixmap()
{
    return ":/icones/milky_cartable.png";
}

QString A2LFILE::fullName()
{
    return fullA2lName;
}

PROJECT *A2LFILE::getProject()
{
    return project;
}
