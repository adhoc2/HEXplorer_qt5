// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
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

#include "csv.h"

#include "data.h"
#include "lexer.h"
#include "lexerCsv.h"
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

bool comp(Data *a, Data *b)
{
   if (a->getName() < b->getName())
       return true;
   else return false;
}

// ___________ Csv Ctor/Dtor __________ //

Csv::Csv(QString fullCsvFileName, WorkProject *parentWP, QString modName, QObject *parent)
    : QObject(parent) , DataContainer(parentWP, modName)
{
    a2lProject = (PROJECT*)getParentWp()->a2lFile->getProject();
    fullPath = fullCsvFileName;
    name = new char[(QFileInfo(fullPath).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(fullPath).fileName()).toLocal8Bit().data());
    maxValueProgbar = 0;
    valueProgBar = 0;
    omp_init_lock(&lock);
}

Csv::~Csv()
{
    omp_destroy_lock(&lock);
    delete[] name;
}

// ____________ read / write ____________ //

bool Csv::readFile()
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
    LexerCsv mylex;
    connect(&mylex, SIGNAL(returnedToken(int)), this, SLOT(checkProgressStream(int)),
            Qt::DirectConnection);
    QString header = in.readLine();
    if (header.startsWith("CALIBRATION VALUES V"))
    {
        char ch;

        //parse "Description Header"
        in.seek(20);
        mylex.getNextToken(in);
        if (mylex.getToken() == Float)
        {
            mylex.getNextToken(in);
            if (mylex.getToken() == myUnknown && (mylex.getLexem() == "," || mylex.getLexem() == ";" || mylex.getLexem() == "\t"))
            {
                mylex.valueSeparator = mylex.getLexem().at(0);
                mylex.getNextToken(in);
                if (mylex.getToken() == myUnknown && (mylex.getLexem() == "," || mylex.getLexem() == "."))
                {
                    mylex.decimalPointSeparator = mylex.getLexem().at(0);
                    in >> ch;
                    if (ch == mylex.valueSeparator)
                    {
                        in >> mylex.commentIndicator;
                        in >> ch;
                        if (ch == mylex.valueSeparator)
                        {
                            char c1, c2;
                            in >> c1;
                            in >> c2;
                            if (c1 == c2)
                                mylex.stringDelimiter = c1;
                            else
                            {
                                showError("CSV description Header : parser error");
                                return false;
                            }
                        }
                        else
                        {
                            showError("CSV description Header : parser error");
                            return false;
                        }
                    }
                    else
                    {
                        showError("CSV description Header : parser error");
                        return false;
                    }
                }
            }
        }
        else
        {
            showError("CSV description Header : parser error");
            return false;
        }

        //parse data
        TokenTyp token;
        token = mylex.getNextToken(in);

        while (!in.atEnd())
        {
            if (token == Keyword && mylex.getLexem() == "FUNCTION_HDR")
            {
                token = mylex.getNextToken(in);
                if (token == Indentation)
                {                    
                }
                token = mylex.getNextToken(in);
                while (token == String)
                {
                    //function_hdr = mylex.getLexem().data();
                    token = mylex.getNextToken(in);
                }
            }
            else if (token == Keyword && mylex.getLexem() == "VARIANT_HDR")
            {
                token = mylex.getNextToken(in);
                while (token == String)
                {
                    token = mylex.getNextToken(in);
                }
            }
            else if (token == Keyword && mylex.getLexem() == "FUNCTION")
            {
                token = mylex.getNextToken(in);
                if (token == Indentation)
                {
                    token = mylex.getNextToken(in);
                    while (token == String || token == Comment )
                    {
                        token = mylex.getNextToken(in);
                    }
                }
                else
                    return false;
            }
            else if (token == Identifier)
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
                        QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                        listData.insert(i, data);
                    }
                    else
                    {
                        Node *label2 = nodeAxis->getNode(mylex.getLexem().c_str());
                        if (label2)
                        {
                            //create a data
                            data = new Data((AXIS_PTS*)label2, a2lProject, this, false);
                            QList<Data*>::iterator i = qLowerBound(listData.begin(), listData.end(), data, comp);
                            listData.insert(i, data);
                        }
                        else
                        {
                            outList.append("read CSV file : " + QString(mylex.getLexem().c_str()) + " not found into "
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
                    while (!ok)
                    {
                        // Z values
                        token = mylex.getNextToken(in);

                        if (token == Keyword && (mylex.getLexem() == "CURVE" ||
                                                          mylex.getLexem() == "MAP" ||
                                                          mylex.getLexem() == "VALUE"))
                        {
                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            // increment the number of rows
                            int nRows = 1;

                            // in case a label is twice in the subset!!
                            data->clearZ();

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            mylex.initialize();
                            while (token ==  Integer || token == Float || token  == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                str.replace(",",".");
                                str.remove("\"");
                                list.append(str);
                                token = mylex.getNextToken(in);
                                // check for Indentation
                                if (token == Indentation)
                                {
                                    nRows++;
                                    token = mylex.getNextToken(in);
                                }
                            }
                            int nCols = list.count() / nRows;

                            // copy z values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                data->appendZ(phys);
                            }

                            // create the x-points if it is a curve
                            AXIS_DESCR *axisDescrX = data->getAxisDescrX();
                            if (axisDescrX)
                            {
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

                            // exit loop                            
                            ok = true;                            

                        }
                        else if (token == Keyword && ( mylex.getLexem() == "VAL_BLK"))
                        {
                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            // in case a label is twice in the subset!!
                            data->clearZ();

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            mylex.initialize();
                            while (token ==  Integer || token == Float || token  == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                str.replace(",",".");
                                str.remove("\"");
                                list.append(str);
                                token = mylex.getNextToken(in);
                                // check for Indentation
                                if (token == Indentation)
                                {
                                    token = mylex.getNextToken(in);
                                }
                            }

                            // copy z values to data
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
                                data->appendZ(phys);
                            }

                            // check length of axisX (when import Csv into project, it is possible that
                            // the length specified into the A2l is longer than the ine into csv
                            if (data->xCount() > data->zCount())
                            {
                                while (data->zCount() != data->xCount())
                                {
                                    data->removeIndexX(data->xCount() - 1);
                                }

                            }

                            // define size (lines)
                            data->updateSize();

                            // exit loop
                            ok = true;

                        }
                        else if (token == Keyword && mylex.getLexem() == "AXIS_PTS")
                        {
                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            // read the Z values
                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token ==  Integer || token == Float || token  == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                str.replace(",",".");
                                str.remove("\"");
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            // add Z values to data
                            data->clearZ(); // in case a label is twice in the subset!!
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "z");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "z");

                                // copy PHYS value into Zaxis
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

                            // exit loop
                            ok = true;
                        }
                        else if (token == Keyword && mylex.getLexem() == "X_AXIS_PTS")
                        {
                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token == Integer || token == Float || token == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                str.replace(",",".");
                                str.remove("\"");
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            data->clearX();
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "x");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "x");

                                // copy PHYS value into Zaxis
                                data->appendX(phys);
                            }
                            data->updateSize();

                            // exit loop
                            data->isAxisXComparable = true;
                            ok = true;
                        }
                        else if (token == Keyword && mylex.getLexem() == "Y_AXIS_PTS")
                        {
                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            QStringList list;
                            token = mylex.getNextToken(in);
                            while (token == Integer ||token == Float ||
                                   token == String)
                            {
                                QString str = mylex.getLexem().c_str();
                                str.replace(",",".");
                                str.remove("\"");
                                list.append(str);
                                token = mylex.getNextToken(in);
                            }

                            data->clearY();
                            foreach (QString str, list)
                            {
                                // convert PHYS value from Csv into HEX value
                                QString hex = data->phys2hex(str, "y");

                                // convert HEX value into PHYS value
                                QString phys = data->hex2phys(hex, "y");

                                // copy PHYS value into Zaxis
                                data->appendY(phys);
                            }
                            data->updateSize();

                            // exit loop
                            data->isAxisYComparable = true;
                            ok = true;
                        }
                        else if (token == Keyword && mylex.getLexem() == "ASCII")
                        {

                            // check for Indentation
                            token = mylex.getNextToken(in);
                            if (token != Indentation)
                            {
                                return false;
                            }

                            // in case a label is twice in the subset!!
                            data->clearZ();

                            // read the ASCII text
                            QString text;
                            token = mylex.getNextToken(in);
                            if (token == String)
                            {
                                text = mylex.getLexem().c_str();
                            }

                            // convert text to char array
                            int asciiVal;
                            for(int i = 0; i < data->xCount(); i++)
                            {
                                if (i < text.length() - 2)
                                {
                                    asciiVal = text.at(i + 1).toAscii();
                                    data->appendZ(QString::number(asciiVal));
                                }
                                else
                                {
                                    data->appendZ("0");
                                }
                            }

                            // define size (lines)
                            data->updateSize();

                            //get next token
                            token = mylex.getNextToken(in);

                            // exit loop
                            ok = true;
                        }
                        else if (token == Comment)
                        {

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
                    while (token != Identifier && token != Eof)
                    {
                       token = mylex.getNextToken(in);
                    }
                }
            }
            else if (token == Comment)
            {
                token = mylex.getNextToken(in);
            }
            else
            {
                return false;
            }
        }

        this->getParentWp()->parentWidget->writeOutput(outList);
        return true;
    }
    else
    {
        return false;
    }
}

