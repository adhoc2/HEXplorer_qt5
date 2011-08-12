#include "workproject.h"
#include <Nodes/a2lfile.h>
#include "a2lgrammar.h"
#include "csv.h"
#include "cdfxfile.h"
#include "qdebug.h"

WorkProject::WorkProject(QString fullFileName, A2lTreeModel *model, MDImain *parent)
    :  A2l(fullFileName, parent)
{

    treeModel = model;
    parentWidget = parent;
}

WorkProject::~WorkProject()
{
}

void WorkProject::attach(QObject *o)
{
    //check owner for validity
    if(o==0)return;

    //check for duplicates
    if(owners.contains(o))return;

    //register
    owners.append(o);
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(detach(QObject*)));

}

void WorkProject::detach(QObject *o)
{
    //remove it
    owners.removeAll(o);

    //remove self after last one
    if(owners.size()==0)
        delete this;
}

QMap<QString, HexFile*> WorkProject::hexFiles()
{
    return hexList;
}

QMap<QString, Csv*> WorkProject::csvFiles()
{
    return csvList;
}

QMap<QString, CdfxFile*> WorkProject::cdfxFiles()
{
    return cdfxList;
}

void WorkProject::addHex(HexFile *hex )
{
    // add hex to a2lfile childrenslist
    a2lFile->addChildNode(hex);
    a2lFile->sortChildrensName();

    // update treeView
    treeModel->dataInserted(a2lFile, a2lFile->childNodes.indexOf(hex));

    // add hex to this hexList
    hexList.insert(hex->fullName(), hex);

    // add this to the hex owners (pseudo garbage collector)
    hex->attach(this);
}

void WorkProject::addCsv(Csv *csv )
{
    // add csv to a2lfile childrenslist
    a2lFile->addChildNode(csv);
    a2lFile->sortChildrensName();

    // update treeView
    treeModel->dataInserted(a2lFile, a2lFile->childNodes.indexOf(csv));

    // add csv to this hexList
    csvList.insert(csv->fullName(), csv);

    // add this to the csv owners
    csv->attach(this);

}

void WorkProject::addCdfx(CdfxFile *cdfx)
{
    // add csv to a2lfile childrenslist
    a2lFile->addChildNode(cdfx);
    a2lFile->sortChildrensName();

    // update treeView
    treeModel->dataInserted(a2lFile, a2lFile->childNodes.indexOf(cdfx));

    // add cdfx to this hexList
    cdfxList.insert(cdfx->fullName(), cdfx);

    // add this to the cdfx owners
    cdfx->attach(this);
}

void WorkProject::removeHexFile(HexFile *hex)
{
    hexList.remove(hex->fullName());

    // remove this Wp to the csv owners
    hex->detach(this);
}

void WorkProject::removeCsv(Csv *csv )
{
    csvList.remove(csv->fullName());

    // remove this Wp to the csv owners
    csv->detach(this);

}

void WorkProject::removeCdfxFile(CdfxFile *cdfx)
{
    cdfxList.remove(cdfx->fullName());

    // remove this Wp to the csv owners
    cdfx->detach(this);

}

void WorkProject::rename(HexFile *hex)
{
    QString key = hexList.key(hex);
    hexList.remove(key);
    hexList.insert(hex->fullName(), hex);
}

void WorkProject::rename(Csv *csv)
{
    QString key = csvList.key(csv);
    csvList.remove(key);
    csvList.insert(csv->fullName(), csv);
}

void WorkProject::rename(CdfxFile *cdfx)
{
    QString key = cdfxList.key(cdfx);
    cdfxList.remove(key);
    cdfxList.insert(cdfx->fullName(), cdfx);
}

HexFile* WorkProject::getHex(QString str)
{
    foreach (HexFile* hex, hexList.values())
    {
        if (hex->fullName() == str)
        {
            return hex;
        }
    }

    return NULL;
}

QStringList WorkProject::getHexList()
{
    return hexList.keys();
}

QString WorkProject::toString()
{
    QString str = "WorkProject* (" + QString(a2lFile->name) + " )" ;
    return str;
}
