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

#include "dcmfile.h"

#include "data.h"
#include "lexer.h"
#include "lexerDcm.h"
#include "formcompare.h"
#include "qdebug.h"
#include "spreadsheetview.h"
#include "sptablemodel.h"
#include <QtCore/qmath.h>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QDomDocument>
#include <QHeaderView>
#include <QApplication>
#include <typeinfo>

bool compDcm(Data *a, Data *b)
{
   if (a->getName() < b->getName())
       return true;
   else return false;
}

// ___________ Dcm Ctor/Dtor __________ //

Dcm::Dcm(QString fullDcmFileName, WorkProject *parentWP, QString modName, QObject *parent)
    : QObject(parent) , DataContainer(parentWP, modName)
{
    a2lProject = (PROJECT*)getParentWp()->a2lFile->getProject();
    fullPath = fullDcmFileName;
    name = new char[(QFileInfo(fullPath).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(fullPath).fileName()).toLocal8Bit().data());
    maxValueProgbar = 0;
    valueProgBar = 0;
    omp_init_lock(&lock);

    //get the byte_order
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
            nByte.insert("FLOAT32_IEEE", 4);
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


}

Dcm::~Dcm()
{
    omp_destroy_lock(&lock);
    delete[] name;
}


// ____________ read / write ____________ //

