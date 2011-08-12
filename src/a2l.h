#ifndef A2L_H
#define A2L_H

#include <string>
#include <QRegExp>
#include <QStringList>
#include <QMap>
#include <Nodes/a2lfile.h>
#include <omp.h>
class HexFile;

using namespace std;

class A2l : public QObject
{
    Q_OBJECT

    public:
        A2l(QString fullFileName, QObject *parent = 0);
        ~A2l();

        A2LFILE *a2lFile;
        string getFullA2lFileName();
        void setFullA2lFileName(string str);
        void setFullA2lFileName(QString str);
        QStringList _outputList();
        void parse();
        void merge(A2LFILE *src, A2LFILE *trg);
        bool isOk();

    private:
        omp_lock_t lockValue;
        bool trunkA2l(QString &str, QString &str1, QString &str2);
        void initialize();
        QString fullA2lName;
        QStringList outputList;

        bool is_Ok;
        int progressVal;
        int progBarMaxValue;
        void openA2l();        
        void parseSTA2l();
        bool parseOpenMPA2l();
        void readSubset();

    signals:
        void incProgressBar(int,int);

    private slots:
        void checkProgressStream(int);

};

#endif // A2L_H
