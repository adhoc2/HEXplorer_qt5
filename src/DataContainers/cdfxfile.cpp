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

#include "cdfxfile.h"

#include <QFile>
#include <QFileInfo>
#include <QDomNode>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QtXmlPatterns>
#include <QHeaderView>
#include <typeinfo>

#include "data.h"
#include "lexer.h"
#include "lexerCsv.h"
#include "formcompare.h"
#include "spreadsheetview.h"
#include "sptablemodel.h"
#include "qdebug.h"

bool compar(Data *a, Data *b)
{
   if (a->getName() < b->getName())
       return true;
   else return false;
}

bool swInstanceCompar(SwInstance *a, SwInstance *b)
{
    if (a->name < b->name)
        return true;
    else return false;
}

class MessageHandler : public QAbstractMessageHandler
{
    public:
        MessageHandler()
            : QAbstractMessageHandler(0)
        {
        }

        QString statusMessage() const
        {
            return m_description;
        }

        int line() const
        {
            return m_sourceLocation.line();
        }

        int column() const
        {
            return m_sourceLocation.column();
        }

    protected:
        virtual void handleMessage(QtMsgType type, const QString &description,
                                   const QUrl &identifier, const QSourceLocation &sourceLocation)
        {
            Q_UNUSED(type);
            Q_UNUSED(identifier);

            m_messageType = type;
            m_description = description;
            m_sourceLocation = sourceLocation;
        }

    private:
        QtMsgType m_messageType;
        QString m_description;
        QSourceLocation m_sourceLocation;
};

// --------- SwInstance clas -----------//

bool SwInstance::isConsistentWith(Data *data)
{
     // ASCII
//     if (data->getType() == "ASCII")
//     {
//         QString text = Zvalues.at(0).at(0);
//         if (text.length() <= data->xCount())
//         {
//             //transform ascii text into array of char
//             int asciiVal;
//             QStringList list;
//             for(int i = 0; i < data->xCount(); i++)
//             {
//                 if (i < text.length() - 2)
//                 {
//                     asciiVal = text.at(i + 1).toLatin1();
//                     list.append(QString::number(asciiVal));
//                 }
//                 else
//                 {
//                     list.append("0");
//                 }
//             }
//             Zvalues.clear();
//             Zvalues.append(list);
//             isValid = true;
//             return true;
//         }
//         else
//         {
//             isValid = false;
//             return false;
//         }
//     }

     data;
     isValid = true;
     return true;
}

// --------- Cdfx Constructor -----------//