bool Dcm::readFile()
{
    QFile file(fullPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QString str;
    QTextStream in;

    QTextStream in1(&file);
    str = in1.readAll();
    file.close();
    in.setString(&str);

    //set progressBar max value
    maxValueProgbar = str.length();

    //Parser
    QStringList outList;
    LexerDcm mylex;
    connect(&mylex, SIGNAL(returnedToken(int)), this, SLOT(checkProgressStream(int)), Qt::DirectConnection);

    //parse the QTextstream
    TokenTyp token;
    token = mylex.getNextToken(in);

    //remove comments
    while(token == Comment)
    {
        token = mylex.getNextToken(in);
        //qDebug() << mylex.toString(token).c_str() <<  mylex.getLexem().c_str();

    }

    //check the version od Dcm format > 2.0
    if (token == Keyword && mylex.getLexem() == "KONSERVIERUNG_FORMAT")
    {
        token = mylex.getNextToken(in);
        qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
        if (token == Float)
        {
            double version = QString(mylex.getLexem().c_str()).toDouble();
            if (version < 2)
                return false;
        }
    }
    else
    {
        return false;
    }

    token = mylex.getNextToken(in);
    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

    //parse the datas
    while (!in.atEnd())
    {        
        //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

        if (token == Keyword && mylex.getLexem() == "FUNKTIONEN")
        {
            while(token != Keyword || mylex.getLexem() != "END")
            {
                token = mylex.getNextToken(in);
            }
            token = mylex.getNextToken(in);
        }
        else if (token == Keyword && mylex.getLexem() == "FESTWERT")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                bool ok = false;
                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "WERT" && mylex.getLexem() != "TEXT"))
                    {
                        token = mylex.getNextToken(in);
                    }

                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

                    if (token == Keyword && ( mylex.getLexem() == "WERT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                        }

                        // copy z values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "z");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "z");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendZ(phys);
                        }

                    }
                    else if (token == Keyword && ( mylex.getLexem() == "TEXT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  String)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                        }

                        // copy z values to data
                        foreach (QString str, list)
                        {
                            //qDebug() << str;

                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "z");

                            //qDebug() << hex;

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "z");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendZ(phys);
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }


            }
        }
        else if (token == Keyword && mylex.getLexem() == "TEXTSTRING")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                bool ok = false;
                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "WERT" && mylex.getLexem() != "TEXT"))
                    {
                        token = mylex.getNextToken(in);
                    }

                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

                    if (token == Keyword && ( mylex.getLexem() == "WERT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                        }

                        // copy z values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "z");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "z");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendZ(phys);
                        }

                    }
                    else if (token == Keyword && ( mylex.getLexem() == "TEXT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        token = mylex.getNextToken(in);
                        QString text;
                        if (token ==  String)
                        {
                            text = mylex.getLexem().c_str();
                            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                            token = mylex.getNextToken(in);
                        }                   

                        // convert text to char array
                        int asciiVal;
                        for(int i = 0; i < data->xCount(); i++)
                        {
                            if (i < text.length())
                            {
                                asciiVal = text.at(i).toLatin1();
                                data->appendZ(QString::number(asciiVal));
                            }
                            else
                            {
                                data->appendZ("0");
                            }
                        }

                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }


            }
        }
        else if (token == Keyword && mylex.getLexem() == "FESTWERTEBLOCK")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "WERT" && mylex.getLexem() != "TEXT"))
                    {
                        token = mylex.getNextToken(in);
                    }

                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();


                    // in case a label is twice in the subset!!
                    data->clearZ();

                    while (token == Keyword && ( mylex.getLexem() != "END"))
                    {
                        if (token == Keyword && ( mylex.getLexem() == "WERT"))
                        {
                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy z values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendZ(phys);
                            }

                        }
                        else if (token == Keyword && ( mylex.getLexem() == "TEXT"))
                        {

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  String)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy z values to data
                            foreach (QString str, list)
                            {
                                //qDebug() << str;

                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "z");

                                //qDebug() << hex;

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendZ(phys);
                            }
                        }
                        else
                        {
                            return false;
                        }
                    }

                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "KENNLINIE")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "ST/X" && mylex.getLexem() != "ST_TX/X"))
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis X values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    if (token == Keyword && ( mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearX();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }

                        // copy x values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "x");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "x");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendX(phys);
                        }

                    }
                    else
                    {
                        return false;
                    }

                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    if (token == Keyword && ( mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }

                        // copy x values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "z");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "z");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendZ(phys);
                        }

                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "FESTKENNLINIE")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "ST/X" && mylex.getLexem() != "ST_TX/X"))
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis X values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    if (token == Keyword && ( mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearX();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }

                        // copy x values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "x");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "x");

                            // copy PHYS value into Zaxis
                            //qDebug() << str << hex << phys;
                            data->appendX(phys);
                        }

                    }
                    else
                    {
                        return false;
                    }

                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    if (token == Keyword && ( mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT"))
                    {
                        // in case a label is twice in the subset!!
                        data->clearZ();

                        // read the Z values
                        QStringList list;
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            list.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }

                        // copy x values to data
                        foreach (QString str, list)
                        {
                            // convert PHYS value from Dcm into HEX value
                            QString hex = data->phys2hex(str, "z");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "z");

                            // copy PHYS value into Zaxis
                            //qDebug() << phys;
                            data->appendZ(phys);
                        }

                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "GRUPPENKENNLINIE")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }
                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "ST/X" && mylex.getLexem() != "ST_TX/X"))
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis X values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    data->clearX();
                    QStringList listX;
                    while (token == Keyword && ( mylex.getLexem() == "ST/X" ||mylex.getLexem() == "ST_TX/X" ))
                    {
                        // read the Z values
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            listX.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }
                    }

                    // copy x values to data
                    foreach (QString str, listX)
                    {
                        // convert PHYS value from Dcm into HEX value
                        QString hex = data->phys2hex(str, "x");

                        // convert HEX value into PHYS value
                        QString phys = data->hex2phys(hex, "x");

                        // copy PHYS value into Zaxis
                        //qDebug() << phys;
                        data->appendX(phys);
                    }


                    //read Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    // in case a label is twice in the subset!!
                    data->clearY();
                    data->clearZ();
                    while (token == Keyword && ( mylex.getLexem() != "END"))
                    {
                        if (token == Keyword && ( mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT"))
                        {

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float || token == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                                if (mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT")
                                {
                                    token = mylex.getNextToken(in);
                                }
                            }

                            // copy x values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendZ(phys);
                            }

                        }
                        else
                        {
                            return false;
                        }
                    }

                    // create the x-points if it is a curve
                    AXIS_DESCR *axisDescrX = data->getAxisDescrX();
                    if (axisDescrX)
                    {
                        int nCols = data->xCount();
                        data->clearX();
                        QString typeAxisX = axisDescrX->getPar("Attribute");
                        if (typeAxisX == "FIX_AXIS")
                        {
                            //OFFSET, SHIFT and NUMBERAPO
                            FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)axisDescrX->getItem("FIX_AXIS_PAR");
                            QString off = fixAxisPar->getPar("Offset");
                            QString sft = fixAxisPar->getPar("Shift");
                            QString napo = fixAxisPar->getPar("Numberapo");
                            bool bl;
                            int offset = off.toInt(&bl, 10);
                            int shift = sft.toInt(&bl, 10);
                            int nPtsX = napo.toUInt(&bl, 10);

                            //check if nPts < nPtsmax
                            QString maxAxisPts = axisDescrX->getPar("MaxAxisPoints");
                            double nmaxPts = maxAxisPts.toDouble();
                            if (nPtsX > nmaxPts)
                                nPtsX = nmaxPts;

                            QString str;
                            for (int i = 0; i < nPtsX; i++)
                            {
                                str.setNum((int)(offset + i * qPow(2, shift)), 16);
                                data->appendX(data->hex2phys(str, "x"));
                            }
                        }
                        else
                        {
                            for (int i = 0; i < nCols; i++)
                            {
                                data->appendX(QString::number(i));
                            }
                        }
                    }

                    // define size (lines)
                    data->updateSize();
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "KENNFELD")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "ST/X" && mylex.getLexem() != "ST_TX/X"))
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis X values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    data->clearX();
                    QStringList listX;
                    while (token == Keyword && ( mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X" ))
                    {

                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            listX.append(str);
                            token = mylex.getNextToken(in);
                        }
                    }

                    // copy x values to data
                    foreach (QString str, listX)
                    {
                        // convert PHYS value from Dcm into HEX value
                        QString hex = data->phys2hex(str, "x");

                        // convert HEX value into PHYS value
                        QString phys = data->hex2phys(hex, "x");

                        // copy PHYS value into Zaxis
                        //qDebug() << phys;
                        data->appendX(phys);
                    }


                    //read Y and Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    // in case a label is twice in the subset!!
                    data->clearY();
                    data->clearZ();
                    while (token == Keyword && ( mylex.getLexem() != "END"))
                    {
                        if (token == Keyword && ( mylex.getLexem() == "WERT" || mylex.getLexem() == "TEXT"))
                        {

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float || token == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy x values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendZ(phys);
                            }

                        }
                        else if (token == Keyword && ( mylex.getLexem() == "ST/Y" || mylex.getLexem() == "ST_TX/Y"))
                        {

                            // read the Y values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float || token == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy x values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "y");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "y");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendY(phys);
                            }

                        }
                        else
                        {
                            return false;
                        }
                    }

                    // define size (lines)
                    data->updateSize();
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "GRUPPENKENNFELD")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || mylex.getLexem() != "ST/X")
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis X values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    QStringList listX;
                    data->clearX();
                    while (token == Keyword && ( mylex.getLexem() == "ST/X"))
                    {
                        // read the X values
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float)
                        {
                            QString str = mylex.getLexem().c_str();
                            listX.append(str);
                            token = mylex.getNextToken(in);
                        }
                    }

                    // copy x values to data
                    foreach (QString str, listX)
                    {
                        // convert PHYS value from Dcm into HEX value
                        QString hex = data->phys2hex(str, "x");

                        // convert HEX value into PHYS value
                        QString phys = data->hex2phys(hex, "x");

                        // copy PHYS value into Zaxis
                        //qDebug() << phys;
                        data->appendX(phys);
                    }

