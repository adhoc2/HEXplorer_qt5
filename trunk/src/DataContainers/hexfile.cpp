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
#include <qtconcurrentrun.h>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDomDocument>
#include <QTime>

#include "Nodes/characteristic.h"
#include "Nodes/compu_method.h"
#include "Nodes/mod_common.h"
#include "Nodes/axis_pts.h"
#include "Nodes/function.h"
#include "data.h"
#include "graphverify.h"
#include "formcompare.h"
#include "stdint.h"

#define CAST2(val, type) *(type*)&val

// ______________ class MemBlock _________________//

bool compare(Data *a, Data *b)
{
   if (a->getName() < b->getName())
       return true;
   else return false;
}

MemBlock::~MemBlock()
{
    delete data;
}

// _______________ class HexFile Ctor/Dtor___________________//

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

    //get the byte_order   
    MOD_COMMON *modCommon = (MOD_COMMON*)a2lProject->getNode("MODULE/" + getModuleName() + "/MOD_COMMON");
    if (modCommon)
    {
        Byte_Order *item = (Byte_Order*)modCommon->getItem("BYTE_ORDER");
        if (item)
            byteOrder = item->getPar("ByteOrder");

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
            nByte.insert("FLOAT32_IEEE", 4);
        }
        else
        {
            nByte.insert("FLOAT32_IEEE", 4);
        }
    }
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
    }
    return false;
}

bool HexFile::parseFile()
{
    // open file
     QFile file(fullHexName);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return false;

     // stream,read the file and save the strings into a QStringList
     QTextStream in(&file);
     QStringList lines;
     QList<int> listStartMemBlock;
     int i = 0;
     while (!in.atEnd())
     {
        QString str = in.readLine();
        bool ok;
        if (str.startsWith(":") || (str.startsWith(26)))
        {
            lines << str;            
            int type = str.mid(7,2).toInt(&ok, 16);
            if (type == 4)
            {
                listStartMemBlock.append(i);
            }
        }
        else
        {
            QMessageBox::critical(0, "HEXplorer :: HexFile", "wrong Hex file format.");
            return false;
        }

        i++;
     }
     fileLinesNum = lines.count();     

     //get the length for the progressBar
     A2LFILE *a2l = (A2LFILE*)this->getParentNode();
     MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + getModuleName());
     int length = module->listChar.count();
     maxValueProgbar = fileLinesNum + length;

     bool myDebug = 0;
#ifdef MY_DEBUG
 myDebug = 1;
