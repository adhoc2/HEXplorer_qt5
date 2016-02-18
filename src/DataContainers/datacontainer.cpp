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

#include "datacontainer.h"
#include "workproject.h"
#include "spreadsheetview.h"
#include "sptablemodel.h"
//#include "mat.h"
#include <QHeaderView>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>

bool dataCompare(Data *a, Data *b)
{
   if (a->getName() < b->getName())
       return true;
   else
       return false;
}

//_______________________________________________ //

DataContainer::DataContainer(WorkProject *parentWP, QString module) : Node(parentWP->a2lFile, parentWP->a2lFile->lex, parentWP->a2lFile->errorList)
{
    parentWp = parentWP;
    isSortedBySubset = false;
    moduleName = module;
}

DataContainer::DataContainer(WorkProject *parentWP) : Node(parentWP->a2lFile)
{
    parentWp = parentWP;
    isSortedBySubset = false;
    //moduleName = module;
}

DataContainer::~DataContainer()
{
    foreach (Data *data, listData)
    {
        //modifiedData already deleted from "a2l->removeChildNode(hex)" in MDImain
        //if (!modifiedData.contains(data) && data->parent() == 0)
        if (data->getParentNode() == 0)
            delete data;
    }
}

//_______________________________________________ //

WorkProject *DataContainer::getParentWp()
{
    return parentWp;
}

QString DataContainer::getModuleName()
{
    return moduleName;
}

void DataContainer::setModuleName(QString _moduleName)
{
    this->moduleName = _moduleName;
}

//_____________ Get / Set Datas __________________ //

Data *DataContainer::getData(QString str)
{
    Node node;
    node.name = new char[str.length() + 1];
    strcpy(node.name, str.toLocal8Bit().data());
    Data dat(&node);
    //QList<Data*>::iterator i = qBinaryFind(listData.begin(), listData.end(), &dat, dataCompare);
    QList<Data*>::iterator i = std::lower_bound(listData.begin(), listData.end(), &dat, dataCompare);

    if (i == listData.end())
    {
        delete[] node.name;
        return NULL;
    }
    else
    {
        if (strcmp(((Data*)*i)->name, node.name) != 0)
        {
            delete[] node.name;
            return NULL;
        }
        else
        {
            delete[] node.name;
            return *i;
        }
    }
}

QList<Data*> DataContainer::getModifiedData()
{
    return modifiedData;
}

void DataContainer::resetAllModifiedData()
{
    foreach(Data* dat, modifiedData)
    {
        dat->resetAll();
    }
}

void DataContainer::updateChildNodes(Data* data, bool add)
{
    if(!isSortedBySubset)
    {
        bool exist = this->isChild(data, true);
        if (add && !exist)
        {
            insertChildNode(data);
            data->setParentNode(this);
            parentWp->treeModel->dataInserted(this, childNodes.indexOf(data));
        }
        else if (!add && exist)
        {
            parentWp->treeModel->dataRemoved(this, childNodes.indexOf(data));
        }
    }
    else
    {
        //find subsetName of Data
        QString subsetName = data->getSubset();
        Node *sub = getNode(subsetName);

        if (add)
        {
            if (sub)
            {
                sub->insertChildNode(data);
                data->setParentNode(sub);
                parentWp->treeModel->dataInserted(sub, sub->childNodes.indexOf(data));
            }
            else
            {
                //create a new node subset
                sub = new Node();
                sub->name = new char[subsetName.toLocal8Bit().count() + 1];
                strcpy(sub->name, subsetName.toLocal8Bit().data());
                this->insertChildNode(sub);
                sub->setParentNode(this);
                parentWp->treeModel->dataInserted(this, childNodes.indexOf(sub));

                //add data into the subset
                sub->insertChildNode(data);
                data->setParentNode(sub);
                parentWp->treeModel->dataInserted(sub, sub->childNodes.indexOf(data));

            }
        }
        else
        {
            //remove Data to node subset
            parentWp->treeModel->dataRemoved(sub, sub->childNodes.indexOf(data));
            if (sub->childCount() == 0)
            {
                parentWp->treeModel->dataRemoved(this, childNodes.indexOf(sub));
            }
        }
    }
}

