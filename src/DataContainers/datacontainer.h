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

#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include "data.h"
#include "node.h"
class WorkProject;

class DataContainer : public Node
{

public:
    DataContainer(WorkProject *parentWP, QString module);
    ~DataContainer();

    WorkProject *getParentWp();
    QString getModuleName();
    void setModuleName(QString moduleName);
    Data *getData(QString str);
    QList<Data*> getModifiedData();
    void resetAllModifiedData();
    void updateChildNodes(Data* data, bool add);
    bool isSortedBySubsets();
    void sortModifiedDataBySubset(bool checked);
    void addModifiedData(Data* data);
    void removeModifiedData(Data* data);
    void editData(Data* data);
    bool exportDataList2Csv(QStringList dataList, QString fileName = "");
    bool exportDataList2Cdf(QStringList dataList, QString fileName = "");
    bool exportDataList2Mat(QStringList dataList, QString fileName = "");
    bool testMonotony(QStringList listLabel);

    QList<Data*> listData;
    QList<Data*> modifiedData;

private:
    bool isSortedBySubset;
    WorkProject *parentWp;
    QString moduleName;


};

#endif // DATACONTAINER_H
