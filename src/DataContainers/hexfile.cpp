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

#include "hexfile.h"

#include <omp.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtAlgorithms>
#include <QFileInfo>
#include <workproject.h>
#include <typeinfo>
#include <math.h>
#include <limits.h>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDomDocument>
#include <QTime>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlQueryModel>

#include <qtconcurrentrun.h>
#include <QFutureWatcher>
#include <QtConcurrentMap>
#include <functional>


#include "Nodes/characteristic.h"
#include "Nodes/compu_method.h"
#include "Nodes/mod_common.h"
#include "Nodes/axis_pts.h"
#include "Nodes/function.h"
#include "data.h"
#include "graphverify.h"
#include "formcompare.h"
#include "stdint.h"
#include "stdio.h"

#define CAST2(val, type) *(type*)&val

// ______________ class MemBlock _________________//

MemBlock::~MemBlock()
{
    delete data;
}

// _______________ class HexFile Ctor/Dtor___________________//

int HexFile::asciiToByte[256*256];

//--ctor in case of a2lFile---//
HexFile::HexFile(QString fullHexFileName, WorkProject *parentWP, QString module, QObject *parent)
    : QObject(parent) , DataContainer(parentWP, module)
{
    //initialize
    fullHexName = fullHexFileName;
    name = new char[(QFileInfo(fullHexName).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(fullHexName).fileName()).toLocal8Bit().data());
    a2lProject = (PROJECT*)getParentWp()->a2lFile->getProject();
    maxValueProgbar = 0;
    valueProgBar = 0;
    omp_init_lock(&lock);

    //fill-in conversion table asciitoByte
    for(int i = 0; i < 16; i++)
    {
        uchar ii = (i<10)?i+48:i+55;
        for(int j = 0; j < 16; j++)
        {
            uchar jj = (j<10)?j+48:j+55;
            asciiToByte[ii*256 + jj] = j*16+i;
        }
    }


    //MOD_COMMON values (BYTE_ORDER, ALIGNEMENT_SWORD,...)
    MOD_COMMON *modCommon = (MOD_COMMON*)a2lProject->getNode("MODULE/" + getModuleName() + "/MOD_COMMON");
    if (modCommon)
    {
        Byte_Order *item = (Byte_Order*)modCommon->getItem("BYTE_ORDER");
        if (item)
            byteOrderCommon = item->getPar("ByteOrder");

        //define dataType map
        bool bl;
        ALIGNMENT_BYTE *item1 = (ALIGNMENT_BYTE*)modCommon->getItem("alignment_byte");
        if (item1)
        {
            QString str = item1->getPar("AlignmentBorder");
            nByte.insert("UBYTE", str.toInt(&bl,10));
            nByte.insert("SBYTE", str.toInt(&bl,10));
        }
        else
        {
            nByte.insert("UBYTE", 1);
            nByte.insert("SBYTE", 1);
        }
        ALIGNMENT_WORD *item2 = (ALIGNMENT_WORD*)modCommon->getItem("alignment_word");
        if (item2)
        {
            QString str = item2->getPar("AlignmentBorder");
            nByte.insert("UWORD", str.toInt(&bl,10));
            nByte.insert("SWORD", str.toInt(&bl,10));
        }
        else
        {
            nByte.insert("UWORD", 2);
            nByte.insert("SWORD", 2);
        }
        ALIGNMENT_LONG *item3 = (ALIGNMENT_LONG*)modCommon->getItem("alignment_long");
        if (item3)
        {
            QString str = item3->getPar("AlignmentBorder");
            nByte.insert("ULONG", str.toInt(&bl,10));
            nByte.insert("SLONG", str.toInt(&bl,10));
        }
        else
        {
            nByte.insert("ULONG", 4);
            nByte.insert("SLONG", 4);
        }
        ALIGNMENT_FLOAT32_IEEE *item4 = (ALIGNMENT_FLOAT32_IEEE*)modCommon->getItem("alignment_float32_ieee");
        if (item4)
        {
            QString str = item4->getPar("AlignmentBorder");
            nByte.insert("FLOAT32_IEEE", str.toInt(&bl,10));
        }
        else
        {
            nByte.insert("FLOAT32_IEEE", 4);
        }
        ALIGNMENT_FLOAT64_IEEE *item5 = (ALIGNMENT_FLOAT64_IEEE*)modCommon->getItem("alignment_float64_ieee");
        if (item5)
        {
            QString str = item5->getPar("AlignmentBorder");
            nByte.insert("FLOAT64_IEEE", str.toInt(&bl,10));
        }
        else
        {
            nByte.insert("FLOAT64_IEEE", 8);
        }
    }

    //save the node to speed-up HexFile import (see Data class constructor)
    compu_method = a2lProject->getNode("MODULE/" + module + "/COMPU_METHOD");
    record_layout = a2lProject->getNode("MODULE/" + module + "/RECORD_LAYOUT");
    compu_vtab = a2lProject->getNode("MODULE/" + module + "/COMPU_VTAB");
    compu_tab = a2lProject->getNode("MODULE/" + module + "/COMPU_TAB");

    //get the memory_block used to store datas
    MOD_PAR *modePar = (MOD_PAR*)a2lProject->getNode("MODULE/" + getModuleName() + "/MOD_PAR");
    foreach (Node* node, modePar->childNodes)
    {
        QString type = typeid(*node).name();
        if (type.endsWith("MEMORY_SEGMENT"))
        {
            MEMORY_SEGMENT* mem_seg = (MEMORY_SEGMENT*)node;
            QString _prgType = mem_seg->getPar("PrgType");
            if (_prgType == "DATA")
            {
                bool bl;
                listMemSegData.append(QString(mem_seg->getPar("Address")).toUInt(&bl, 16));
                listMemSegData.append(QString(mem_seg->getPar("Address")).toUInt(&bl, 16) +
                                      (QString(mem_seg->getPar("Size")).toUInt(&bl, 16)));
            }
        }
    }

    _read = true;

}

