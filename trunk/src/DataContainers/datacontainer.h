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
