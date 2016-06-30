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

#ifndef DCMFILE_H
#define DCMFILE_H

#include "node.h"
#include "workproject.h"
#include "datacontainer.h"
class Data;

class Dcm : public QObject, public DataContainer
{
    Q_OBJECT

public:
    Dcm(QString fullDcmFileName, WorkProject *parentWP, QString modName, QObject *parent = 0);
    ~Dcm();

    std::string pixmap();
    QString fullName();
    QStringList getListNameData();
    void checkDisplay();
    void addLabelsAsChild();
    bool save(QString fileName = "");
    void setFullName(QString fullName);
    bool readFile();
    int getNumByte(std::string str);

public slots:
    void attach(QObject*o);
    void detach(QObject*o);

private:
    omp_lock_t lock;
    int maxValueProgbar;
    int valueProgBar;
    QList<QObject*> owners;
    PROJECT *a2lProject;
    QString fullPath;
    QString byteOrderCommon;
    QHash<QString,int> nByte;



signals:
    void incProgressBar(int,int);

private slots:
    void checkProgressStream(int);

};

#endif // DCMFILE_H