#endif
     if (omp_get_num_procs() > 1 && !myDebug)
     {
         //divide file into 2 distinct parts for parallel sections
         int _mid = listStartMemBlock.size() / 2;
         int _midLine = listStartMemBlock.at(_mid);
         QStringList lines1 = lines.mid(0, _midLine);
         lines1.append(":00000001FF");
         QStringList lines2 = lines.mid(_midLine, fileLinesNum - _midLine);

         // parse the file into 2 threads
         omp_set_num_threads(2);
         QList<MemBlock*> blockList1;
         QList<MemBlock*> blockList2;
         #pragma omp parallel
         {
            #pragma omp sections
             {
                #pragma omp section
                 {
                     // parse the file
                     int dataCnt = 0;
                     int lineLength = 0;
                     bool firstLineOfBlock = false;
                     int type = 0;
                     QString reclen = "";
                     MemBlock *actBlock = 0;
                     int cnt = 0;
                     int previous = 0;
                     while (cnt < lines1.count())
                     {
                         incrementValueProgBar(cnt - previous);
                         previous = cnt;

                         bool ok;
                         type = lines1.at(cnt).mid(7,2).toInt(&ok, 16);
                         reclen = lines1.at(cnt).mid(1,2);

                         switch (type)
                         {
                            case 0: //Data Record
                                {
                                    //create a new memory block
                                    if (firstLineOfBlock)
                                    {
                                        actBlock = new MemBlock();
                                        actBlock->offset = lines1.at(cnt - 1).mid(9, 4);
                                        actBlock->start = (actBlock->offset + lines1.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                                        int end = (actBlock->offset + "FFFF").toUInt(&ok, 16);
                                        actBlock->lineLength = reclen.toInt(&ok, 16);

                                        actBlock->data = new unsigned char [(end - actBlock->start + 1)];
                                        firstLineOfBlock = false;
                                    }

                                    //read all the lines1 of the memory block
                                    //unsigned int addr = (actBlock->offset + lines1.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                                    while (type == 0)
                                    {
                                        lineLength = lines1.at(cnt).mid(1,2).toInt(&ok, 16);
                                        dataCnt = (actBlock->offset + lines1.at(cnt).mid(3, 4)).toUInt(&ok, 16) - actBlock->start;

                                        for (int i = 0; i < lineLength; i++)
                                        {
                                            QString str = lines1.at(cnt).mid(9 + 2 * i, 2);
                                            bool bl;
                                            actBlock->data[dataCnt] = str.toUShort(&bl, 16);

                                            //actBlock->addrList.append(addr);
                                            //actBlock->hexList.append(str);

                                            dataCnt++;
                                        }

                                        cnt++;
                                        type = lines1.at(cnt).mid(7, 2).toInt(&ok, 16);
                                        reclen = lines1.at(cnt).mid(1, 2);
                                    }

                                    //set blockend
                                    actBlock->end = actBlock->start + dataCnt - 1;
                                    actBlock->length = actBlock->end - actBlock->start + 1;

                                    //add block to the list of block
                                    blockList1.append(actBlock);
                                }

                                break;

                            case 1: //End of File
                                cnt = lines1.count();
                                break;

                            case 2: //Extended segment address record
                                cnt++;
                                break;

                            case 3: //Start Segment address record
                                cnt++;
                                break;

                            case 4: // Extended Linear Address Record
                                firstLineOfBlock = true;
                                cnt++;
                                break;

                            case 5: // Start Linear Address record
                                cnt++;
                                break;

                            default:
                                break;
                         }
                     }
                     incrementValueProgBar(cnt - previous);
                 }

                #pragma omp section
                 {
                     // parse the file
                     int dataCnt = 0;
                     int lineLength = 0;
                     bool firstLineOfBlock = false;
                     int type = 0;
                     QString reclen = "";
                     MemBlock *actBlock = 0;
                     int cnt = 0;
                     int previous = 0;
                     while (cnt < lines2.count())
                     {
                         incrementValueProgBar(cnt - previous);
                         previous = cnt;

                         bool ok;
                         type = lines2.at(cnt).mid(7,2).toInt(&ok, 16);
                         reclen = lines2.at(cnt).mid(1,2);

                         switch (type)
                         {
                            case 0: //Data Record
                                {
                                    //create a new memory block
                                    if (firstLineOfBlock)
                                    {
                                        actBlock = new MemBlock();
                                        actBlock->offset = lines2.at(cnt - 1).mid(9, 4);
                                        actBlock->start = (actBlock->offset + lines2.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                                        int end = (actBlock->offset + "FFFF").toUInt(&ok, 16);
                                        actBlock->lineLength = reclen.toInt(&ok, 16);

                                        actBlock->data = new unsigned char [(end - actBlock->start + 1)];
                                        firstLineOfBlock = false;
                                    }

                                    //read all the lines2 of the memory block
                                    //unsigned int addr = (actBlock->offset + lines2.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                                    while (type == 0)
                                    {
                                        lineLength = lines2.at(cnt).mid(1,2).toInt(&ok, 16);
                                        dataCnt = (actBlock->offset + lines2.at(cnt).mid(3, 4)).toUInt(&ok, 16) - actBlock->start;

                                        for (int i = 0; i < lineLength; i++)
                                        {
                                            QString str = lines2.at(cnt).mid(9 + 2 * i, 2);
                                            bool bl;
                                            actBlock->data[dataCnt] = str.toUShort(&bl, 16);

                                            //actBlock->addrList.append(addr);
                                            //actBlock->hexList.append(str);

                                            dataCnt++;
                                        }

                                        cnt++;
                                        type = lines2.at(cnt).mid(7, 2).toInt(&ok, 16);
                                        reclen = lines2.at(cnt).mid(1, 2);
                                    }

                                    //set blockend
                                    actBlock->end = actBlock->start + dataCnt - 1;
                                    actBlock->length = actBlock->end - actBlock->start + 1;

                                    //add block to the list of block
                                    blockList2.append(actBlock);
                                }

                                break;

                            case 1: //End of File
                                cnt = lines2.count();
                                break;

                            case 2: //Extended segment address record
                                cnt++;
                                break;

                            case 3: //Start Segment address record
                                cnt++;
                                break;

                            case 4: // Extended Linear Address Record
                                firstLineOfBlock = true;
                                cnt++;
                                break;

                            case 5: // Start Linear Address record
                                cnt++;
                                break;

                            default:
                                break;
                         }
                     }
                     incrementValueProgBar(cnt - previous);
                 }
            }
        }

         // merge the blockList
         blockList.append(blockList1);
         blockList.append(blockList2);
     }
     else
     {
         // parse the file
         int dataCnt = 0;
         int lineLength = 0;
         bool firstLineOfBlock = false;
         int type = 0;
         QString reclen = "";
         MemBlock *actBlock = 0;
         int cnt = 0;
         int previous = 0;
         while (cnt < lines.count())
         {
             incrementValueProgBar(cnt - previous);
             previous = cnt;

             bool ok;
             type = lines.at(cnt).mid(7,2).toInt(&ok, 16);
             reclen = lines.at(cnt).mid(1,2);

             switch (type)
             {
                case 0: //Data Record
                    {
                        //create a new memory block
                        if (firstLineOfBlock)
                        {
                            actBlock = new MemBlock();
                            actBlock->offset = lines.at(cnt - 1).mid(9, 4);
                            actBlock->start = (actBlock->offset + lines.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                            int end = (actBlock->offset + "FFFF").toUInt(&ok, 16);
                            actBlock->lineLength = reclen.toInt(&ok, 16);

                            actBlock->data = new unsigned char [(end - actBlock->start + 1)];
                            firstLineOfBlock = false;
                        }

                        //read all the lines of the memory block
                        //unsigned int addr = (actBlock->offset + lines.at(cnt).mid(3, 4)).toUInt(&ok, 16);
                        while (type == 0)
                        {
                            lineLength = lines.at(cnt).mid(1,2).toInt(&ok, 16);
                            dataCnt = (actBlock->offset + lines.at(cnt).mid(3, 4)).toUInt(&ok, 16) - actBlock->start;

                            for (int i = 0; i < lineLength; i++)
                            {
                                QString str = lines.at(cnt).mid(9 + 2 * i, 2);
                                bool bl;
                                actBlock->data[dataCnt] = str.toUShort(&bl, 16);

                                //actBlock->addrList.append(addr);
                                //actBlock->hexList.append(str);

                                dataCnt++;
                            }

                            cnt++;
                            type = lines.at(cnt).mid(7, 2).toInt(&ok, 16);
                            reclen = lines.at(cnt).mid(1, 2);
                        }

                        //set blockend
                        actBlock->end = actBlock->start + dataCnt - 1;
                        actBlock->length = actBlock->end - actBlock->start + 1;

                        //add block to the list of block
                        blockList.append(actBlock);
                    }

                    break;

                case 1: //End of File
                    cnt = lines.count();
                    break;

                case 2: //Extended segment address record
                    cnt++;
                    break;

                case 3: //Start Segment address record
                    cnt++;
                    break;

                case 4: // Extended Linear Address Record
                    firstLineOfBlock = true;
                    cnt++;
                    break;

                case 5: // Start Linear Address record
                    cnt++;
                    break;

                default:
                    break;
             }
         }
         incrementValueProgBar(cnt - previous);
     }

    return true;

 }

bool HexFile::isA2lCombined()
{
    //get the address of Bosch number in A2l
    MOD_PAR *modePar = (MOD_PAR*)a2lProject->getNode("MODULE/" + getModuleName() + "/MOD_PAR");

    if (modePar)
    {
        ADDR_EPK *addr_epk = (ADDR_EPK*)modePar->getItem("addr_epk");
        if (addr_epk)
        {
            QString address = addr_epk->getPar("Address");
            if (isValidAddress(address))
            {
                QStringList hexVal = getHexValues(address, 0, 1, 90);

                QString str = "";
                double c;
                for (int i = 0; i < hexVal.count(); i++)
                {
                    QString val = hexVal.at(i);
                    bool bl;
                    c = val.toUInt(&bl,16);
                    if (32 <= c && c < 127)
                        str.append((unsigned char)c);
                }

                EPK *epk = (EPK*)modePar->getItem("epk");
                if (epk)
                {
                    QString str1 = epk->getPar("Identifier");
                    str1.remove('\"');
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
            else
                return true;
        }
        else
            return true;
    }
    else
        return true;
}

void HexFile::readAllData()
{
    bool phys = true;

    //empty the list
    listData.clear();

    //create list
    A2LFILE *a2l = (A2LFILE*)this->getParentNode();
    MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + getModuleName());

    //read labels
    Node *nodeChar = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
    Node *nodeAxis = a2l->getProject()->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
    if (nodeChar)
    {
        int length = module->listChar.count();
        bool myDebug = 0;
#ifdef MY_DEBUG
    myDebug = 1;
#endif
        if (length > 5000 && omp_get_num_procs() > 1 && !myDebug)
        {
            // split listChar into 2 lists            
            int middle = 0;
            if (length % 2 == 0)
                middle = length / 2;
            else
                middle = (int)(length/2);

            QStringList list1 = module->listChar.mid(0, middle);
            QStringList list2 = module->listChar.mid(middle, length - middle);


            // read datas
            QList<Data*> listData1;
            QList<Data*> listData2;

            omp_set_num_threads(2);
            #pragma omp parallel
            {
                #pragma omp sections
                {
                    #pragma omp section
                    {
                        int i = 0;
                        foreach (QString str, list1)
                        {
                            bool found = false;

                            // search into CHARACTERISTIC
                            if (nodeChar)
                            {
                                Node *label = nodeChar->getNode(str);
                                if (label)
                                {
                                    found = true;
                                    CHARACTERISTIC *charac = (CHARACTERISTIC*)label;
                                    QString add = charac->getPar("Adress");
                                    bool bl = isValidAddress(add);

                                    if(bl)
                                    {
                                        Data *data = new Data(charac, a2lProject, this);
//                                        if (phys)
//                                            data->hex2phys();
                                        listData1.append(data);
                                    }
                                    else
                                    {

                                    }
                                }
                            }

                            // search into AXIS_PTS
                            if (nodeAxis && !found)
                            {
                                Node *label2 = nodeAxis->getNode(str);
                                if (label2)
                                {
                                    found = true;
                                    AXIS_PTS *axis = (AXIS_PTS*)label2;
                                    QString add = axis->getPar("Adress");

                                    bool bl = isValidAddress(add);
                                    if (bl)
                                    {
                                        Data *data = new Data(axis, a2lProject, this);
                                        if (phys)
                                            data->hex2phys();
                                        listData1.append(data);
                                    }
                                    else
                                    {

                                    }
                                }
                            }

                            // increment valueProgBar
                            if (i % 6 == 1)
                                incrementValueProgBar(6);

                            i++;

                        }
                    }
                    #pragma omp section
                    {
                        int i = 0;
                        foreach (QString str, list2)
                        {

                            bool found = false;
                            // search into CHARACTERISTIC
                            if (nodeChar)
                            {
                                Node *label = nodeChar->getNode(str);
                                if (label)
                                {
                                    found = true;
                                    CHARACTERISTIC *charac = (CHARACTERISTIC*)label;
                                    QString add = charac->getPar("Adress");
                                    bool bl = isValidAddress(add);

                                    if(bl)
                                    {
                                        Data *data = new Data(charac, a2lProject, this);
//                                        if (phys)
//                                            data->hex2phys();
                                        listData2.append(data);
                                    }
                                    else
                                    {

                                    }
                                }
                            }

                            // search into AXIS_PTS
                            if (nodeAxis && !found)
                            {
                                Node *label2 = nodeAxis->getNode(str);
                                if (label2)
                                {
                                    found = true;
                                    AXIS_PTS *axis = (AXIS_PTS*)label2;
                                    QString add = axis->getPar("Adress");

                                    bool bl = isValidAddress(add);
                                    if (bl)
                                    {
                                        Data *data = new Data(axis, a2lProject, this);
                                        if (phys)
                                            data->hex2phys();
                                        listData2.append(data);
                                    }
                                    else
                                    {

                                    }
                                }
                            }


                            // increment valueProgBar
                            if (i % 6 == 1)
                                incrementValueProgBar(6);

                            i++;
                        }
                    }
                }
            }

            listData.append(listData1);
            listData.append(listData2);

        }
        else
        {
            int i = 0;
            foreach (QString str, module->listChar)
            {
                bool found = false;

                // search into CHARACTERISTIC
                if (nodeChar)
                {
                    Node *label = nodeChar->getNode(str);
                    if (label)
                    {
                        found = true;
                        CHARACTERISTIC *charac = (CHARACTERISTIC*)label;
                        QString add = charac->getPar("Adress");
                        bool bl = isValidAddress(add);

                        if(bl)
                        {
                            Data *data = new Data(charac, a2lProject, this);
//                            if (phys)
//                                data->hex2phys();
                            listData.append(data);
                        }
                        else
                        {

                        }
                    }
                }

                // search into AXIS_PTS
                if (nodeAxis && !found)
                {
                    Node *label2 = nodeAxis->getNode(str);
                    if (label2)
                    {
                        found = true;
                        AXIS_PTS *axis = (AXIS_PTS*)label2;
                        QString add = axis->getPar("Adress");

                        bool bl = isValidAddress(add);
                        if (bl)
                        {
                            Data *data = new Data(axis, a2lProject, this);
                            if (phys)
                                data->hex2phys();
                            listData.append(data);
                        }
                        else
                        {

                        }
                    }
                }

                // display not found
                if (!found)
                {

                }

                // increment valueProgBar
                if (i % 6 == 1)
                    incrementValueProgBar(6);

                i++;
            }
        }
    }
}

// ________________ read Hex values___________________ //

QString HexFile::getHexValue(QString address, int offset,  int nByte)
{
    QTime time;
    time.start();

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
    int line = IAddr - blockList[block]->start;

    //read the byte in QList
    QList<unsigned char> tab;
    if (line + nByte < blockList[block]->length)
    {
        for (int i = 0; i < nByte; i++)
            tab.append( blockList[block]->data[line + i]);
    }
    else
    {
        int size = blockList[block]->length - line;
        int i = 0;
        for (i = 0; i < size; i++)
            tab.append(blockList[block]->data[line + i]);
        for (int j = 0; j < nByte - size; j++)
            tab.append(blockList[block + 1]->data[j]);
    }

    //MSB_FIRST or MSB_LAST
    QString str = "ZZ";
    if (byteOrder.toLower() == "msb_first")
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
    else if (byteOrder.toLower() == "msb_last")
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

QStringList HexFile::getHexValues(QString address, int offset, int nByte, int count)
{
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
    QStringList hexList;
    if (byteOrder.toLower() == "msb_first")
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
    else if (byteOrder.toLower() == "msb_last")
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

QList<double> HexFile::getDecValues(double IAddr, int nByte, int count, std::string type)
{
    //find block and line
    //bool bl;
    //unsigned int IAddr =address.toUInt(&bl, 16);
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

    QList<double> decList;
    if (byteOrder.toLower() == "msb_last")
    {
        if(type == "SBYTE")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], int8_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int8_t));
                }
            }
        }
        if(type == "UBYTE")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], uint8_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], uint8_t));
                }
            }
        }
        else if(type == "SWORD")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], int16_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int16_t));
                }
            }

        }
        else if(type == "UWORD")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], uint16_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], uint16_t));
                }
            }

        }
        else if(type == "SLONG")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], int32_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int32_t));
                }
            }
        }
        else if(type == "ULONG")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], int32_t));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], int32_t));
                }
            }
        }
        else if(type == "FLOAT32_IEEE")
        {
            if (line + nByte*count < blockList[block]->length)
            {
                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(blockList[block]->data[line + nByte*i], double));
                }
            }
            else
            {
                char *buffer = new char[nByte*count];
                int size = blockList[block]->length - line;
                for (int i = 0; i < size; i++)
                    buffer[i] = blockList[block]->data[line + i];
                for (int j = 0; j < nByte*count - size; j++)
                    buffer[size + j] = blockList[block + 1]->data[j];

                for (int i = 0; i < count; i++)
                {
                    decList.append(CAST2(buffer[nByte*i], double));
                }
            }
        }
    }
    else
    {

    }

    return decList;
}