bool DataContainer::isSortedBySubsets()
{
    return isSortedBySubset;
}

void DataContainer::sortModifiedDataBySubset(bool subset)
{
    if (subset)
    {
        //clear treeView
        parentWp->treeModel->dataRemoved(this, 0, childNodes.count());

        isSortedBySubset = true;

        //add subsets
        foreach(Data *data, modifiedData)
        {
            updateChildNodes(data, true);
        }
    }
    else
    {
        //clear treeView
        foreach(Node *subset, childNodes)
        {
            foreach(Node *data, subset->childNodes)
                parentWp->treeModel->dataRemoved(subset, subset->childNodes.indexOf(data));

            parentWp->treeModel->dataRemoved(this, childNodes.indexOf(subset));
        }

        isSortedBySubset = false;

        //add datas
        foreach(Data *data, modifiedData)
        {
            updateChildNodes(data, true);
        }
    }
}

void DataContainer::addModifiedData(Data* data)
{
   //QList<Data*>::iterator i = qLowerBound(modifiedData.begin(), modifiedData.end(), data, dataCompare);
   QList<Data*>::iterator i = std::lower_bound(modifiedData.begin(), modifiedData.end(), data, dataCompare);
   modifiedData.insert(i, data);
   updateChildNodes(data, true);
}

void DataContainer::removeModifiedData(Data* data)
{
   modifiedData.removeOne(data);
   updateChildNodes(data, false);
}

void DataContainer::editData(Data* data)
{
    QMainWindow *win = new QMainWindow(0);
    SpreadsheetView *view = new SpreadsheetView(win);
    view->horizontalHeader()->setVisible(false);
    view->verticalHeader()->setVisible(false);
    SpTableModel *model = new SpTableModel();
    QList<Data*> *list = new QList<Data*>;
    list->append(data);
    model->setList(list);
    view->setModel(model);
    view->horizontalHeader()->setDefaultSectionSize(50);
    view->verticalHeader()->setDefaultSectionSize(18);
    view->setColumnWidth(0, 200);
    win->setCentralWidget(view);

    win->show();
    win->resize(600, 400);
}

//_____________ Export Data __________________ //

bool DataContainer::exportDataList2Csv(QStringList dataList, QString fileName)
{
    //check labels monotony
    if (!testMonotony(dataList))
    {
        return false;
    }

    // choose a file name if not already
    if (fileName == "")
    {
        //give a name to the file
        QSettings settings(qApp->organizationName(), qApp->applicationName());
        QString path = settings.value("currentCsvPath").toString();
        fileName = QFileDialog::getSaveFileName(0,"select CSV files", path,
                                                        "CSV files (*.csv);;all files (*.*)");


        if (fileName.isEmpty())
            return false;
        else if (!fileName.toLower().endsWith(".csv"))
            fileName.append(".csv");        
    }

    //set the csvFilePath into settings
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString currentCsvPath = QFileInfo(fileName).absolutePath();
    settings.setValue("currentCsvPath", currentCsvPath);

    //create a QTextStream from QFile
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(0, "Application",
                             "Cannot write file" +  fileName + "\n" +
                             file.errorString());

        return false;
    }
    QTextStream out(&file);

    //HEADER PART
    out << "CALIBRATION VALUES V2.1;.;*;\"\";\r\n\r\n";

    out << "* File generated by HEXplorer : " << qApp->applicationVersion() << "\r\n";

    A2LFILE *a2l = parentWp->a2lFile;
    out << "* ASAP file : " << QString(a2l->name) << "\r\n\r\n";
    out << "FUNCTION_HDR;;\r\n";

    //SUBSETS name
    QStringList listSubset;
    Node *fun = a2l->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");
    if (fun)
    {
        foreach (QString label, dataList)
        {
            //get the data from hexfile
            Data *data = NULL;
            data = getData(label);

            FUNCTION *subset = (FUNCTION*)fun->getNode(data->getSubset());
            if (subset)
            {
                QString subsetName = subset->name;
                if (!listSubset.contains(subsetName))
                {
                    listSubset.append(subsetName);
                }
            }
        }
        listSubset.sort();
    }
    QString strSubsets = "";
    foreach (QString str, listSubset)
    {
        strSubsets.append(";\"" + str + "\"");
    }
    out << strSubsets << "\r\n";
    out << "\r\n";

    //VALUES
    foreach (QString label, dataList)
    {
        //get the data from hexfile
        Data *data = getData(label);

        //write values to file
        if (fun)
        {
            FUNCTION *subset = (FUNCTION*)fun->getNode(data->getSubset());
            data->writeValue2Csv(subset, out);
        }
        else
            data->writeValue2Csv(0, out);
    }

    //close file
    file.close();

    return true;
}

