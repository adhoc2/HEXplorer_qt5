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

#include <QApplication>
#ifdef Q_WS_WIN32
    #include "usewinhoard.cpp"
#endif
#include "mdimain.h"
#include <bitset>
#include <stdlib.h>
#include <stdint.h>
#include "qdebug.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Ch.Hoël");
    app.setApplicationName("HEXplorer");
    app.setApplicationVersion("0.6.11");

    MDImain w;
    w.show();
    return app.exec();
}
