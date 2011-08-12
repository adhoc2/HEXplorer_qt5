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