bool HexFile::isValidAddress(QString address)
{
    bool bl;
    unsigned int IAddr =address.toUInt(&bl, 16);

    int block = 0;
    while (block < blockList.count())
    {
        if ((blockList[block]->start <= IAddr) && (IAddr <= blockList[block]->end))
        {
            break;
        }
        block++;
    }

    if (block >=  blockList.count())
        return false;
    else
        return true;
}

int HexFile::getNumByte(std::string str)
{
    return nByte.value(str);
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

void HexFile::setValue(unsigned int IAddr, QString hex,  int nByte)
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
    if (byteOrder.toLower() == "msb_first")
    {
        QString str;
        for (int i = 0; i < nByte; i++)
        {
            str = hex.mid(i * 2, 2);
            tab.append(str.toUShort(&bl, 16));
        }
    }
    else if (byteOrder.toLower() == "msb_last")
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

void HexFile::setValues(unsigned int IAddr, QStringList hexList, int nByte)
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
    if (byteOrder.toLower() == "msb_first")
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
    else if (byteOrder.toLower() == "msb_last")
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
            int nbyte = data->getZ(0).count() / 2;
            setValue(data->getAddressZ(), data->getZ(0), nbyte);
        }
        else if(type.toLower() == "curve")
        {
            //axisX
            int nbyteX = data->getX(0).count() / 2;
            setValues(data->getAddressX(), data->getX(), nbyteX);

            //axisZ
            if (!data->isSizeChanged())
            {
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(data->getAddressZ(), data->getZ(), nbyteZ);
            }
            else
            {
                // write new length of axis X into HexFile
                bool bl;
                double addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                QString length = data->getnPtsXHexa();
                int nbyteNPtsX = length.length() / 2;
                setValue(addr, length, nbyteNPtsX);

                //calculate new Address Z due to axisX length modification
                double newAddressZ = data->getAddressX() + nbyteX * data->xCount();

                //write the Z hex values
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(newAddressZ, data->getZ(), nbyteZ);

            }
        }
        else if(type.toLower() == "map")
        {
            //axisX
            int nbyteX = data->getX(0).count() / 2;
            setValues(data->getAddressX(), data->getX(), nbyteX);

            //axisY
            if (!data->isSizeChanged())
            {
                int nbyteY = data->getY(0).count() / 2;
                setValues(data->getAddressY(), data->getY(), nbyteY);

                //axisZ
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(data->getAddressZ(), data->getZ(), nbyteZ);
            }
            else
            {
                // write new length of axis X into HexFile
                bool bl;
                double addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                QString length = data->getnPtsXHexa();
                int nbyteNPtsX = length.length() / 2;
                setValue(addr, length, nbyteNPtsX);

                // write new length of axis Y into HexFile
                bl;
                addr = QString(node->getPar("Adress")).toUInt(&bl, 16);
                length = data->getnPtsYHexa();
                int nbyteNPtsY = length.length() / 2;
                setValue(addr + nbyteNPtsX, length, nbyteNPtsY);

                //calculate new Address Y due to axisX length modification
                double newAddressY = data->getAddressX() + nbyteX * data->xCount();

                //write the Y hex values
                int nbyteY = data->getY(0).count() / 2;
                setValues(newAddressY, data->getY(), nbyteY);

                //calculate new Address Z due to axisX and axisY length modification
                double newAddressZ = newAddressY + nbyteY * data->yCount();

                //write the Z hex values
                int nbyteZ = data->getZ(0).count() / 2;
                setValues(newAddressZ, data->getZ(), nbyteZ);


            }
        }
        else if (type.toLower() == "val_blk")
        {
            //axisZ
            int nbyteZ = data->getZ(0).count() / 2;
            setValues(data->getAddressZ(), data->getZ(), nbyteZ);
        }
    }
    else
    {
        //axisZ
        int nbyteZ = data->getZ(0).count() / 2;
        setValues(data->getAddressZ(), data->getZ(), nbyteZ);
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
        QString cks = checksum(":02000004" + blockList[i]->offset);
        lineList.append(":02000004" + blockList[i]->offset + cks);
        x = 0;
        j = 0;

        bool bl;
        QString start = blockList[i]->offset + "0000";
        int strt = start.toUInt(&bl, 16);

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

Data* HexFile::readLabel(CHARACTERISTIC *label, bool phys)
{
    Data dat(label);
    QList<Data*>::iterator i = qBinaryFind(listData.begin(), listData.end(), &dat, compare);

    if (i == listData.end())
    {
        Data *data = new Data(label, a2lProject, this);

        if (phys)
           data->hex2phys();

        listData.append(data);
        qSort(listData.begin(), listData.end(), compare);

        return data;
    }
    else
        return *i;
}

Data* HexFile::readLabel(AXIS_PTS *label, bool phys)
{
    Data dat(label);
    QList<Data*>::iterator i = qBinaryFind(listData.begin(), listData.end(), &dat, compare);

    if (i == listData.end())
    {
        Data *data = new Data(label, a2lProject, this);

        if (phys)
           data->hex2phys();

        listData.append(data);
        qSort(listData.begin(), listData.end(), compare);

        return data;
    }
    else
        return *i;
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
            this->attach(wp);
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
        QSettings settings;
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
        QSettings settings;
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
    }

    //check monotony of each column
    QList<int> list;
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

    return list;
}

void HexFile::attach(QObject *o)
{
    //check owner for validity
    if(o==0)
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
    //owners.removeAll(o);
    owners.removeOne(o);

    //remove self after last one
    if(owners.size()==0)
        delete this;
}

std::string HexFile::pixmap()
{
    return ":/icones/ram.png";
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

    // change the name displayed into owners
    foreach(QObject *obj, owners)
    {
        QString type = typeid(*obj).name();
        if (type.toLower().endsWith("formcompare"))
        {
            FormCompare *fcomp = (FormCompare*)obj;
            if (fcomp->getHex1() == this)
            {
                 QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                               + "/"
                               + QFileInfo(fullHexName).fileName();
                 fcomp->setDataset1(str);
            }
            else if (fcomp->getHex2() == this)
            {
                QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                              + "/"
                              + QFileInfo(fullHexName).fileName();
                 fcomp->setDataset2(str);
            }
        }
    }
}

void HexFile::incrementValueProgBar(int n)
{
    omp_set_lock(&lock);

    valueProgBar += n;
    emit progress(valueProgBar, maxValueProgbar);

    omp_unset_lock(&lock);
}