//--ctor in case of dbFile---//
HexFile::HexFile(QString fullHexFileName, WorkProject *parentWP, QObject *parent)
    : QObject(parent) , DataContainer(parentWP)
{
    //initialize
    fullHexName = fullHexFileName;
    name = new char[(QFileInfo(fullHexName).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(fullHexName).fileName()).toLocal8Bit().data());
    //a2lProject = (PROJECT*)getParentWp()->a2lFile->getProject();
    maxValueProgbar = 0;
    valueProgBar = 0;
    omp_init_lock(&lock);

    //fill-in conversion table asciitoByte
    for(int i = 0; i < 16; i++)
    {
        uchar ii = (i<10)?i+48:i+55;
        for(int j = 0; j < 16; j++)
        {
            uchar jj = (j<10)?j+48:j+55;
            asciiToByte[ii*256 + jj] = j*16+i;
        }
    }

    _read = false;
}

HexFile::~HexFile()
{
    omp_destroy_lock(&lock);
    qDeleteAll(blockList);
    delete[] name;
}

// ________________ Parser ___________________ //

bool HexFile::read()
{
    // parse the file
    if (parseFile())
    {
        //check hex version before reading all datas
        if (isA2lCombined())
        {
            readAllData();
            return true;
        }
        return false;
    }
    return false;
}

bool HexFile::isRead()
{
    return _read;
}

void HexFile::setRead(bool on)
{
    if (on)
        _read = true;
    else
        _read = false;

}

bool HexFile::read_db()
{
    // parse the file
//    if (parseFile())
//    {
//        //check hex version before reading all datas
//        if (isDbCombined())
//        {
//            readAllData_db();
//            return true;
//        }
//        return false;
//    }
    return false;

}

bool HexFile::parseFile()
{
    QTime timer;
    timer.start();

    // open file in binary format
    FILE *fid = fopen(fullHexName.toLocal8Bit(), "rb");

    if (fid == NULL)
    {
        return false;
    }

    // save the file into a buffer
    fseek(fid, 0, SEEK_END);
    int size = ftell(fid);
    rewind(fid);
    char *buffer = new char[size * sizeof(char)];
    fread(buffer, sizeof(char), size, fid);
    fclose(fid);

    qDebug() << " ---- HEXfile ---- ";
    qDebug() << "1- read file : " << timer.elapsed();
    timer.restart();

    //save the buffer into a QStringList
    QString strBuffer = QString::fromLatin1(buffer, size);
    QTextStream in;
    in.setString(&strBuffer);
    QStringList lines;
    while (!in.atEnd())
    {
        QString str = in.readLine();

        if (str.startsWith(":"))
        {
            lines << str;
        }
        else if (str.startsWith(26))
        {
            // ignore line
            // some A2L files end with "SUB"...
        }
        else
        {
            QMessageBox::critical(0, "HEXplorer :: HexFile", "wrong Hex file format.");
            return false;
        }
    }
    fileLinesNum = lines.count();

    //get the length for the progressBar
    QString name = typeid(*this->getParentNode()).name();
    if (name.toLower().endsWith("a2lfile"))
    {
//        A2LFILE *a2l = (A2LFILE*)this->getParentNode();
//        MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + getModuleName());
//        int length = module->listChar.count();
        maxValueProgbar = fileLinesNum;// + length;
    }
    else if (name.toLower().endsWith("dbfile"))
    {
        //connect to the correct Database
        DBFILE *dbFile = (DBFILE*)this->getParentNode();
        QString connectionName = dbFile->getSqlConnection();
        QSqlDatabase db = QSqlDatabase::database(connectionName, true);

        //getthe number of characteristics into database
        int length = 0;
        QSqlQuery query("select count(*) from CHARACTERISTICS", db);
        while (query.next())
        {
            length += query.value(0).toInt();
        }
        query.exec("select count(*) from AXIS_PTS");
        while (query.next())
        {
            length += query.value(0).toInt();
        }
        maxValueProgbar = fileLinesNum + length;
    }

    // initialize variables
    int dataCnt = 0;
    int lineLength = 0;
    bool firstLineOfBlock = false;
    int type = 0;
    MemBlock *actBlock = 0;
    int cnt = 0;
    int previous = 0;

    // parse each line
    while (cnt < lines.count())
    {
        // get the first line to be processed
        QByteArray barray = lines.at(cnt).toLocal8Bit();
        char* _line = barray.data();

        // get the type and the length of the record
        bool ok;
        type = asciiToByte[*(ushort*)(_line + 7)];

        // according to the type of the record, do :
        switch (type)
        {
        case 0: //Data Record
        {
            //finish MemBlock definition
            if (firstLineOfBlock)
            {
                // get the length, start, offset of the block
                if (actBlock->uSBA.isEmpty())
                {
                    actBlock->start = (actBlock->uLBA + QByteArray(_line + 3, 4)).toUInt(&ok, 16);
                    int end = (actBlock->uLBA + "FFFF").toUInt(&ok, 16);
                    actBlock->lineLength = asciiToByte[*(ushort*)(_line + 1)];
                    actBlock->data = new unsigned char [(end - actBlock->start + 1)];
                }
                else if (actBlock->uLBA.isEmpty())
                {
                    actBlock->start = (actBlock->uSBA + "0").toUInt(&ok, 16) + QByteArray(_line + 3, 4).toUInt(&ok, 16);
                    int end = (actBlock->uSBA + "0").toUInt(&ok, 16) +  65535;
                    actBlock->lineLength = asciiToByte[*(ushort*)(_line + 1)];
                    actBlock->data = new unsigned char [(end - actBlock->start + 1)];
                }

                firstLineOfBlock = false;
            }

            //read all the lines of the memory block
            while (type == 0)
            {
                lineLength = asciiToByte[*(ushort*)(_line + 1)];
                if (actBlock->uSBA.isEmpty())
                {
                    dataCnt = (actBlock->uLBA + QByteArray(_line + 3, 4)).toUInt(&ok, 16) - actBlock->start;
                }
                else if (actBlock->uLBA.isEmpty())
                {
                    dataCnt = (actBlock->uSBA + "0").toUInt(&ok, 16) + QByteArray(_line + 3, 4).toUInt(&ok, 16) - actBlock->start;
                }

                // save the ascii characters into a byte array
                for (int i = 0; i < lineLength; i++)
                {
                    actBlock->data[dataCnt] = asciiToByte[*(ushort*)(_line + 9 + 2*i)];
                    dataCnt++;
                }

                // get the type and length of next line
                cnt++;
                barray = lines.at(cnt).toLocal8Bit();
                _line = barray.data();
                type = asciiToByte[*(ushort*)(_line + 7)];

            }

            //set blockend
            actBlock->end = actBlock->start + dataCnt - 1;
            actBlock->length = actBlock->end - actBlock->start + 1;

            //add block to the list of block
            blockList.append(actBlock);
        }

            break;

        case 1: //End of File
            //cnt++;
            cnt = lines.count();
            break;

        case 2: //Extended segment address record
        {
            // create a new memory block
            actBlock = new MemBlock();

            // get the length, start, offset of the block
            actBlock->uLBA = "";
            actBlock->uSBA = QByteArray(_line + 9, 4);

            // set the flag to firstLineOfBlock to finish MemBlock definition
            firstLineOfBlock = true;

            cnt++;
        }
            break;

        case 3: //Start Segment address record
            cnt++;
            break;

        case 4: // Extended Linear Address Record
        {
            // create a new memory block
            actBlock = new MemBlock();

            // get the length, start, offset of the block
            actBlock->uSBA = "";
            actBlock->uLBA = QByteArray(_line + 9, 4);

            // set the flag to firstLineOfBlock to finish MemBlock definition
            firstLineOfBlock = true;

            cnt++;
        }

            break;

        case 5: // Start Linear Address record
            cnt++;
            break;

        default:
            break;
        }

        // increment progressbar
        incrementValueProgBar(cnt - previous);
        previous = cnt;
    }
    incrementValueProgBar(cnt - previous);

    qDebug() << "2- parse file : " << timer.elapsed();

    // free memory from buffer
    delete buffer;

    return true;

 }

