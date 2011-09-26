#ifndef HEXFILE_H
#define HEXFILE_H

#include <string>
#include <QStringList>
#include <omp.h>
#include "node.h"
#include "datacontainer.h"
class WorkProject;
class CHARACTERISTIC;
class AXIS_PTS;
class MODULE;
class Data;
class PROJECT;
class MEMORY_SEGMENT;

class MemBlock
{
   public:
         ~MemBlock();
         QString offset;
         unsigned int start;
         unsigned int end;
         int length;
         int lineLength;
         unsigned char *data;
};

using namespace std;

class HexFile : public QObject, public DataContainer
{
    Q_OBJECT

    public:
        HexFile(QString fullHexFileName, WorkProject *parentWP, QString module, QObject *parent = 0);
        ~HexFile();

        // members
        Node* record_layout;
        Node* compu_method;
        Node* compu_vatb;
        QStringList listNotValidData;

        // methods
        void attach(QObject*o);
        int getNumByte(std::string str);
        void exportSubsetList2Csv(QStringList subsetList);        
        void exportSubsetList2Cdf(QStringList subsetList);        
        bool isValidAddress(QString address);
        bool read();
        void verify();
        void checkDisplay();
        void setFullName(QString fullName);
        QStringList writeHex();
        QString getHexValue(QString address, int offset, int nByte);
        QStringList getHexValues(QString address, int offset, int nByte, int size);
        QList<double> getDecValues(double address, int nByte, int size, std::string type);
        QString fullName();
        QList<int> checkFmtcMonotony(bool *bl);
        std::string pixmap();
        Data* readLabel(CHARACTERISTIC *label, bool phys);
        Data* readLabel(AXIS_PTS *label, bool phys);       
        PROJECT *getA2lFileProject();

    public slots:
        void detach(QObject*o);
        Data* getData(QString str);
        QString toString();


    private:

        // members
        omp_lock_t lock;
        int fileLinesNum;
        int maxValueProgbar;
        int valueProgBar;
        PROJECT *a2lProject;
        QList<MemBlock*> blockList;
        int refCount;        
        QList<QObject*> owners;
        QString fullHexName;
        QString byteOrder;
        QHash<QString,int> nByte;
        QList<uint> listMemSegData;


        static int asciiToByte[256*256];

        // methods
        void incrementValueProgBar(int n);        
        bool parseFile();
        void readAllData();
        bool isA2lCombined();
        void hex2MemBlock(Data* data);
        bool data2block();
        QStringList block2list();
        void setValue(unsigned int IAddr, QString hex, int nByte);
        void setValues(unsigned int IAddr, QStringList hexList, int nByte);
        QString checksum(QString str);       

    signals:
        //void lineParsed(int,int);
        void progress(int,int);
};

#endif // HEXFILE_H