bool DataContainer::exportDataList2Cdf(QStringList dataList, QString fileName)
{
    //check labels monotony
    if (!testMonotony(dataList))
    {
        return false;
    }

    // choose a file name if not already
    if (fileName == "")
    {
        //create CDF file(s)
        QSettings settings(qApp->organizationName(), qApp->applicationName());
        QString path = settings.value("currentCdfxPath").toString();
        fileName = QFileDialog::getSaveFileName(0,"select CDF files", path,
                            "CDF files (*.cdfx | *.cdf);;all files (*.*)");

        //ensure that the file name ends with .cdfx
        if (fileName.isEmpty())
            return false;
        else if (!fileName.toLower().endsWith(".cdfx"))
            fileName.append(".cdfx");        
    }

    //set the csvFilePath into settings
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString currentCdfxPath = QFileInfo(fileName).absolutePath();
    settings.setValue("currentCdfxPath", currentCdfxPath);

    //save the file
    A2LFILE *a2l = parentWp->a2lFile;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(0, "Application",
                             "Cannot write file" +  fileName + "\n" +
                             file.errorString());
        return false;
    }

    //create a QDomDocument
    QDomDocument doc("MSRSW PUBLIC \"-//ASAM//DTD CALIBRATION DATA FORMAT:V2.0.0:LAI:IAI:XML:CDF200.XSD//EN\" \"cdf_v2.0.0.sl.dtd\"");

    //node MSRSW
    QDomElement msrsw = doc.createElement("MSRSW");
    msrsw.setAttribute("CREATOR-VERSION", "ADM V14.1.40");
    msrsw.setAttribute("CREATOR", "HEXplorer " + qApp->applicationVersion());
    doc.appendChild(msrsw);

    // node MSRSW\SHORT-NAME
    QDomElement shortName = doc.createElement("SHORT-NAME");
    QDomText text = doc.createTextNode(name);
    shortName.appendChild(text);
    msrsw.appendChild(shortName);

    // node MSRSW\CATEGORY
    QDomElement category = doc.createElement("CATEGORY");
    text = doc.createTextNode("CDF20");
    category.appendChild(text);
    msrsw.appendChild(category);

    // node MSRSW\SW_SYSTEMS
    QDomElement swSystems = doc.createElement("SW-SYSTEMS");
    msrsw.appendChild(swSystems);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM
    QDomElement swSystem = doc.createElement("SW-SYSTEM");
    swSystems.appendChild(swSystem);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SHORT-NAME
    shortName = doc.createElement("SHORT-NAME");
    text = doc.createTextNode("information-not-available");
    shortName.appendChild(text);
    swSystem.appendChild(shortName);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC
    QDomElement swInstanceSpec = doc.createElement("SW-INSTANCE-SPEC");
    swSystem.appendChild(swInstanceSpec);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE
    QDomElement swInstanceTree = doc.createElement("SW-INSTANCE-TREE");
    swInstanceSpec.appendChild(swInstanceTree);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE\SHORT-NAME
    shortName = doc.createElement("SHORT-NAME");
    QString qstr = "projects\\" + QString(a2l->name) + "\\" + QString(this->name);
    text = doc.createTextNode(qstr);
    shortName.appendChild(text);
    swInstanceTree.appendChild(shortName);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE\CATEGORY
    category = doc.createElement("CATEGORY");
    text = doc.createTextNode("NO_VCD");
    category.appendChild(text);
    swInstanceTree.appendChild(category);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE\SW-INSTANCE-TREE-ORIGIN
    QDomElement swInstanceTreeOrigin = doc.createElement("SW-INSTANCE-TREE-ORIGIN");
    swInstanceTree.appendChild(swInstanceTreeOrigin);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE\SW-INSTANCE-TREE-ORIGIN\SYMBOLIC-FILE
    QDomElement symbolicFile = doc.createElement("SYMBOLIC-FILE");
    text = doc.createTextNode(QString(a2l->name));
    symbolicFile.appendChild(text);
    swInstanceTreeOrigin.appendChild(symbolicFile);

    // node MSRSW\SW_SYSTEMS\SW_SYSTEM\SW-INSTANCE-SPEC\SW-INSTANCE-TREE\SW-INSTANCE-TREE-ORIGIN\DATA-FILE
    QDomElement dataFile = doc.createElement("DATA-FILE");
    text = doc.createTextNode(QString(this->name));
    dataFile.appendChild(text);
    swInstanceTreeOrigin.appendChild(dataFile);

    //write values
    QStringList exportedRefAxis;
    foreach (QString label, dataList)
    {
        //get the data from hexfile
        Data *data = getData(label);

        //write to values to file
        swInstanceTree.appendChild(data->writeValue2Node(doc));

        //export reference axisX if exists
        if (data->getAxisDescrX())
        {
            if (QString(data->getAxisDescrX()->getPar("Attribute")) == "COM_AXIS")
            {
                QString refAxisX = "";
                AXIS_PTS_REF *axisPtsRef = (AXIS_PTS_REF*)data->getAxisDescrX()->getItem("AXIS_PTS_REF");
                if (axisPtsRef)
                {
                    refAxisX = axisPtsRef->getPar("AxisPoints");
                }
                if (!dataList.contains(refAxisX) && !exportedRefAxis.contains(refAxisX))
                {
                    //get the data from hexfile
                    Data *dataRefAxisX = getData(refAxisX);
                    if (dataRefAxisX)
                    {
                        swInstanceTree.appendChild(dataRefAxisX->writeValue2Node(doc));
                    }

                    //add the refAxisX as already exported to prevent from exporting twice
                    exportedRefAxis.append(refAxisX);
                }
            }
        }

        //export reference axisX if exists
        if (data->getAxisDescrY())
        {
            if (QString(data->getAxisDescrY()->getPar("Attribute")) == "COM_AXIS")
            {
                QString refAxisY = "";
                AXIS_PTS_REF *axisPtsRef = (AXIS_PTS_REF*)data->getAxisDescrY()->getItem("AXIS_PTS_REF");
                if (axisPtsRef)
                {
                    refAxisY = axisPtsRef->getPar("AxisPoints");
                }
                if (!dataList.contains(refAxisY) && !exportedRefAxis.contains(refAxisY))
                {
                    //get the data from hexfile
                    Data *dataRefAxisY = getData(refAxisY);
                    if (dataRefAxisY)
                    {
                        swInstanceTree.appendChild(dataRefAxisY->writeValue2Node(doc));
                    }

                    //add the refAxisX as already exported to prevent from exporting twice
                    exportedRefAxis.append(refAxisY);
                }
            }
        }
    }

    //write the doc to the file
    QTextStream ts( &file );
    ts << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
    ts << doc.toString();

    //close file
    file.close();

    return true;

}