bool HexFile::isA2lCombined()
{
    MOD_PAR *modePar = (MOD_PAR*)a2lProject->getNode("MODULE/" + getModuleName() + "/MOD_PAR");
    if (modePar)
    {
        QString str1;
        QString str;

        //EPK in A2l
        EPK *epk = (EPK*)modePar->getItem("epk");
        if (epk)
        {
            str1 = epk->getPar("Identifier");
            str1.remove('\"');

            //get the address of EPK in A2l
            ADDR_EPK *addr_epk = (ADDR_EPK*)modePar->getItem("addr_epk");
            if (addr_epk)
            {
                //get EPK address
                QString address = addr_epk->getPar("Address");
                QStringList hexVal = getHexValues(address, 0, 1, str1.size());

                //get EPK value into HexFile;
                double c;
                for (int i = 0; i < hexVal.count(); i++)
                {
                    QString val = hexVal.at(i);
                    bool bl;
                    c = val.toUInt(&bl,16);
                    if (32 <= c && c < 127)
                        str.append((unsigned char)c);
                }

                //compare EPK from A2L with EPK from HexFile
                if (str == str1)
                    return true;
                else
                {
                    int r = QMessageBox::question(0,
                                                  "HEXplorer::question",
                                                  "The a2l and hex version does not seem to be compatible.\n"
                                                  "A2l EEPROM Identifier : " + str1 + "\n"
                                                  "Hex EEPROM Identifier : " + str + "\n\n"
                                                  "Are you sure you want to proceed ?",
                                                  QMessageBox::Yes, QMessageBox::No);
                    if (r == QMessageBox::Yes)
                        return true;
                    else
                        return false;
                }
            }
            else
                return true;
        }
        else //EDC7: no addr_epk available into HexFile
            return true;
    }
    else
        return true;
}

bool HexFile::isDbCombined()
{
    return true;
}

void HexFile::readAllData()
{
    QTime timer;
    timer.start();

    //empty the list
    listData.clear();

    //create list
    A2LFILE *a2l = (A2LFILE*)this->getParentNode();
    MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + getModuleName());

    //read labels
    QProgressDialog dialog;
    dialog.setLabelText(QString("Reading HEX file : progressing using %1 thread(s)...").arg(QThread::idealThreadCount()));

    QFutureWatcher<Data*> futureWatcher;
    QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int,int)), &dialog, SLOT(setRange(int,int)));
    QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));

    // Start the computation
    futureWatcher.setFuture(QtConcurrent::mapped(module->listChar, std::bind(&HexFile::runCreateDataMapped, this, std::placeholders::_1)));
    dialog.exec();
    futureWatcher.waitForFinished();

    //fill-in listData
    listData = futureWatcher.future().results();

    qDebug() << "3- readAllData " << timer.elapsed();
}

void HexFile::readAllData_db()
{
    QTime timer;
    timer.start();

    //empty the list
    listData.clear();

    //create list
    DBFILE *dbFile = (DBFILE*)this->getParentNode();
    QString connectionName = dbFile->getSqlConnection();
    QSqlDatabase db = QSqlDatabase::database(connectionName, true);

    //read labels names : create a list of all the CHARACTERISTIC names
    QStringList listChar;
    if (db.isOpen())
    {
        int length = 0;
        QSqlQuery query("select Name from CHARACTERISTICS", db);
        while (query.next())
        {
            listChar.append(query.value(0).toString());
        }
        query.exec("select Name from AXIS_PTS");
        while (query.next())
        {
            listChar.append(query.value(0).toString());
        }
        listChar.sort();

        //create a Data for each CHAARCTERISTIC into DB
        query.exec("SELECT * FROM CHARACTERISTICS");

        int i= 0;
        while (query.next())
        {
            QSqlRecord record = query.record();

            Data *data = new Data(record, db, this);
            listData.append(data);

             // increment valueProgBar
             if (i % 6 == 1)
                incrementValueProgBar(6);

            i++;
        }

    }

    qDebug() << "3- readAllData " << timer.elapsed();
}

Data* HexFile::runCreateDataMapped(const QString &str)
{
    A2LFILE *a2l = (A2LFILE*)this->getParentNode();
    Node *nodeChar = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
    Node *nodeAxis = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");

    bool found = false;

    // search into CHARACTERISTIC
    if (nodeChar)
    {
        //rwLock.lockForRead();
        Node *label = nodeChar->getNode(str);
        //rwLock.unlock();
        if (label)
        {
            found = true;
            CHARACTERISTIC *charac = (CHARACTERISTIC*)label;
            QString add = charac->getPar("Adress");
            bool bl = isValidAddress(add);
            if(1)
            {
                //rwLock.lockForRead();
                Data *data = new Data(charac, a2lProject, this);
                //rwLock.unlock();
                return data;
            }
            else
            {
                return 0;
            }
        }
    }

    // search into AXIS_PTS
    if (nodeAxis && !found)
    {
        //rwLock.lockForRead();
        Node *label2 = nodeAxis->getNode(str);
        //rwLock.unlock();
        if (label2)
        {
            found = true;
            AXIS_PTS *axis = (AXIS_PTS*)label2;
            QString add = axis->getPar("Adress");

            bool bl = isValidAddress(add);
            if (1)
            {
                //rwLock.lockForRead();
                Data *data = new Data(axis, a2lProject, this);
                //rwLock.unlock();
                return data;
            }
            else
            {
                return 0;
            }
        }
    }

    // display not found
    if (!found)
    {
        return 0;
    }

}


