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

#ifndef A2LFILE_H
#define A2LFILE_H

#include "node.h"
#include "lexer.h"
#include <QMap>
#include <QPixmap>
#include "Nodes/project.h"


class WorkProject;
class A2lGrammar;

class A2LFILE : public Node
{
    public:
        A2LFILE(QString fullFileName);
        A2LFILE(Node *parentNode, A2lLexer *lexer,
                QStringList *errorList, QString fullFileName = "");       
        ~A2LFILE();

        std::string pixmap();
        QString fullName();
        PROJECT *getProject();
        bool isConform();
        bool isParsed();

    private:
        PROJECT *project; //ASAP2 file must contain exactly one PROJECT
        QString fullA2lName;
        void parser();
        void getAsap2Version();
        void getA2mlVersion();
        bool parsed;

        QMap<std::string, std::string> *optParameters;

};

#endif // BLOCKA2LFILE_H