//                    else
//                    {
//                        return false;
//                    }

                    //read Y and Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    // in case a label is twice in the subset!!
                    data->clearY();
                    data->clearZ();
                    while (token == Keyword && ( mylex.getLexem() != "END"))
                    {
                        if (token == Keyword && ( mylex.getLexem() == "WERT"))
                        {

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy x values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendZ(phys);
                            }

                        }
                        else if (token == Keyword && ( mylex.getLexem() == "ST/Y"))
                        {

                            // read the Y values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float)
                            {
                                QString str = mylex.getLexem().c_str();
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // copy x values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Dcm into HEX value
                                QString hex = data->phys2hex(str, "y");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "y");

                                // copy PHYS value into Zaxis
                                //qDebug() << phys;
                                data->appendY(phys);
                            }

                        }
                        else
                        {
                            return false;
                        }
                    }

                    // create the x-points if it is a curve
                    AXIS_DESCR *axisDescrX = data->getAxisDescrX();
                    if (axisDescrX)
                    {
                        int nCols = data->xCount();
                        data->clearX();
                        QString typeAxisX = axisDescrX->getPar("Attribute");
                        if (typeAxisX == "FIX_AXIS")
                        {
                            //OFFSET, SHIFT and NUMBERAPO
                            FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)axisDescrX->getItem("FIX_AXIS_PAR");
                            QString off = fixAxisPar->getPar("Offset");
                            QString sft = fixAxisPar->getPar("Shift");
                            QString napo = fixAxisPar->getPar("Numberapo");
                            bool bl;
                            int offset = off.toInt(&bl, 10);
                            int shift = sft.toInt(&bl, 10);
                            int nPtsX = napo.toUInt(&bl, 10);

                            //check if nPts < nPtsmax
                            QString maxAxisPts = axisDescrX->getPar("MaxAxisPoints");
                            double nmaxPts = maxAxisPts.toDouble();
                            if (nPtsX > nmaxPts)
                                nPtsX = nmaxPts;

                            QString str;
                            for (int i = 0; i < nPtsX; i++)
                            {
                                str.setNum((int)(offset + i * qPow(2, shift)), 16);
                                data->appendX(data->hex2phys(str, "x"));
                            }
                        }
                        else
                        {
                            for (int i = 0; i < nCols; i++)
                            {
                                data->appendX(QString::number(i));
                            }
                        }
                    }

                    // create the y-points if it is a map
                    AXIS_DESCR *axisDescrY = data->getAxisDescrY();
                    if (axisDescrY)
                    {
                        int nRows = data->yCount();
                        data->clearY();
                        QString typeAxisY = axisDescrY->getPar("Attribute");
                        if (typeAxisY == "FIX_AXIS")
                        {
                            //OFFSET, SHIFT and NUMBERAPO
                            FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)axisDescrX->getItem("FIX_AXIS_PAR");
                            QString off = fixAxisPar->getPar("Offset");
                            QString sft = fixAxisPar->getPar("Shift");
                            QString napo = fixAxisPar->getPar("Numberapo");
                            bool bl;
                            int offset = off.toInt(&bl, 10);
                            int shift = sft.toInt(&bl, 10);
                            int nPtsX = napo.toUInt(&bl, 10);

                            //check if nPts < nPtsmax
                            QString maxAxisPts = axisDescrX->getPar("MaxAxisPoints");
                            double nmaxPts = maxAxisPts.toDouble();
                            if (nPtsX > nmaxPts)
                                nPtsX = nmaxPts;

                            QString str;
                            for (int i = 0; i < nPtsX; i++)
                            {
                                str.setNum((int)(offset + i * qPow(2, shift)), 16);
                                data->appendY(data->hex2phys(str, "y"));
                            }

                        }
                        else
                        {
                            for (int i = 0; i < nRows; i++)
                            {
                                data->appendY(QString::number(i));
                            }
                        }
                    }

                    // define size (lines)
                    data->updateSize();
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }
        }
        else if (token == Keyword && mylex.getLexem() == "STUETZSTELLENVERTEILUNG")
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();

            if (token == Identifier)
            {
                // create a data if not already created
                Data *data = NULL;
                if (!getData(mylex.getLexem().c_str()))
                {
                    //get the characteristic node from a2l
                    Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
                    Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
                    Node *label = nodeChar->getNode(mylex.getLexem().c_str());
                    if (label)
                    {
                        //create a data
                        data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                        data->isSortedByRow = 1;
                        //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compDcm);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read Dcm file : " + QString(mylex.getLexem().c_str()) + " not found into "
                                           + QString(this->getParentWp()->a2lFile->name));
                        }
                    }
                }
                else
                {
                    data = getData(mylex.getLexem().c_str());
                }

                if (data)
                {
                    //do not use longname, unit,...
                    //qDebug() << "data created";
                    token = mylex.getNextToken(in);
                    while(token != Keyword || (mylex.getLexem() != "ST/X" && mylex.getLexem() != "ST_TX/X"))
                    {
                        token = mylex.getNextToken(in);

                    }

                    //read axis Z values
                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    data->clearZ();
                    QStringList listZ;
                    while (token == Keyword && ( mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X"))
                    {
                        // read the Z values                        
                        token = mylex.getNextToken(in);
                        while (token ==  Integer || token == Float || token == String)
                        {
                            QString str = mylex.getLexem().c_str();
                            listZ.append(str);
                            token = mylex.getNextToken(in);
                            if (mylex.getLexem() == "ST/X" || mylex.getLexem() == "ST_TX/X")
                            {
                                token = mylex.getNextToken(in);
                            }
                        }
                    }

                    // copy z values to data
                    foreach (QString str, listZ)
                    {
                        // convert PHYS value from Dcm into HEX value
                        QString hex = data->phys2hex(str, "z");

                        // convert HEX value into PHYS value
                        QString phys = data->hex2phys(hex, "z");

                        // copy PHYS value into Zaxis
                        //qDebug() << phys;
                        data->appendZ(phys);
                    }

                    // add X values to data
                    data->clearX(); // in case a label is twice in the subset!!
                    for (int i = 0; i < data->zCount(); i++)
                    {
                        data->appendX(QString::number(i));
                    }

                    // define size (lines)
                    data->updateSize();
//                    }
//                    else
//                    {
//                        return false;
//                    }
                }
                else
                {
                    token = mylex.getNextToken(in);
                    while (token != Keyword ||  mylex.getLexem() != "END")
                    {
                       token = mylex.getNextToken(in);
                    }
                }

                //check for Keyword END
                if (token == Keyword && ( mylex.getLexem() == "END"))
                {

                    //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
                    token = mylex.getNextToken(in);
                }

            }

        }
        else if (token == Comment)
        {
            token = mylex.getNextToken(in);
            //qDebug() << mylex.toString(token).c_str() << mylex.getLexem().c_str();
        }
        else
        {
            return true;
        }
    }

    //write result in outPut list
    this->getParentWp()->parentWidget->writeOutput(outList);
    return true;

}