// ________________ read Hex values___________________ //

QString HexFile::getHexValue(QString address, int offset,  int nByte, QString _byteOrder)
{
    //find block and index of the desired address
    bool bl;
    unsigned int IAddr =address.toUInt(&bl, 16) + offset;
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }

    //if address is outside the Hex file address range => exit
    if (block >= blockList.count())
        return "00";

    //get index of the address into block
    int index = IAddr - blockList[block]->start;

    //read the byte in QList
    QList<unsigned char> tab;
    if (index + nByte < blockList[block]->length)
    {
        for (int i = 0; i < nByte; i++)
            tab.append( blockList[block]->data[index + i]);
    }
    else if (block < blockList.count() - 1)
    {
        int size = blockList[block]->length - index;
        int i = 0;
        for (i = 0; i < size; i++)
            tab.append(blockList[block]->data[index + i]);
        for (int j = 0; j < nByte - size; j++)
            tab.append(blockList[block + 1]->data[j]);
    }
    else
        return "00";

    //MSB_FIRST or MSB_LAST
    QString str = "ZZ";
    if (_byteOrder.isEmpty())
    {
        _byteOrder = byteOrderCommon;
    }
    if (_byteOrder.toLower() == "msb_first")
    {
        QString hex;
        str = "";
        for (int i = 0; i < tab.count(); i++)
        {
            hex.setNum(tab.at(i), 16);
            if (hex.length() < 2)
             str += "0" + hex;
            else
                str += hex;
        }
    }
    else if (_byteOrder.toLower() == "msb_last")
    {
        QString hex;
        str = "";
        for (int i = tab.count() - 1; i >= 0; i--)
        {
            hex.setNum(tab.at(i), 16);
            if (hex.length() < 2)
             str += "0" + hex;
            else
                str += hex;
        }
    }

    return str.toUpper();
}

QStringList HexFile::getHexValues(QString address, int offset, int nByte, int count, QString _byteOrder)
{
    //variable to be returned
    QStringList hexList;

    //find block and line
    bool bl;
    unsigned int IAddr =address.toUInt(&bl, 16) + offset;
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }

    //if address is outside the Hex file address range => exit
    if (block >= blockList.count())
    {
        hexList.append("00");
        return hexList;
    }

    //get index of the address into block
    int line = IAddr - blockList[block]->start;

    //read the byte in QList
    QList<unsigned char> tab;
    if (line + nByte*count < blockList[block]->length)
    {
        for (int i = 0; i < nByte*count; i++)
            tab.append( blockList[block]->data[line + i]);
    }
    else
    {
        int size = blockList[block]->length - line;
        for (int i = 0; i < size; i++)
            tab.append(blockList[block]->data[line + i]);
        for (int j = 0; j < nByte*count - size; j++)
            tab.append(blockList[block + 1]->data[j]);
    }

    //MSB_FIRST or MSB_LAST
    if (_byteOrder.isEmpty())
    {
        _byteOrder = byteOrderCommon;
    }
    if (byteOrderCommon.toLower() == "msb_first")
    {
        QString hex;
        QString str;
        for (int i = 0; i < count; i++)
        {
            str = "";
            for (int n = 0; n < nByte; n++)
            {
                hex.setNum(tab.at(i*nByte + n), 16);
                if (hex.length() < 2)
                    str += "0" + hex;
                else
                    str += hex;
            }

            hexList.append(str);
        }
    }
    else if (byteOrderCommon.toLower() == "msb_last")
    {
        QString hex;
        QString str;
        for (int i = 0; i < count; i++)
        {
            str = "";
            for (int n = nByte - 1; n >= 0; n--)
            {
                hex.setNum(tab.at(i*nByte + n), 16);
                if (hex.length() < 2)
                 str += "0" + hex;
                else
                    str += hex;
            }
            hexList.append(str);
        }
    }

    return hexList;
}

QList<double> HexFile::getDecValues(double IAddr, int nByte, int count, std::string type, QString _byteOrder)
{
    //variable to be returned
    QList<double> decList;

    //find block and line
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }

    //if address is outside the Hex file address range => exit
    if (block >= blockList.count())
    {
        decList.append(0);
        return decList;
    }

    //get index of the address into block
    int index = IAddr - blockList[block]->start;

    //MSB_FIRST or MSB_LAST
    if (_byteOrder.isEmpty())
    {
        _byteOrder = byteOrderCommon;
    }
    if (_byteOrder.toLower() == "msb_last")
    {
        if(type == "SBYTE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], int8_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int8_t));
                }

                delete buffer;
            }
        }
        else if(type == "UBYTE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], uint8_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], uint8_t));
                }

                delete buffer;
            }
        }
        else if(type == "SWORD")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], int16_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int16_t));
                }

                delete buffer;
            }

        }
        else if(type == "UWORD")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], uint16_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], uint16_t));
                }

                delete buffer;
            }

        }
        else if(type == "SLONG")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], int32_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int32_t));
                }

                delete buffer;
            }
        }
        else if(type == "ULONG")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], uint32_t));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], uint32_t));
                }

                delete buffer;
            }
        }
        else if(type == "FLOAT32_IEEE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[index + nByte*i], float));
                }
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], float));
                }

                delete buffer;
            }
        }
    }
    else
    {
        if(type == "SBYTE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], int8_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], int8_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "UBYTE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], uint8_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], uint8_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "SWORD")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], int16_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                {
                    //qDebug() << blockList.length() << " : " << block;
                    buffer[size + j] = blockList[block + 1]->data[j];
                }

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], int16_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "UWORD")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], uint16_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], uint16_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "SLONG")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], int32_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], int32_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "ULONG")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], uint32_t));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], uint32_t));
                }

                delete mem;
                delete buffer;
            }
        }
        else if(type == "FLOAT32_IEEE")
        {
            if (index + nByte*count < blockList[block]->length)
            {
                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = blockList[block]->data[index + (nByte * (i + 1) - j - 1)];
                    }
                    decList.append(CAST2(mem[0], float));
                }

                delete mem;
            }
            else if (block < blockList.count() - 1)
            {
                char* buffer = new char[nByte*count];
                int size = blockList[block]->length - index;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[index + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                char* mem = new char[nByte];
                for (int i = 0; i < count; i++)
                {
                    for (int j = 0; j < nByte; j++)
                    {
                        mem[j] = buffer[nByte * (i + 1) - j - 1];
                    }
                    decList.append(CAST2(mem[0], float));
                }

                delete mem;
                delete buffer;
            }
        }

    }

    return decList;
}