bool Csv::save(QString fileName)
{

    if (fileName == "")
    {
      if (exportDataList2Csv(getListNameData(), this->fullName()))
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
        if (exportDataList2Csv(getListNameData(), fileName))
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

QStringList Csv::getListNameData()
{
    QStringList list;
    foreach (Data *data, listData)
    {
        list.append(data->name);
    }
    return list;
}

void Csv::checkDisplay()
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

        int ret = QMessageBox::warning(0, "HEXplorer :: add CSVFile",
                                        "This CSV file was deleted !\nDo you want to reload it ?",
                                        QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            WorkProject *wp = getParentWp();
            wp->addCsv(this);
            this->attach(wp);
        }
    }
}

void Csv::addLabelsAsChild()
{
    foreach (Data *data, listData)
    {
        data->setParentNode(this);
        this->addChildNode(data);
        getParentWp()->treeModel->dataInserted(this, childNodes.indexOf(data));
    }
}

QString Csv::fullName()
{
    return fullPath;
}

void Csv::setFullName(QString fullName)
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
            if (fcomp->getCsv1() == this)
            {
                 QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                               + "/"
                               + QFileInfo(fullPath).fileName();
                 fcomp->setDataset1(str);
            }
            else if (fcomp->getCsv2() == this)
            {
                QString str = QFileInfo(getParentWp()->getFullA2lFileName().c_str()).fileName()
                              + "/"
                              + QFileInfo(fullPath).fileName();
                 fcomp->setDataset2(str);
            }
        }
    }
}

void Csv::attach(QObject *o)
{
    //check owner for validity
    if(o==0)return;

    //check for duplicates
    //if(owners.contains(o))return;

    //register
    owners.append(o);
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(detach(QObject*)));

}

void Csv::detach(QObject *o)
{
    //remove it
    //owners.removeAll(o);
    owners.removeOne(o);

    //remove self after last one
    if(owners.size()==0)
        delete this;
}

std::string Csv::pixmap()
{
    return ":/icones/excel.png";
}

void Csv::checkProgressStream(int n)
{
    omp_set_lock(&lock);

    valueProgBar += n;
    emit incProgressBar(valueProgBar, maxValueProgbar);

    omp_unset_lock(&lock);

}