CdfxFile::CdfxFile(QString fullCdfxFileFileName, WorkProject *parentWP, QString modName, QObject *parent)
        : QObject(parent) , DataContainer(parentWP, modName)
{
    a2lProject = (PROJECT*)getParentWp()->a2lFile->getProject();
    fullPath = fullCdfxFileFileName;
    name = new char[(QFileInfo(fullPath).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(fullPath).fileName()).toLocal8Bit().data());

    if (readFile())
    {
        isRead = true;
    }
    else
    {
        isRead = false;
    }

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

CdfxFile::~CdfxFile()
{
    delete[] name;
    qDeleteAll(listSwInstance);
}

// ------------ Parser -------------//

bool CdfxFile::readFile()
{
    // create a QDomDocument
    QFile file(fullPath);
    if (file.open(QIODevice::ReadOnly))
    {
        //check syntax conformity with .dtd file
        if (validate(&file))
        {
            file.seek(0);
            if (!document.setContent(&file))
            {
                errorList.append("DomDocument could not be created");
                return false;
            }
            file.close();
        }
        else
        {
            return false;
        }
    }

    //parse MSRSW
    QDomNodeList listMSRSW = document.elementsByTagName("MSRSW");
    QDomNode node = listMSRSW.at(0);
    if (node.isNull())
    {
        errorList.append("parser CDFX error :  no MSRSW node in file "
                       + QString(this->getParentWp()->a2lFile->name));
        QMessageBox::warning(0,"parser CDFX error","no MSRSW node in Cdf file");

        return false;
    }
    else
    {
        parseMSRSW(node);

        if (errorList.isEmpty())
        {
            swInstance2Data();
            return true;
        }
        else
            return false;
    }
}

bool CdfxFile::validate(QFile *file)
{
    // create a schema
    MessageHandler messageHandler;
    QXmlSchema schema;    
    schema.setMessageHandler(&messageHandler);
    QFile schemaFile(":/files/cdf_v2.0.0.cl.xsd");
    schemaFile.open(QIODevice::ReadOnly);
    schema.load(&schemaFile);

    bool errorOccurred = false;
    if (schema.isValid())
    {
        QXmlSchemaValidator validator(schema);

        if (!validator.validate(file))
        {
            errorOccurred = true;
        }
    }
    else
    {
        errorList.append("CDFx schema file not valid");
        QMessageBox::warning(0,"check Cdf file schema","CDFx schema file not valid");
        return false;
    }

    schemaFile.close();

    if (errorOccurred)
    {
        errorList.append("CDFX file is not valid  at line " + QString::number(messageHandler.line()) + " /column : " + QString::number(messageHandler.column()));
        errorList.append(messageHandler.statusMessage());
        QMessageBox::warning(0,"check Cdf file syntax","Cdf file not consistent with the schema");
        return false;
    }

    return true;
}

void CdfxFile::parseMSRSW(QDomNode &node)
{
    //parse the childNodes
    QDomNodeList childMSRSW = node.childNodes();
    for (int i = 0; i < childMSRSW.count(); i++)
    {
        QDomNode node = childMSRSW.at(i);

        //SHORT-NAME (1)
        if (node.nodeName() == "SHORT-NAME")
        {
        }

        //CATEGORY (?)
        else if (node.nodeName() == "CATEGORY")
        {
            QString category = node.firstChild().nodeValue();
            if (category != "CDF20")
            {
                errorList.append("parser CDFX error :  MSRSW category " + category + " unknown in "
                                 + QString(this->getParentWp()->a2lFile->name)
                                 + " at line " + QString::number(node.lineNumber()));
                return;
            }
        }

        //SW-SYSTEMS (?)
        else if (node.nodeName() == "SW-SYSTEMS")
        {
            QDomElement swSystems = node.toElement();

            //SW-SYSTEM (*)
            QDomNodeList listSwSystem = swSystems.elementsByTagName("SW-SYSTEM");
            for (uint i = 0; i < listSwSystem.length(); i++)
            {
                //parse the SW-SYSTEM
                QDomNode qnode= listSwSystem.at(i);
                parseSwSystem(qnode);
            }
        }

        //SDGS (?)
        else if (node.nodeName() == "SDGS")
        {
        }

        //LOCS (?)
        else if (node.nodeName() == "LOCS")
        {
        }
    }
}

void CdfxFile::parseSwSystem(QDomNode &node)
{
    //parse the childNodes
    QDomNodeList childSwSystem = node.childNodes();
    for (int i = 0; i < childSwSystem.count(); i++)
    {

        QDomNode node  = childSwSystem.at(i);

        //SHORT-NAME (1)
        if (node.nodeName() == "SHORT-NAME")
        {
        }

        //LONG-NAME (?)
        else if (node.nodeName() == "LONG-NAME")
        {
        }

        //CATEGORY(?)
        else if (node.nodeName() == "CATEGORY")
        {
        }

        //SW-INSTANCE-SPEC(?)
        else if (node.nodeName() == "SW-INSTANCE-SPEC")
        {
            //get childNodes
            QDomNodeList childSwInstanceSpec = node.childNodes();
            for (int i = 0; i < childSwInstanceSpec.count(); i++)
            {
                QDomNode node  = childSwInstanceSpec.at(i);


                //SW-INSTANCE-TREE (?)
                if (node.nodeName() == "SW-INSTANCE-TREE")
                {
                    parseSwInstanceTree(node);
                }

            }
        }
    }
}

void CdfxFile::parseSwInstanceTree(QDomNode &node)
{
    //parse the childNodes
    QDomNodeList childSwInstanceTree = node.childNodes();
    for (int i = 0; i < childSwInstanceTree.count(); i++)
    {
        QDomNode node = childSwInstanceTree.at(i);

        //SHORT-NAME (1)
        if (node.nodeName() == "SHORT-NAME")
        {
        }

        //LONG-NAME (?)
        else if (node.nodeName() == "LONG-NAME")
        {
        }

        //CATEGORY(?)
        else if (node.nodeName() == "CATEGORY")
        {
            QString category = node.firstChild().nodeValue();
            if (category != "VCD" && category != "NO_VCD")
            {
                errorList.append("parser CDFX error :  SW-INSTANCE-TREE category " + category + " unknown in "
                                 + QString(this->getParentWp()->a2lFile->name)
                                 + " at line " + QString::number(node.lineNumber()));
                return;
            }
        }

        //SW-INSTANCE-TREE-ORIGIN(?)
        else if (node.nodeName() == "SW-INSTANCE-TREE-ORIGIN")
        {
        }

        //SW-CS-COLLECTIONS(?)
        else if (node.nodeName() == "SW-CS-COLLECTIONS")
        {
        }

        //SW-INSTANCE (*)
        else if (node.nodeName() == "SW-INSTANCE")
        {
            //parse the SW-INSTANCE
            parseSwInstance(node);
        }
    }
}

void CdfxFile::parseSwInstance(QDomNode &node)
{
    // check if name exists
    QDomElement swInstance = node.toElement();
    QDomNodeList list = swInstance.elementsByTagName("SHORT-NAME");
    if (list.count() != 1)
    {
        return;
    }

    // create a new SwInstance
    SwInstance *instance = new SwInstance();

    //parse the childNodes
    QDomNodeList childSwInstance = node.childNodes();
    for (int i = 0; i < childSwInstance.count(); i++)
    {
        QDomNode node = childSwInstance.at(i);

        //SHORT-NAME (1)
        if (node.nodeName() == "SHORT-NAME")
        {
            instance->name = node.firstChild().nodeValue();
        }

        //SW-ARRAY-INDEX (1)
        else if (node.nodeName() == "SW-ARRAY-INDEX")
        {

        }

        //LONG-NAME (?)
        else if (node.nodeName() == "LONG-NAME")
        {
            instance->description = node.firstChild().nodeValue();
        }

        //DISPLAY-NAME (?)
        else if (node.nodeName() == "DISPLAY-NAME")
        {
        }

        //CATEGORY (?)
        else if (node.nodeName() == "CATEGORY")
        {
            QString category = node.firstChild().nodeValue();
            if (category == "VALUE" || category == "DEPENDENT_VALUE" ||
                category == "BOOLEAN" || category == "ASCII" ||
                category == "VAL_BLK" || category == "CURVE" ||
                category == "MAP" || category == "COM_AXIS" ||
                category == "CURVE_AXIS" || category == "RES_AXIS" ||
                category == "STRUCTURE" || category == "UNION" ||
                category == "VALUE_ARRAY" || category == "CURVE_ARRAY" ||
                category == "MAP_ARRAY" || category == "STRUCTURE_ARRAY")
            {
                instance->category = category;
            }
            else
            {
                errorList.append("parser CDFX error :  SW-INSTANCE category " + category + " unknown in "
                                 + QString(this->getParentWp()->a2lFile->name)
                                 + " at line " + QString::number(node.lineNumber()));
                return;
            }
        }

        //SW-FEATURE-REF (?)
        else if (node.nodeName() == "SW-FEATURE-REF")
        {
            instance->subset = node.firstChild().nodeValue();
        }

        // SW-VALUE-CONT (?)
        else if (node.nodeName() == "SW-VALUE-CONT")
        {
            parseSwValueCont(node, instance);
        }

        // SW-AXIS-CONTS (?)
        else if (node.nodeName() == "SW-AXIS-CONTS")
        {
            QDomNodeList childSwAxisConts = node.childNodes();
            for (int i = 0; i < childSwAxisConts.count(); i++)
            {
                QDomNode node = childSwAxisConts.at(i);

                // SW-AXIS-CONT (*)
                if (node.nodeName() == "SW-AXIS-CONT")
                {
                    parseSwAxisCont(node, instance, i);
                }
            }
        }

        //SW-CS-HISTORY (?)
        else if (node.nodeName() == "SW-CS-HISTORY")
        {
        }

        //SW-CS-FLAGS (?)
        else if (node.nodeName() == "FLAGS")
        {
        }

        //SW-INSTANCE-PROPS-VARIANTS (?)
        else if (node.nodeName() == "SW-INSTANCE-PROPS-VARIANTS")
        {
        }

        // SW-INSTANCE (*)
        else if (node.nodeName() == "SW-INSTANCE")
        {
            //parseSwInstance(node);
        }

    }

    // add the swInstance to the list
    //QList<SwInstance*>::iterator i = qLowerBound(listSwInstance.begin(), listSwInstance.end(), instance, swInstanceCompar);
    QList<SwInstance*>::iterator i = std::lower_bound(listSwInstance.begin(), listSwInstance.end(), instance, swInstanceCompar);
    listSwInstance.insert(i, instance);
}

void CdfxFile::parseSwValueCont(QDomNode &node, SwInstance *instance)
{
    QDomNodeList childSwValueCont = node.childNodes();
    for (int i = 0; i < childSwValueCont.count(); i++)
    {
        QDomNode node = childSwValueCont.at(i);

        //UNIT-DISPLAY-NAME (?)
        if (node.nodeName() == "UNIT-DISPLAY-NAME")
        {
            instance->unit = node.firstChild().nodeValue();
        }

        //SW-ARRAYSIZE (?)
        else if (node.nodeName() == "SW-ARRAYSIZE")
        {
            instance->arraySize = node.firstChild().nodeValue();
        }

        //SW-VALUES-PHYS (?)
        else if (node.nodeName() == "SW-VALUES-PHYS")
        {
            QStringList _zValues;
            QDomNodeList childSwValuePhys = node.childNodes();
            QString valueType = "";
            for (int i = 0; i < childSwValuePhys.count(); i++)
            {
                QDomNode node = childSwValuePhys.at(i);
                valueType = childSwValuePhys.at(0).nodeName();

                //VT (*)
                if (node.nodeName() == "VT" && valueType == "VT")
                {
                    //_zValues.append("\"" + node.firstChild().nodeValue() + "\"");
                    _zValues.append(node.firstChild().nodeValue());
                }

                //V (*)
                else if (node.nodeName() == "V" && valueType == "V")
                {
                    _zValues.append(node.firstChild().nodeValue());
                }

                //VG (*)
                else if (node.nodeName() == "VG" && valueType == "VG")
                {
                    QStringList rowZvalues;
                    parseVG(node, rowZvalues);
                    //instance->Zvalues.append(rowZvalues);
                    _zValues.append(rowZvalues);
                }

                else
                {
                    errorList.append("mixed <VT>, <V> and <VG> in Cdf file.");
                    QMessageBox::warning(0,"import Cdf file","check Cdf format");
                    return;
                }
            }

            // if zValues : append to instance->Zvalues
            if (_zValues.count() != 0)
                instance->Zvalues.append(_zValues);

        }
    }
}

void CdfxFile::parseSwAxisCont(QDomNode &node, SwInstance *instance, int index)
{
    //parse the childNodes
    QDomNodeList childSwAxisCont = node.childNodes();
    for (int i = 0; i < childSwAxisCont.count(); i++)
    {
        QDomNode node = childSwAxisCont.at(i);

        //CATEGORY (?)
        if (node.nodeName() == "CATEGORY")
        {
            QString category = node.firstChild().nodeValue();
            if (category == "FIX_AXIS" || category == "STD_AXIS" ||
                category == "COM_AXIS" || category == "CURVE_AXIS" ||
                category == "RES_AXIS")
            {
                if (index == 0)
                    instance->axisXtype = category;
                else if (index == 1)
                    instance->axisYtype = category;
                else if (index == 2)
                    instance->axisZtype = category;
            }
            else
            {
                errorList.append("parser CDFX error :  SW-AXIS-CONT category" + category + " unknown in "
                                 + QString(this->getParentWp()->a2lFile->name)
                                 + " at line " + QString::number(node.lineNumber()));
                return;
            }
        }

        //UNIT-DISPLAY-NAME (?)
        else if (node.nodeName() == "UNIT-DISPLAY-NAME")
        {
            QString unit = "\"" + node.firstChild().nodeValue() + "\"";
            if (index == 0)
                instance->axisXunit = unit;
            else if (index == 1)
                instance->axisYunit = unit;
            else if (index == 2)
                instance->axisZunit= unit;
        }

        //SW-INSTANCE-REF (?)
        else if (node.nodeName() == "SW-INSTANCE-REF")
        {
            QString reference = node.firstChild().nodeValue();
            if (index == 0)
                instance->axisXreference = reference;
            else if (index == 1)
                instance->axisYreference = reference;
            else if (index == 2)
                instance->axisZreference = reference;
        }

        //SW-ARRAYSIZE (?)
        else if (node.nodeName() == "SW-ARRAYSIZE")
        {
        }

        //SW-VALUES-PHYS (?)
        else if (node.nodeName() == "SW-VALUES-PHYS")
        {
            QStringList listAxisValues;
            QDomNodeList childSwValuePhys = node.childNodes();
            QString valueType = "";
            for (int i = 0; i < childSwValuePhys.count(); i++)
            {
                QDomNode node = childSwValuePhys.at(i);
                valueType = childSwValuePhys.at(0).nodeName();

                //VT (*)
                if (node.nodeName() == "VT" && valueType == "VT")
                {
                    //listAxisValues.append("\"" + node.firstChild().nodeValue() + "\"");
                    listAxisValues.append(node.firstChild().nodeValue());
                }

                //V (*)
                else if (node.nodeName() == "V" && valueType == "V")
                {
                    listAxisValues.append(node.firstChild().nodeValue());
                }

                //VG (*)
                else if (node.nodeName() == "VG" && valueType == "VG")
                {
                    parseVG(node, listAxisValues);
                }

                else
                {
                    errorList.append("mixed <VT>, <V> and <VG> in Cdf file.");
                    QMessageBox::warning(0,"import Cdf file","check Cdf format");
                    return;
                }
            }

            // copy the listValues into data.x or data.y
            if (index == 0)
            {
                instance->axisXvalues.append(listAxisValues);
            }
            else if (index == 1)
            {
                instance->axisYvalues.append(listAxisValues);
            }
            else if (index == 2)
            {
                instance->axisZvalues.append(listAxisValues);
            }
        }
    }
}

void CdfxFile::parseVG(QDomNode &node, QStringList &list)
{
    QDomNodeList childVG = node.childNodes();
    QString valueType = "";
    for (int i = 0; i < childVG.count(); i++)
    {
        QDomNode child = childVG.at(i);

        //VT (*)
        if (child.nodeName() == "VT" && (valueType == "VT" || valueType == ""))
        {
            valueType = "VT";
            //list.append("\"" + child.firstChild().nodeValue() + "\"");
            list.append(child.firstChild().nodeValue());
        }

        //V (*)
        else if (child.nodeName() == "V" && (valueType == "V" || valueType == ""))
        {
            valueType = "V";
            list.append(child.firstChild().nodeValue());
        }

        //VG (*)
        else if (child.nodeName() == "VG" && (valueType == "VG" || valueType == ""))
        {
            valueType = "VG";
            parseVG(child, list);
        }

        //LABEL (?)
        else if (child.nodeName() == "LABEL")
        {

        }

        else
        {
            errorList.append("mixed <VT>, <V> and <VG> in Cdf file.");
            QMessageBox::warning(0,"import Cdf file","check Cdf format");
            return;
        }
    }

}

void CdfxFile::swInstance2Data()
{
    //convert SwInstance into Data
    foreach (SwInstance *instance, listSwInstance)
    {
        Data *data = NULL;
        QString instanceName = instance->name;
        if (!getData(instanceName))
        {
            //get the characteristic node from a2l
            Node *nodeChar = a2lProject->getNode("MODULE/" + getModuleName() + "/CHARACTERISTIC");
            Node *nodeAxis = a2lProject->getNode("MODULE/" + getModuleName() + "/AXIS_PTS");
            Node *label = nodeChar->getNode(instanceName);
            if (label)
            {
                //create a data
                data = new Data((CHARACTERISTIC*)label, a2lProject, this, false);
                data->isSortedByRow = 1;
            }
            else
            {
                Node *label2 = nodeAxis->getNode(instanceName);
                if (label2)
                {
                    //create a data
                    data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                }
                else
                {
                    infoList.append("parser CDFX info : " + instanceName + " not found into "
                                   + QString(this->getParentWp()->a2lFile->name));
                }
            }

            // perform checks and convert to Data
            if (data && instance->isConsistentWith(data))
            {
                // status to define if can be added or not to dataList
                bool swInstanceStatus = true;

                // case of ASCII
                QString type = data->getType();
                if (type == "ASCII")
                {
                    QString text = instance->Zvalues.at(0);
                    if (text.length() <= data->xCount())
                    {
                        //transform ascii text into array of char
                        int asciiVal;
                        QStringList list;
                        for(int i = 0; i < data->xCount(); i++)
                        {
                            if (i < text.length() - 2)
                            {
                                asciiVal = text.at(i + 1).toLatin1();
                                list.append(QString::number(asciiVal));
                            }
                            else
                            {
                                list.append("0");
                            }
                        }
                        instance->Zvalues.clear();
                        instance->Zvalues.append(list);
                    }
                    else
                    {
                        swInstanceStatus = false;
                        infoList.append("label " + instanceName + " : text too long.");
                    }
                }

                // copy the listValues into data.z
                foreach (QString str, instance->Zvalues)
                {
                    // convert PHYS value from Csv into HEX value
                    QString hex = data->phys2hex(str, "z");

                    // convert HEX value into PHYS value
                    QString phys = data->hex2phys(hex, "z");

                    // copy PHYS value into Zaxis
                    data->appendZ(phys);
                }

                // fill in axis for : AXIS_PTS / CURVE / MAP
                if (type == "AXIS_PTS")
                {
                    // axisX
                    for (int i = 0; i < data->zCount(); i++)
                    {
                        data->appendX(QString::number(i));
                    }
                }
                else if (type == "CURVE")
                {
                    if (QString(data->getAxisDescrX()->getPar("Attribute")) == "COM_AXIS")
                    {                        
                        //get dataName corrisponding to the AXIS_PTS
                        AXIS_PTS_REF *axisPtsRef = (AXIS_PTS_REF*)data->getAxisDescrX()->getItem("AXIS_PTS_REF");
                        QString nameAxisX = axisPtsRef->getPar("AxisPoints");

                        //check if ref_axis is present
                        SwInstance *axisXInstance = getSwInstance(nameAxisX);
                        if (axisXInstance)
                        {
                            //check if axisX and Zvalues have the same length
                            if ( instance->Zvalues.count() == axisXInstance->Zvalues.count() )
                            {
                                for (int i = 0; i < axisXInstance->Zvalues.count(); i++)
                                {
                                    data->appendX(QString::number(i));
                                }
                            }
                            else
                            {
                                // page 40 "Calibration data format V2.0.0"
                                swInstanceStatus = false;
                                infoList.append("label " + instanceName + " : different axisX and axisZ length.");
                            }
                        }
                        else
                        {
                            // page 41 "Calibration data format V2.0.0"
                            //if the object type and the including axis type description defines referenced
                            //axis and the values are not available, this parameter is not valid

                            swInstanceStatus = false;
                            infoList.append("label " + instanceName + " : axisX reference is missing.");
                        }

                    }
                    else if (QString(data->getAxisDescrX()->getPar("Attribute")) == "FIX_AXIS")
                    {
                        //OFFSET, SHIFT and NUMBERAPO
                        FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)data->getAxisDescrX()->getItem("FIX_AXIS_PAR");
                        QString off = fixAxisPar->getPar("Offset");
                        QString sft = fixAxisPar->getPar("Shift");
                        QString napo = fixAxisPar->getPar("Numberapo");
                        bool bl;
                        int offset = off.toInt(&bl, 10);
                        int shift = sft.toInt(&bl, 10);
                        int nPtsX = napo.toUInt(&bl, 10);

                        //check if nPts < nPtsmax
                        QString maxAxisPts = data->getAxisDescrX()->getPar("MaxAxisPoints");
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
                    else if (QString(data->getAxisDescrX()->getPar("Attribute")) == "STD_AXIS")
                    {
                        //check if axisX and Zvalues have the same length
                        if ( instance->Zvalues.count() == instance->axisXvalues.count() )
                        {
                            foreach (QString str, instance->axisXvalues)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "x");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "x");

                                // copy PHYS value into Zaxis
                                data->appendX(phys);
                            }
                        }
                        else
                        {
                            // page 40 "Calibration data format V2.0.0"
                            swInstanceStatus = false;
                            infoList.append("label " + instanceName + " : different axisX and axisZ length.");
                        }
                    }
                }
                else if (type == "MAP")
                {
                    //axisX
                    if (QString(data->getAxisDescrX()->getPar("Attribute")) == "COM_AXIS")
                    {
                        //get dataName corrisponding to the AXIS_PTS
                        AXIS_PTS_REF *axisPtsRef = (AXIS_PTS_REF*)data->getAxisDescrX()->getItem("AXIS_PTS_REF");
                        QString nameAxisX = axisPtsRef->getPar("AxisPoints");
                        SwInstance *axisXInstance = getSwInstance(nameAxisX);

                        //check if ref_axis is present
                        if (axisXInstance)
                        {
                            for (int i = 0; i < axisXInstance->Zvalues.count(); i++)
                            {
                                data->appendX(QString::number(i));
                            }
                        }
                        else
                        {
                            // page 41 "Calibration data format V2.0.0"
                            //if the object type and the including axis type description defines referenced
                            //axis and the values are not available, this parameter is not valid

                            swInstanceStatus = false;
                            infoList.append("label " + instanceName + " : axisX reference is missing.");
                        }
                    }
                    else if (QString(data->getAxisDescrX()->getPar("Attribute")) == "FIX_AXIS")
                    {
                        //OFFSET, SHIFT and NUMBERAPO
                        FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)data->getAxisDescrX()->getItem("FIX_AXIS_PAR");
                        QString off = fixAxisPar->getPar("Offset");
                        QString sft = fixAxisPar->getPar("Shift");
                        QString napo = fixAxisPar->getPar("Numberapo");
                        bool bl;
                        int offset = off.toInt(&bl, 10);
                        int shift = sft.toInt(&bl, 10);
                        int nPtsX = napo.toUInt(&bl, 10);

                        //check if nPts < nPtsmax
                        QString maxAxisPts = data->getAxisDescrX()->getPar("MaxAxisPoints");
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
                    else if (QString(data->getAxisDescrX()->getPar("Attribute")) == "STD_AXIS")
                    {
                        foreach (QString str, instance->axisXvalues)
                        {
                            // convert PHYS value from Csv into HEX value
                            QString hex = data->phys2hex(str, "x");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "x");

                            // copy PHYS value into Zaxis
                            data->appendX(phys);
                        }
                    }

                    // axisY
                    if (QString(data->getAxisDescrY()->getPar("Attribute")) == "COM_AXIS")
                    {
                        //get dataName corrisponding to the AXIS_PTS
                        AXIS_PTS_REF *axisPtsRef = (AXIS_PTS_REF*)data->getAxisDescrY()->getItem("AXIS_PTS_REF");
                        QString nameAxisY = axisPtsRef->getPar("AxisPoints");
                        SwInstance *axisYInstance = getSwInstance(nameAxisY);

                        //check if ref_axis is present
                        if (axisYInstance)
                        {
                            for (int i = 0; i < axisYInstance->Zvalues.count(); i++)
                            {
                                data->appendY(QString::number(i));
                            }
                        }
                        else
                        {
                            // page 41 "Calibration data format V2.0.0"
                            //if the object type and the including axis type description defines referenced
                            //axis and the values are not available, this parameter is not valid

                            swInstanceStatus = false;
                            infoList.append("label " + instanceName + " : axisY reference is missing.");
                        }
                    }
                    else if (QString(data->getAxisDescrY()->getPar("Attribute")) == "FIX_AXIS")
                    {
                        //OFFSET, SHIFT and NUMBERAPO
                        FIX_AXIS_PAR *fixAxisPar = (FIX_AXIS_PAR*)data->getAxisDescrY()->getItem("FIX_AXIS_PAR");
                        QString off = fixAxisPar->getPar("Offset");
                        QString sft = fixAxisPar->getPar("Shift");
                        QString napo = fixAxisPar->getPar("Numberapo");
                        bool bl;
                        int offset = off.toInt(&bl, 10);
                        int shift = sft.toInt(&bl, 10);
                        int nPtsX = napo.toUInt(&bl, 10);

                        //check if nPts < nPtsmax
                        QString maxAxisPts = data->getAxisDescrY()->getPar("MaxAxisPoints");
                        double nmaxPts = maxAxisPts.toDouble();
                        if (nPtsX > nmaxPts)
                            nPtsX = nmaxPts;

                        QString str;
                        for (int i = 0; i < nPtsX; i++)
                        {
                            str.setNum((int)(offset + i * qPow(2, shift)), 16);
                            data->appendY(data->hex2phys(str, "x"));
                        }
                    }
                    else if (QString(data->getAxisDescrY()->getPar("Attribute")) == "STD_AXIS")
                    {
                        foreach (QString str, instance->axisYvalues)
                        {
                            // convert PHYS value from Csv into HEX value
                            QString hex = data->phys2hex(str, "y");

                            // convert HEX value into PHYS value
                            QString phys = data->hex2phys(hex, "y");

                            // copy PHYS value into Zaxis
                            data->appendY(phys);
                        }
                    }

                    // check dimension
                    if (data->zCount() != data->xCount() * data->yCount())
                    {
                        // page 40 "Calibration data format V2.0.0"
                        swInstanceStatus = false;
                        infoList.append("label " + instanceName + " : different axisX, axisY and axisZ length.");
                    }

                }

                //add data to the listData only if every check is passed
                if (swInstanceStatus)
                {
                    data->updateSize();
                    //QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, compar);
                    QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), data, compar);
                    listData.insert(i, data);
                }
                else
                {
                    delete data;
                }
            }
            else
            {
                infoList.append("label " + instanceName + " from Cdf file is not consistent with this project");
            }
        }
        else
        {
            // ignore this SW-INSTANCE
            // the first SW-INSTANCE of the CDF v2.0 file is used
            // see Calibration Data Format v2.0.0 at page 40
             infoList.append("label " + instanceName + " from Cdf file is ignored because declared more times.");
        }
    }
 }