bool Dcm::save(QString fileName)
{

    if (fileName == "")
    {
      if (exportDataList2Dcm(getListNameData(), this->fullName()))
      {
          //remove childrens nodes
          foreach (Data* data, modifiedData)
          {
              data->clearOldValues();
              removeModifiedData(data);
          }

          return true;
      }
      else
          return false;
    }
    else
    {
        if (exportDataList2Dcm(getListNameData(), fileName))
        {
           //remove childrens nodes
            foreach (Data* data, modifiedData)
            {
                data->clearOldValues();
                removeModifiedData(data);
            }

            return true;
        }
        else
            return false;
    }
}

// ____________ Other Methods ____________ //

QStringList Dcm::getListNameData()
{
    QStringList list;
    foreach (Data *data, listData)
    {
        list.append(data->name);
    }
    return list;
}

void Dcm::checkDisplay()
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

        int ret = QMessageBox::warning(0, "HEXplorer :: add DcmFile",
                                        "This Dcm file was deleted !\nDo you want to reload it ?",
                                        QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            WorkProject *wp = getParentWp();
            wp->addDcm(this);
            this->attach(wp);
        }
    }
}

void Dcm::addLabelsAsChild()
{
    foreach (Data *data, listData)
    {
        data->setParentNode(this);
        this->addChildNode(data);
        getParentWp()->treeModel->dataInserted(this, childNodes.indexOf(data));
    }
}