bool HexFile::isValidAddress(QString address)
{
//    int length = listMemSegData.count();
//    bool bl;
//    unsigned int IAddr =address.toUInt(&bl, 16);

//    if (length == 0)
//    {
//        int block = 0;
//        while (block < blockList.count())
//        {
//            if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
//            {
//                break;
//            }
//            block++;
//        }

//        if (block >=  blockList.count())
//            return false;
//        else
//            return true;

//    }
//    else
//    {
//        for (int i = 0; i < length - 1; i+=2)
//        {
//            if (listMemSegData.at(i) <= IAddr && IAddr < listMemSegData.at(i + 1) )
//                return true;
//        }
//    }


//    return false;

    bool bl;
    unsigned int IAddr =address.toUInt(&bl, 16);

    //find block and line
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }

    //if address is outside the Hex file address range => exit
    if (block >= blockList.count())
    {
        return false;
    }
    else
        return true;
}

int HexFile::getNumByte(std::string str)
{
    return nByte.value(str.c_str());
}

// _______________ save Hex values __________________ //

QStringList HexFile::writeHex()
{
    QStringList list;
    if (testMonotony(list))
    {
        data2block();
        return block2list();
    }
    else
    {
        QStringList list;
        return list;
    }
}

void HexFile::setValue(unsigned int IAddr, QString hex,  int nByte, QString _byteOrder)
{
    //find block and line
    bool bl;
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }
    int line = IAddr - blockList[block]->start;

    //MSB_FIRST or MSB_LAST
    QList<unsigned char> tab;
    if (_byteOrder.isEmpty())
    {
        _byteOrder = byteOrderCommon;
    }
    if (_byteOrder.toLower() == "msb_first")
    {
        QString str;
        for (int i = 0; i < nByte; i++)
        {
            str = hex.mid(i * 2, 2);
            tab.append(str.toUShort(&bl, 16));
        }
    }
    else if (_byteOrder.toLower() == "msb_last")
    {
        QString str;
        for (int i = nByte - 1; i >= 0; i--)
        {
            str = hex.mid(i * 2, 2);
            tab.append(str.toUShort(&bl, 16));
        }
    }

    //copy tab into MemBlock
    if (line + nByte < blockList[block]->length)
    {
        for (int i = 0; i < nByte; i++)
        {
            blockList[block]->data[line + i] = tab.at(i);
        }
    }
    else
    {
        int size = blockList[block]->length - line;
        int i = 0;
        for (i = 0; i < size; i++)
            blockList[block]->data[line + i] = tab.at(i);
        for (int j = 0; j < nByte - size; j++)
            blockList[block + 1]->data[j] = tab.at(size + j);
    }
}

void HexFile::setValues(unsigned int IAddr, QStringList hexList, int nByte, QString _byteOrder)
{
    //find block and line
    bool bl;
    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }
    int line = IAddr - blockList[block]->start;

    //MSB_FIRST or MSB_LAST
    QList<unsigned char> tab;
    if (_byteOrder.isEmpty())
    {
        _byteOrder = byteOrderCommon;
    }
    if (_byteOrder.toLower() == "msb_first")
    {
        QString str;
        foreach (QString hex, hexList)
        {
            for (int i = 0; i < nByte; i++)
            {
                str = hex.mid(i * 2, 2);
                tab.append(str.toUShort(&bl, 16));
            }
        }
    }
    else if (_byteOrder.toLower() == "msb_last")
    {
        QString str;
        foreach (QString hex, hexList)
        {
            for (int i = nByte - 1; i >= 0; i--)
            {
                str = hex.mid(i * 2, 2);
                tab.append(str.toUShort(&bl, 16));
            }
        }
    }

    //copy tab into MemBlock
    if (line + tab.count() < blockList[block]->length)
    {
        for (int i = 0; i < tab.count(); i++)
            blockList[block]->data[line + i] = tab.at(i);
    }
    else
    {
        int size = blockList[block]->length - line;
        for (int i = 0; i < size; i++)
            blockList[block]->data[line + i] = tab.at(i);
        for (int j = 0; j < tab.count() - size; j++)
            blockList[block + 1]->data[j] = tab.at(size + j);
    }
}