int CdfxFile::getNumByte(std::string str)
{
    return nByte.value(str.c_str());
}
// -------------------------------//

std::string CdfxFile::pixmap()
{
    return ":/icones/excel.png";
}

QString CdfxFile::fullName()
{
    return fullPath;
}

QStringList CdfxFile::getListNameData()
{
    QStringList list;
    foreach (Data *data, listData)
    {
        list.append(data->name);
    }
    return list;
}

QStringList CdfxFile::getErrorList()
{
    return errorList;
}

QStringList CdfxFile::getInfoList()
{
    return infoList;
}

void CdfxFile::checkDisplay()
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

        int ret = QMessageBox::warning(0, "HEXplorer :: add CdfxFileFile",
                                        "This CdfxFile file was deleted !\nDo you want to reload it ?",
                                        QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            WorkProject *wp = this->getParentWp();
            wp->addCdfx(this);
            this->attach(wp);
        }
    }
}

void CdfxFile::attach(QObject *o)
{
    //check owner for validity
    if(o==0)return;

    //check for duplicates
    //if(owners.contains(o))return;

    //register
    owners.append(o);
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(detach(QObject*)));

}

void CdfxFile::detach(QObject *o)
{
    //remove it
    //owners.removeAll(o);
    owners.removeOne(o);

    //remove self after last one
    if(owners.size()==0)
        delete this;
}

