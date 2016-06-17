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

#ifndef CDFXFILE_H
#define CDFXFILE_H

#include <QObject>
#include <QDomDocument>

#include "node.h"
#include "datacontainer.h"
#include "workproject.h"

QT_BEGIN_NAMESPACE
class QDomNode;
class QFile;
QT_END_NAMESPACE

class Data;

class SwInstance
{
    public :
            bool isValid;
            QString parseError;

            QString name;
            QString description;
            QString category;
            QString subset;
            QString unit;
            QString arraySize;

            QStringList Zvalues;

            QString axisXtype;
            QString axisXunit;
            QStringList axisXvalues;
            QString axisXreference;

            QString axisYtype;
            QString axisYunit;
            QStringList axisYvalues;
            QString axisYreference;

            QString axisZtype;
            QString axisZunit;
            QStringList axisZvalues;
            QString axisZreference;

            bool isConsistentWith(Data *data);            
};

class CdfxFile : public QObject, public DataContainer
{
    Q_OBJECT

public:
    explicit CdfxFile(QString fullCsvFileName, WorkProject *parentWP, QString modName, QObject *parent = 0);
    ~CdfxFile();

    std::string pixmap();
    QString fullName();
    QStringList getListNameData();
    QStringList getErrorList();
    QStringList getInfoList();
    void checkDisplay();
    void addLabelsAsChild();
    bool save(QString fileName = "");
    void setFullName(QString fullName);
    bool isRead;
    int getNumByte(std::string str);

public slots:
    void attach(QObject*o);
    void detach(QObject*o);

private:
    QStringList errorList;
    QStringList infoList;
    QDomDocument document;
    QList<QObject*> owners;
    PROJECT *a2lProject;
    QString fullPath;
    QList<SwInstance*> listSwInstance;
    QString byteOrderCommon;
    QHash<QString,int> nByte;

    SwInstance *getSwInstance(QString str);
    void checkConsistency(Data* data, SwInstance* instance);
    void swInstance2Data();
    bool readFile();
    void updateChildNodes(Data* data, bool add);
    bool validate(QFile *file);
    void parseMSRSW(QDomNode &node);
    void parseSwSystem(QDomNode &node);
    void parseSwInstanceTree(QDomNode &node);
    void parseVG(QDomNode &node, QStringList &list);
    void parseSwInstance(QDomNode &node);
    void parseSwValueCont(QDomNode &node, SwInstance *instance);
    void parseSwAxisCont(QDomNode &node, SwInstance *instance, int i);

public slots:

};

#endif // CDFXFILE_H
