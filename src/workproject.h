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

#ifndef WORKPROJECT_H
#define WORKPROJECT_H

#include <a2l.h>
#include <hexfile.h>
#include "srecfile.h"
#include "a2ltreemodel.h"
#include "mdimain.h"
class Csv;
class CdfxFile;
class Node;

class WorkProject :  public A2l, public Node
{
    Q_OBJECT

    public:
        WorkProject(QString fullFileName, A2lTreeModel *model = NULL, MDImain *parent = 0);
        ~WorkProject();

        void attach(QObject*o);

        QMap<QString, HexFile*> hexFiles();
        void addHex(HexFile *hex);
        void removeHexFile(HexFile *hex);
        void rename(HexFile *hex);
        bool containsHex(QString str);

        QMap<QString, SrecFile*> srecFiles();
        void addSrec(SrecFile *srec);
        void removeSrecFile(SrecFile *srec);
        void rename(SrecFile *srec);
        bool containsSrec(QString str);

        QMap<QString, Csv*> csvFiles();
        void addCsv(Csv *csv);
        void removeCsv(Csv *csv );
        void rename(Csv *csv);
        bool containsCsv(QString str);

        QMap<QString, CdfxFile*> cdfxFiles();
        void addCdfx(CdfxFile *cdfx);
        void removeCdfxFile(CdfxFile *cdfx );
        void rename(CdfxFile *cdfx);
        bool containsCdfx(QString str);

        A2lTreeModel *treeModel;
        MDImain *parentWidget;
        QString getFullNodeName();
        std::string pixmap();

    private:
        QList<QObject*> owners;
        QMap<QString, HexFile*> hexList;
        QMap<QString, SrecFile*> srecList;
        QMap<QString, Csv*> csvList;
        QMap<QString, CdfxFile*> cdfxList;

    public slots:
        void detach(QObject*o);
        HexFile* getHex(QString str);
        QStringList getHexList();
        SrecFile* getSrec(QString str);
        QStringList getSrecList();
        QString toString();

};

#endif // WORKPROJECT_H
