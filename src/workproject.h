#ifndef WORKPROJECT_H
#define WORKPROJECT_H

#include <a2l.h>
#include <hexfile.h>
#include "a2ltreemodel.h"
#include "mdimain.h"
class Csv;
class CdfxFile;

class WorkProject :  public A2l
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

        QMap<QString, Csv*> csvFiles();
        void addCsv(Csv *csv);
        void removeCsv(Csv *csv );
        void rename(Csv *csv);

        QMap<QString, CdfxFile*> cdfxFiles();
        void addCdfx(CdfxFile *cdfx);
        void removeCdfxFile(CdfxFile *cdfx );
        void rename(CdfxFile *cdfx);

        A2lTreeModel *treeModel;
        MDImain *parentWidget;

    private:
        QList<QObject*> owners;
        QMap<QString, HexFile*> hexList;
        QMap<QString, Csv*> csvList;
        QMap<QString, CdfxFile*> cdfxList;

    public slots:
        void detach(QObject*o);
        HexFile* getHex(QString str);
        QStringList getHexList();
        QString toString();

};

#endif // WORKPROJECT_H