void CdfxFile::addLabelsAsChild()
{
    foreach (Data *data, listData)
    {
        data->setParentNode(this);
        this->addChildNode(data);
        getParentWp()->treeModel->dataInserted(this, childNodes.indexOf(data));
    }
}

bool CdfxFile::save(QString fileName)
{
    if (fileName == "")
    {
      if (exportDataList2Cdf(getListNameData(), this->fullName()))
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
        if (exportDataList2Cdf(getListNameData(), fileName))
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

void CdfxFile::setFullName(QString fullName)
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
            if (fcomp->getCdf1() == this)
            {
                 QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                               + "/"
                               + QFileInfo(fullPath).fileName();
                 fcomp->setDataset1(str);
            }
            else if (fcomp->getCdf2() == this)
            {
                QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                              + "/"
                              + QFileInfo(fullPath).fileName();
                 fcomp->setDataset2(str);
            }
        }
    }
}

SwInstance *CdfxFile::getSwInstance(QString str)
{
    SwInstance instance;
    instance.name = str;
    //QList<SwInstance*>::iterator i = qBinaryFind(listSwInstance.begin(), listSwInstance.end(), &instance, swInstanceCompar);
    QList<SwInstance*>::iterator i = std::lower_bound(listSwInstance.begin(), listSwInstance.end(), &instance, swInstanceCompar);

    if (i == listSwInstance.end())
    {
        return NULL;
    }
    else
    {
        if (strcmp(((SwInstance*)*i)->name.toLocal8Bit(), instance.name.toLocal8Bit()) != 0)
        {
            return NULL;
        }
        else
        {
            return *i;
        }
    }
}