void HexFile::hex2MemBlock(Data *data)
{
    QString type = typeid(*data->getA2lNode()).name();
    if (type.endsWith("CHARACTERISTIC"))
    {
        CHARACTERISTIC *node = (CHARACTERISTIC*)data->getA2lNode();
        type = node->getPar("Type");

        if (type.toLower() == "value")
        {
            //BIT_MASK
            BIT_MASK *_bitmask = (BIT_MASK*)node->getItem("BIT_MASK");
            uint16_t mask = 0;
            bool bl;
            if (_bitmask)
            {
                // get the mask into uint
                mask = QString(_bitmask->getPar("Mask")).toUInt(&bl, 16);
                // calculate the decalage
                uint16_t decalage = tzn(mask);
                // get the original Value into int
                int nbyte = data->getZ(0).count() / 2;
                int16_t orgValue = getDecValues(data->getAddressZ(), nbyte, 1, data->getDatatypeZ(), data->getByteOrderZ()).at(0);
                // get the value to be set into int
                int16_t _setValue = data->getZ(0).toInt(&bl,16);
                _setValue = _setValue << decalage;
                // set the bits into orgValue according to the mask
                int n = 16;
                for (int i = 0; i < n; i++)
                {
                    if (mask & (1 << i))
                    {
                        if (_setValue & (1 << i))
                        {
                            orgValue |= (1 << i);
                        }
                        else
                        {
                            orgValue &= ~(1 << i);
                        }
                    }
                }
                // convert orgValue into HEX
                QString hexOrgValue = dec2hex(orgValue, data->getDatatypeZ());
                // write the HEX value
                setValue(data->getAddressZ(), hexOrgValue, nbyte, data->getByteOrderZ());
            }
            else
            {
                int nbyte = data->getZ(0).count() / 2;
                setValue(data->getAddressZ(), data->getZ(0), nbyte, data->getByteOrderZ());
            }
        }
        else if(type.toLower() == "curve")
        {
            int nbyteX = data->getX(0).count() / 2;

            //axisX : copy axisX only if it is a std_axis
            if (data->getAxisDescrX())
            {
                QString typeAxisX = data->getAxisDescrX()->getPar("Attribute");
                if (typeAxisX.compare("STD_AXIS") == 0)
                {
                    setValues(data->getAddressX(), data->getX(), nbyteX, data->getByteOrderX());
                }
            }

            //axisZ
            if (!data->isSizeChanged())
            {
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(data->getAddressZ(), data->getZ(), nbyteZ, data->getByteOrderZ());
            }
            else
            {
                // write new length of axis X into HexFile
                bool bl;
                double addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                QString length = data->getnPtsXHexa();
                int nbyteNPtsX = length.length() / 2;
                setValue(addr, length, nbyteNPtsX, data->getByteOrderX());

                //calculate new Address Z due to axisX length modification
                double newAddressZ = data->getAddressX() + nbyteX * data->xCount();

                //write the Z hex values
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(newAddressZ, data->getZ(), nbyteZ, data->getByteOrderZ());

            }
        }
        else if(type.toLower() == "map")
        {
             int nbyteX = data->getX(0).count() / 2;
             int nbyteY = data->getY(0).count() / 2;

            //axisX : copy axisX only if it is a std_axis
            if (data->getAxisDescrX())
            {
                QString typeAxisX = data->getAxisDescrX()->getPar("Attribute");
                if (typeAxisX.compare("STD_AXIS") == 0)
                {
                    setValues(data->getAddressX(), data->getX(), nbyteX, data->getByteOrderX());
                }
            }

            //axisY : copy axisY only if it is a std_axis
            if (!data->isSizeChanged())
            {
                if (data->getAxisDescrY())
                {
                    QString typeAxisX = data->getAxisDescrX()->getPar("Attribute");
                    if (typeAxisX.compare("STD_AXIS") == 0)
                    {
                        setValues(data->getAddressY(), data->getY(), nbyteY, data->getByteOrderY());
                    }
                }

                //axisZ
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(data->getAddressZ(), data->getZ(), nbyteZ, data->getByteOrderZ());
            }
            else
            {
                // write new length of axis X into HexFile
                bool bl;
                double addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                QString length = data->getnPtsXHexa();
                int nbyteNPtsX = length.length() / 2;
                setValue(addr, length, nbyteNPtsX, data->getByteOrderX());

                // write new length of axis Y into HexFile
                bl;
                addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                length = data->getnPtsYHexa();
                int nbyteNPtsY = length.length() / 2;
                setValue(addr + nbyteNPtsX, length, nbyteNPtsY, data->getByteOrderY());

                //calculate new Address Y due to axisX length modification
                double newAddressY = data->getAddressX() + nbyteX * data->xCount();

                //write the Y hex values
                int nbyteY = data->getY(0).count() / 2;
                setValues(newAddressY, data->getY(), nbyteY, data->getByteOrderY());

                //calculate new Address Z due to axisX and axisY length modification
                double newAddressZ = newAddressY + nbyteY * data->yCount();

                //write the Z hex values
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(newAddressZ, data->getZ(), nbyteZ, data->getByteOrderZ());


            }
        }
        else if (type.toLower() == "val_blk")
        {
            //axisZ
            int nbyteZ = data->getZ(0).count() / 2;
            setValues(data->getAddressZ(), data->getZ(), nbyteZ, data->getByteOrderZ());
        }
        else if (type.toLower() == "ascii")
        {
            //axisZ
            int nbyteZ = data->getZ(0).count() / 2;
            setValues(data->getAddressZ(), data->getZ(), nbyteZ, data->getByteOrderZ());
        }
    }
    else
    {
        //axisZ
        int nbyteZ = data->getZ(0).count() / 2;
        setValues(data->getAddressZ(), data->getZ(), nbyteZ, data->getByteOrderZ());
    }
}

QString HexFile::checksum(QString values)
{
    //Sum the bytes
    int count = 0;
    int length = values.count();
    bool bl;
    for (int i = 1; i < length - 1; i = i + 2)
        count += values.mid(i, 2).toUInt(&bl, 16);

    //Count mod 0x100 or 256(int)
    int mod = count % 256;

    //Return 256 - mod
    char hex[31];
    sprintf(hex, "%X", 256 - mod);

    QString cks = hex;
    if (cks == "100")
        cks = "00";
    else if (cks.count() < 2)
        cks = "0" + cks;

    return cks;
}

QStringList HexFile::block2list()
{
    QStringList lineList;
    QString address = "";
    QString line = "";
    int x = 0;
    int j = 0;

    //define the progressBar length
    maxValueProgbar = fileLinesNum + blockList.count() * 2000;

    for (int i = 0; i < blockList.count(); i++)
    {
        QString cks = "";
        int strt = 0;
        if (blockList[i]->uSBA.isEmpty())
        {
            cks = checksum(":02000004" + blockList[i]->uLBA);
            lineList.append(":02000004" + blockList[i]->uLBA + cks);
        }
        else if (blockList[i]->uLBA.isEmpty())
        {
            cks = checksum(":02000002" + blockList[i]->uSBA);
            lineList.append(":02000002" + blockList[i]->uSBA + cks);
        }

        x = 0;
        j = 0;

//        bool bl;
//        QString start = blockList[i]->uLBA + "0000";
//        strt = start.toUInt(&bl, 16);

        strt = blockList[i]->start;

        int end = blockList[i]->length;
        while (j < end)
        {
            //create a line from data
            QString dat;
            while (line.length() < blockList[i]->lineLength * 2)
            {
                if (j <  blockList[i]->length)
                {
                    dat.setNum(blockList[i]->data[j], 16);
                    if (dat.length() < 2)
                     dat = "0" + dat;

                    line.append(dat);
                    j++;
                }
                else
                {
                    j = (int)blockList[i]->length;
                    break;
                }
            }

            //complete the line (:, address, checksum)
            if (line.count() != 0)
            {
                //HEX: line address
                int tamere =  blockList[i]->start - strt + x * blockList[i]->lineLength;
                char hex[31];
                sprintf(hex, "%X", tamere);
                address = hex;
                while (address.length() < 4)
                    address = "0" + address;

                //HEX: line length
                sprintf(hex, "%X", line.count()/ 2);
                QString length = hex;
                if (length.length() < 2)
                    length = "0" + length;
                QString str1 = ":" + length + address + "00" + line;
                cks = checksum(str1);
                lineList.append((str1 + cks).toUpper());
                x++;
                line = "";
            }
        }

        emit progress(i *2000, maxValueProgbar);
    }

    lineList.append(":00000001FF");

    return lineList;

}

