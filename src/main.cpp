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
#ifdef Q_OS_WIN32
#ifdef CL_COMPILER
   //#include "uselibhoard.cpp"
#endif
#endif
#include "mdimain.h"
#include <QTextCodec>
#include "qdebug.h"


int main(int argc, char *argv[])
{
    Qt::AA_UseSoftwareOpenGL;
    QApplication app(argc, argv);
    QByteArray encodedString = "Ch.Hoël";
//    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-1");
//    QString string = codec->toUnicode(encodedString);
    app.setOrganizationName(encodedString);
    app.setApplicationName("HEXplorer");
    app.setApplicationVersion("0.7.8");


    MDImain w;
    w.show();
    return app.exec();
}