QString Dcm::fullName()
{
    return fullPath;
}

void Dcm::setFullName(QString fullName)
{
    fullPath = fullName;
    WorkProject *wp = getParentWp();
    wp->rename(this);

    // change the name displayed into owners
    foreach(QObject *obj, owners)
    {
        QString type = typeid(*obj).name();
        if (type.toLower().endsWith("formcompare"))
        {
            FormCompare *fcomp = (FormCompare*)obj;
            if (fcomp->getDcm1() == this)
            {
                 QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                               + "/"
                               + QFileInfo(fullPath).fileName();
                 fcomp->setDataset1(str);
            }
            else if (fcomp->getDcm2() == this)
            {
                QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                              + "/"
                              + QFileInfo(fullPath).fileName();
                 fcomp->setDataset2(str);
            }
        }
    }
}

void Dcm::attach(QObject *o)
{
    //check owner for validity
    if(o==0)return;

    //check for duplicates
    //if(owners.contains(o))return;

    //register
    owners.append(o);
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(detach(QObject*)));

}

void Dcm::detach(QObject *o)
{
    //remove it
    //owners.removeAll(o);
    owners.removeOne(o);

    //remove self after last one
    if(owners.size()==0)
        delete this;
}

std::string Dcm::pixmap()
{
    return ":/icones/excel.png";
}

void Dcm::checkProgressStream(int n)
{
    omp_set_lock(&lock);

    valueProgBar += n;
    emit incProgressBar(valueProgBar, maxValueProgbar);

    omp_unset_lock(&lock);

}

int Dcm::getNumByte(std::string str)
{
    return nByte.value(str.c_str());
}