bool HexFile::data2block()
{
    foreach(Data* data, modifiedData)
    {
        data->clearOldValues();

        //transform all the values into data in Hex format : getX() or getY() when COM_AXIS still returns physical format
        data->phys2hex();

        hex2MemBlock(data);
        data->hex2phys();
        removeModifiedData(data);
    }

    return true;
}

// ________________ Data ___________________ //

Data* HexFile::getData(QString str)
{
    return ((DataContainer*)this)->getData(str);
}

void HexFile::checkDisplay()
{
    //check if parentWp still exists
    if (!parentNode->getParentNode()->isChild(this->parentNode->name))
    {
        int ret = QMessageBox::warning(0, "HEXplorer :: add project",
                                        "This project was deleted !\nDo you want to reload it ?",
                                        QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            MDImain *mdi = getParentWp()->parentWidget;
            mdi->reAppendProject(getParentWp());
        }
    }

    //check if this is alaways a child of its parentWp
    if (!parentNode->isChild(this->name))
    {

        int ret = QMessageBox::warning(0, "HEXplorer :: add hexFile",
                                        "This Hex file was deleted !\nDo you want to reload it ?",
                                        QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            WorkProject *wp = getParentWp();
            wp->addHex(this);
            //this->attach(wp);
        }
    }
}

// ________________ export Subset ___________________ //

void HexFile::exportSubsetList2Csv(QStringList subsetList)
{
    A2LFILE *a2l = getParentWp()->a2lFile;

    //create CSV file(s)
    Node *fun = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/FUNCTION");
    if (fun != NULL)
    {
        QSettings settings(qApp->organizationName(), qApp->applicationName());
        QString path = settings.value("currentCsvPath").toString();
        QString fileName = QFileDialog::getSaveFileName(0,tr("select CSV files"), path,
                                                        tr("CSV files (*.csv);;all files (*.*)"));
        if (fileName.isEmpty())
            return;
        else if (!fileName.toLower().endsWith(".csv"))
            fileName.append(".csv");

        //define progressBar
        maxValueProgbar = subsetList.count();

        foreach (QString str, subsetList)
        {
            //create a file with subset name
            QString dir = QFileInfo(fileName).absolutePath();
            QString name = QFileInfo(fileName).fileName();
            name = str + "_" + name;
            QString newFileName = dir + "/" + name;

            //check if exists the new file name
            QFile file(newFileName);
            if (file.exists())
            {

                int ret = QMessageBox::question(0, "HEXplorer :: export subset",
                                     "the file " + newFileName + " already exists.\n"
                                     "Do you want to overwrite it ?", QMessageBox::Yes, QMessageBox::No);

                if (ret == QMessageBox::No)
                {
                    return;
                }
            }

            //save the file
            FUNCTION *subset = (FUNCTION*)fun->getNode(str);
            if (subset)
            {
                // test monotony
                DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)subset->getNode("DEF_CHARACTERISTIC");
                QStringList labelList = def_char->getCharList();

                exportDataList2Csv(labelList, newFileName);
            }


            //increment progressBar
            emit incrementValueProgBar(1);
        }
    }
}

void HexFile::exportSubsetList2Cdf(QStringList subsetList)
{
    A2LFILE *a2l = getParentWp()->a2lFile;

    //create CDF file(s)
    Node *fun = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/FUNCTION");
    if (fun != NULL)
    {
        QSettings settings(qApp->organizationName(), qApp->applicationName());
        QString path = settings.value("currentCdfxPath").toString();
        QString fileName = QFileDialog::getSaveFileName(0,tr("select CDF files"), path,
                                                        tr("CDF files (*.cdfx | *.cdf);;all files (*.*)"));

        if (fileName.isEmpty())
            return;
        else if (!fileName.toLower().endsWith(".cdfx"))
            fileName.append(".cdfx");

        foreach (QString str, subsetList)
        {
            //create a file with subset name
            QString dir = QFileInfo(fileName).absolutePath();
            QString name = QFileInfo(fileName).fileName();
            name = str + "_" + name;
            QString newFileName = dir + "/" + name;

            //check if exists the new file name
            QFile file(newFileName);
            if (file.exists())
            {

                int ret = QMessageBox::question(0, "HEXplorer :: export Subset to Cdf file",
                                     "the file " + newFileName + " already exists.\n"
                                     "Do you want to overwrite it ?", QMessageBox::Yes, QMessageBox::No);

                if (ret == QMessageBox::No)
                {
                    return;
                }
            }

            //write values
            FUNCTION *subset = (FUNCTION*)fun->getNode(str);
            if (subset)
            {
                DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)subset->getNode("DEF_CHARACTERISTIC");
                QStringList labelList = def_char->getCharList();

                exportDataList2Cdf(labelList, newFileName);
            }
        }
    }
}

// _______________ Others __________________ //

void HexFile::verify()
{
    GraphVerify *graphVerify = new GraphVerify(this);
    graphVerify->show();
}