bool DataContainer::exportDataList2Mat(QStringList dataList, QString fileName)
{
//    MATFile *pmat;

//    //now create a new mat-file and save some variable/matrix in it
//    double dbl1[]={1.1, 4.3, -1.6, -4, -2.75};
//    double dbl2[]={-4.9, 2.3, -5};
//    mxArray *A, *B;

//    A = mxCreateDoubleMatrix(1, 5, mxREAL);
//    B = mxCreateDoubleMatrix(1, 3, mxREAL);

//    //copy an array to matrix A and B
//    memcpy(mxGetPr(A), dbl1, 5 * sizeof(double));
//    memcpy(mxGetPr(B), dbl2, 3 * sizeof(double));


//    //opening TestVar.mat for writing new data
//    pmat=matOpen("TestVar.mat", "w");
//    matPutVariable(pmat, "A", A);
//    matPutVariable(pmat, "B", B);
//    matClose(pmat);

//    mxDestroyArray(A);
//    mxDestroyArray(B);


   return true;
}

bool DataContainer::testMonotony(QStringList listLabel)
{
    if (listLabel.isEmpty())
    {
        foreach (Data* data, modifiedData)
        {

            if (!data->checkAxisXMonotony())
            {
                int ret = QMessageBox::warning(0, "HEXplorer :: check monotony",
                                     "X axis of " + QString(data->name) + " is not strictly monotone !\n"
                                     "Do you want to proceed ?",
                                     QMessageBox::Yes  | QMessageBox::YesToAll | QMessageBox::No,
                                    QMessageBox::No);


                if (ret == QMessageBox::Yes)
                {

                }
                else if (ret == QMessageBox::YesToAll)
                {
                    return true;
                }
                else if (ret == QMessageBox::No)
                {
                    //display the wrong label
                    editData(data);

                    return false;
                }
            }
            else if (!data->checkAxisYMonotony())
            {
                int ret = QMessageBox::warning(0, "HEXplorer :: check monotony",
                                     "Y axis of " + QString(data->name) + " is not strictly monotone !\n"
                                     "Do you want to proceed ?",
                                     QMessageBox::Yes  | QMessageBox::YesToAll | QMessageBox::No,
                                    QMessageBox::No);

                if (ret == QMessageBox::Yes)
                {

                }
                else if (ret == QMessageBox::YesToAll)
                {
                    return true;
                }
                else if (ret == QMessageBox::No)
                {
                    //display the wrong label
                    editData(data);

                    return false;
                }
            }
        }
    }
    else
    {
        foreach (QString label, listLabel)
        {
            //get the data from hexfile
            Data *data = getData(label);

            if (data && !data->checkAxisXMonotony())
            {
                int ret = QMessageBox::warning(0, "HEXplorer :: check monotony",
                                     "X axis of " + QString(data->name) + " is not strictly monotone !\n"
                                     "Do you want to proceed ?",
                                     QMessageBox::Yes  | QMessageBox::YesToAll | QMessageBox::No,
                                    QMessageBox::No);


                if (ret == QMessageBox::Yes)
                {

                }
                else if (ret == QMessageBox::YesToAll)
                {
                    return true;
                }
                else if (ret == QMessageBox::No)
                {
                    //display the wrong label
                    editData(data);

                    return false;
                }
            }
            else if (data && !data->checkAxisYMonotony())
            {
                int ret = QMessageBox::warning(0, "HEXplorer :: check monotony",
                                     "Y axis of " + QString(data->name) + " is not strictly monotone !\n"
                                     "Do you want to proceed ?",
                                     QMessageBox::Yes  | QMessageBox::YesToAll | QMessageBox::No,
                                    QMessageBox::No);

                if (ret == QMessageBox::Yes)
                {

                }
                else if (ret == QMessageBox::YesToAll)
                {
                    return true;
                }
                else if (ret == QMessageBox::No)
                {
                    //display the wrong label
                    editData(data);

                    return false;
                }
            }
        }
    }

    return true;
}
