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

#ifndef DBFILE_H
#define DBFILE_H

#include "node.h"
#include <QMap>
#include <QPixmap>



class DBFILE : public Node
{
    public:
        DBFILE(Node *parentNode, QString sqlConnection, QString fullFileName = "" );
        ~DBFILE();

        std::string pixmap();
        QString fullName();
        QString getSqlConnection();

    private:
        QString fullA2lName;
        void getAsap2Version();
        void getA2mlVersion();
        QString sqlConnection;

        QMap<std::string, std::string> *optParameters;

};

#endif // BLOCKDBFILE_H