QList<int> HexFile::checkFmtcMonotony(bool *bl)
{
    //get FMTC map
    Data *fmtc = NULL;
    QString projectName = getA2lFileProject()->getPar("name");;
    if (projectName.toLower() == "c340")
    {
        fmtc = getData("FMTC_trq2qBas_MAP");
    }
    else if(projectName.toLower() == "p_662")
    {
        fmtc = getData("PhyMod_trq2qBasEOM0_MAP");
        if (!fmtc)
        {
            fmtc = getData("PhyMod_trq2qBas_MAP");
        }
    }

    //check monotony of each column
    QList<int> list;
    if (fmtc)
    {
        *bl = true;
        for (int col = 0; col < fmtc->xCount(); col++)
        {
            for (int row = 0; row < fmtc->yCount() - 1; row++)
            {
                bool bl1;
                double dbl1 = fmtc->getZ(row, col, &bl1);
                double dbl2 = fmtc->getZ(row + 1, col, &bl1);

                if (dbl1 >= dbl2)
                {
                    list.append(col);
                    *bl = false;
                }

            }
        }
    }

    return list;
}

void HexFile::attach(QObject *o)
{
    //check owner for validity
    if(o == 0)
        return;

    //check for duplicates
    //if(owners.contains(o)) return;

    //register
    owners.append(o);
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(detach(QObject*)));

}

void HexFile::detach(QObject *o)
{
    //remove it
    owners.removeOne(o);

    //remove self after last one
    if(owners.size() == 0)
    {
        qDebug() << "HEX : " << this->name << " deleted";
        delete this;
    }
}

QList<QObject*> HexFile::getOwners()
{
    return owners;
}

std::string HexFile::pixmap()
{
    if (isRead())
        return ":/icones/ram.png";
    else
        return ":/icones/content-download.png";
}

QString HexFile::fullName()
{
    return fullHexName;
}

QString HexFile::toString()
{
    QString str = "HexFile* (" + QString(name) + " )" ;
    return str;
}

PROJECT *HexFile::getA2lFileProject()
{
    return a2lProject;
}

void HexFile::setFullName(QString fullName)
{
    fullHexName = fullName;
    WorkProject *wp = getParentWp();
    wp->rename(this);

    // change the name displayed into owners formcompare
    foreach(QObject *obj, owners)
    {
        QString type = typeid(*obj).name();
        if (type.toLower().endsWith("formcompare"))
        {
            FormCompare *fcomp = (FormCompare*)obj;
            if (fcomp->getHex1() == this)
            {
                 QString str = wp->getFullNodeName()
                               + "/"
                               + QFileInfo(fullHexName).fileName();                
                 fcomp->setDataset1(str);
            }
            else if (fcomp->getHex2() == this)
            {
                QString str = wp->getFullNodeName()
                              + "/"
                              + QFileInfo(fullHexName).fileName();
                 fcomp->setDataset2(str);
            }
        }
    }
}

void HexFile::incrementValueProgBar(int n)
{
    valueProgBar += n;

    if (valueProgBar < maxValueProgbar)
    {
        omp_set_lock(&lock);

        if (valueProgBar / maxValueProgbar < 0.98)
            emit progress(valueProgBar, maxValueProgbar);
        else
            emit progress(maxValueProgbar, maxValueProgbar);

        omp_unset_lock(&lock);
    }

}

unsigned int HexFile::tzn(unsigned int v)
{
    unsigned int c;     // c will be the number of zero bits on the right,
                    // so if v is 1101000 (base 2), then c will be 3
    // NOTE: if 0 == v, then c = 31.
    if (v & 0x1)
    {
      // special case for odd v (assumed to happen half of the time)
      c = 0;
    }
    else
    {
      c = 1;
      if ((v & 0xffff) == 0)
      {
        v >>= 16;
        c += 16;
      }
      if ((v & 0xff) == 0)
      {
        v >>= 8;
        c += 8;
      }
      if ((v & 0xf) == 0)
      {
        v >>= 4;
        c += 4;
      }
      if ((v & 0x3) == 0)
      {
        v >>= 2;
        c += 2;
      }
      c -= v & 0x1;
    }

    return c;
}

QString HexFile::dec2hex(double dec, std::string type)
{
    QString qHex = "";
    int E = (int)dec;
    char hex[31];

    if(type == "SBYTE")
    {
        if ((0.5 <= dec - E && dec >= 0 )|| (dec - E > -0.5 && dec < 0))
        {
            if (E < CHAR_MAX)
            {
                sprintf(hex, "%X", (int)ceil(dec));

            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > CHAR_MIN)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }

        qHex = QString(hex).right(2);
        while (qHex.count() < 2)
            qHex = "0" + qHex;
    }
    else if(type == "UBYTE")
    {
        if (0.5 <= dec - E)
        {
            if (E < UCHAR_MAX)
            {
                sprintf(hex, "%X", (int)ceil(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > 0)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        qHex = QString(hex).right(2);
        while (qHex.count() < 2)
            qHex = "0" + qHex;
    }
    else if(type == "SWORD")
    {
        if ((0.5 <= dec - E && dec >=0 )|| (dec - E > -0.5 && dec < 0))
        {
            if (E < SHRT_MAX)
            {
                sprintf(hex, "%X", (int)ceil(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > SHRT_MIN)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        qHex = QString(hex).right(4);
        while (qHex.count() < 4)
            qHex = "0" + qHex;
    }
    else if(type == "UWORD")
    {
        if (0.5 <= dec - E)
        {
            if (E < USHRT_MAX)
            {
                sprintf(hex, "%X", (int)ceil(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > 0)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        qHex = QString(hex).right(4);
        while (qHex.count() < 4)
            qHex = "0" + qHex;
    }
    else if(type == "SLONG")
    {
        if ((0.5 <= dec - E && dec >=0 )|| (dec - E > -0.5 && dec < 0))
        {
            if (E < INT_MAX)
            {
                sprintf(hex, "%X", (int)ceil(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > INT_MIN)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        qHex = QString(hex).right(8);
        while (qHex.count() < 8)
            qHex = "0" + qHex;
    }
    else if(type == "ULONG")
    {
        E = (unsigned int)dec;
        if (0.5 <= dec - E)
        {
            if (E < (unsigned int)UINT_MAX)
            {
                sprintf(hex, "%X", (unsigned int)ceil(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        else
        {
            if (E > 0)
            {
                sprintf(hex, "%X", (int)floor(dec));
            }
            else
            {
                sprintf(hex, "%X", E);
            }
        }
        qHex = QString(hex).right(8);
        while (qHex.count() < 8)
            qHex = "0" + qHex;
    }
    else if(type == "FLOAT32_IEEE")
    {
        float f = (float)dec;
        sprintf(hex, "%08X", *(int*)&f);
        qHex = QString(hex);
    }

    return qHex;
}
