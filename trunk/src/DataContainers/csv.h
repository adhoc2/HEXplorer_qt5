#ifndef CSV_H
#define CSV_H

#include "node.h"
#include "workproject.h"
#include "datacontainer.h"
class Data;

class Csv : public QObject, public DataContainer
{
    Q_OBJECT

public:
    Csv(QString fullCsvFileName, WorkProject *parentWP, QString modName, QObject *parent = 0);
    ~Csv();

    std::string pixmap();
    QString fullName();
    QStringList getListNameData();
    void checkDisplay();
    void addLabelsAsChild();
    bool save(QString fileName = "");
    void setFullName(QString fullName);
    bool readFile();

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



signals:
    void incProgressBar(int,int);

private slots:
    void checkProgressStream(int);

};

#endif // CSV_H
