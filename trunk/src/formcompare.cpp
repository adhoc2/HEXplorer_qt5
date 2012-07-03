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

#include "formcompare.h"
#include "ui_formcompare.h"
#include "treemodelcompleter.h"
#include "a2ltreemodel.h"
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include "node.h"
#include "hexfile.h"
#include "srecfile.h"
#include "Nodes/a2lfile.h"
#include "chooselabel.h"
#include "choosesubset.h"
#include "sptablemodel.h"
#include <typeinfo>
#include "mdimain.h"
#include "comboboxdelegate.h"
#include "omp.h"
#include "data.h"
#include "comparemodel.h"
#include "diffmodel.h"
#include "a2ltreemodel.h"
#include "noeud.h"
#include <istream>
#include <string>
#include <vector>
#include "csv.h"
#include "cdfxfile.h"
#include "dialogchooseexportformat.h"

bool dataLessThan(Data *a, Data *b )
{
   if (a->getName() < b->getName())
       return true;
   else return false;
}

FormCompare::FormCompare(A2lTreeModel *model, QTreeView *tree, QTreeView *tree_2, QWidget *parent)
    :  QWidget(parent), ui(new Ui::FormCompare)
{
    //graph
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    //ui->quicklook->setProperty("border-radius", "10px");

//    setStyleSheet(
//            "QPushButton {"
//                    "border-style: outset;"
//                    "border-width: 1px;"
//                    "border-radius: 10px;"
//                    "border-color: black;"
//                    "font: 11px;"
//                    "min-width: 8em;"
//                    "padding: 3px; }"

//            "QPushButton::pressed {"
//                    "background-color: rgb(224, 0, 0);"
//                    "border-style: inset; }"

//            "QLineEdit {"
//                    "border-style: outset;"
//                    "border-width: 1px;"
//                    "border-radius: 10px;"
//                    "border-color: black;"
//                    "font: bold 12px;"
//                    "min-width: 6em;"
//                    "padding: 3px; }"

//                    );

    //initialize pointers
    hex1 = NULL;
    hex2 = NULL;
    srec1 = NULL;
    srec2 = NULL;
    csv1 = NULL;
    csv2 = NULL;
    cdfx1 = NULL;
    cdfx2 = NULL;
    a2l1 = NULL;
    a2l2 = NULL;

    //widget Parent
    mainWidget = parent;

    //treeView(s)
    treeView = tree;
    treeView_2 = tree_2;

    //treeModel
    treeModel = model;

    //create a tablemodel for tableView
    tableModel = NULL;
    diffModel = NULL;

    //lineEdit
    ui->lineEdit->setReadOnly(true);
    ui->lineEdit_2->setReadOnly(true);

    // lineEdit SLOTS
    connect(ui->lineEdit, SIGNAL(textDropped(QString )), this, SLOT(checkDroppedFile(QString )));
    connect(ui->lineEdit_2, SIGNAL(textDropped(QString )), this, SLOT(checkDroppedFile_2(QString )));
}

FormCompare::~FormCompare()
{
    delete ui;
    delete tableModel;
    delete diffModel;
}

void FormCompare::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void FormCompare::on_quicklook_clicked()
{
    // start timer
    QTime timer;
    timer.start();

    //select the right dataset for quicklook
    if ((ui->checkBoxSrc->isChecked() && ui->checkBoxTrg->isChecked()) ||
        (!ui->checkBoxSrc->isChecked() && !ui->checkBoxTrg->isChecked()))
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please select first which dataset");
        return;
    }

    //get the hex and a2l nodes
    HexFile *hex = NULL;
    SrecFile *srec = NULL;
    Csv *csv = NULL;
    CdfxFile *cdfx = NULL;
    A2LFILE *a2l = NULL;
    QString moduleName;
    if (ui->checkBoxSrc->isChecked() && !ui->lineEdit->text().isEmpty())
    {
        hex = hex1;
        srec = srec1;
        a2l = a2l1;
        csv = csv1;
        cdfx = cdfx1;
        if (hex)
            moduleName = hex->getModuleName();
        else if (srec)
            moduleName = srec->getModuleName();
        else if (csv)
            moduleName = csv->getModuleName();
        else
            moduleName = cdfx->getModuleName();

    }
    else if (ui->checkBoxTrg->isChecked() && !ui->lineEdit_2->text().isEmpty())
    {
        hex = hex2;
        srec = srec2;
        a2l = a2l2;
        csv = csv2;
        cdfx = cdfx2;
        if (hex)
            moduleName = hex->getModuleName();
        else if (srec)
            moduleName = srec->getModuleName();
        else if (csv)
            moduleName = csv->getModuleName();
        else
            moduleName = cdfx->getModuleName();
    }
    else
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please choose first a dataset in the project list");
        return;
    }

    // check if labels to be read are selected or not
    bool all = false;
    if (charList.isEmpty())
    {
        if (hex)
        {
            MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
            charList = module->listChar;
        }
        else if (srec)
        {
            MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
            charList = module->listChar;
        }
        else if (csv)
        {
            charList = csv->getListNameData();
        }
        else if (cdfx)
        {
            charList = cdfx->getListNameData();
        }
        all = true;
    }

    // read values
    QStringList outList;
    QList<Data*> *list = new QList<Data*>;
    if (hex)
    {
        foreach (QString str, charList)
        {
            Data *data = hex->getData(str);
            if (data)
            {
                list->append(data);
            }
            else
            {
                outList.append("Action quicklook : " + str + " not an adjustable into "
                               + QString(hex->name));
            }
        }
    }
    else if (srec)
    {
        foreach (QString str, charList)
        {
            Data *data = srec->getData(str);
            if (data)
            {
                list->append(data);
            }
            else
            {
                outList.append("Action quicklook : " + str + " not an adjustable into "
                               + QString(srec->name));
            }
        }
    }
    else if (csv)
    {
        foreach (QString str, charList)
        {
            Data *data = csv->getData(str);
            if (data)
            {
                list->append(data);
            }
            else
            {
                outList.append("Action quicklook : " + str + " not an adjustable into "
                               + QString(csv->name));
            }
        }
    }
    else
    {
        foreach (QString str, charList)
        {
            Data *data = cdfx->getData(str);
            if (data)
            {
                list->append(data);
            }
            else
            {
                outList.append("Action quicklook : " + str + " not an adjustable into "
                               + QString(cdfx->name));
            }
        }
    }

    //create a table model
    if (tableModel)
        delete tableModel;
    SpTableModel *spTableModel = new SpTableModel(this);
    tableModel = spTableModel;

    // display values
    spTableModel->setList(list);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(50);
    ui->tableView->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView->setModel(spTableModel);
    ui->tableView->setColumnWidth(0, 200);

    //control if unknown type label
    QStringList unknownList;
    foreach (Data *dat, *list)
    {
        if (dat->getZ(0) == "unknown")
        {
            unknownList.append("Action quicklook : " + dat->getName() + " unknown type");
            list->removeOne(dat);
        }
    }

    // stop timer
    double t = timer.elapsed();
    QString sec ;
    sec.setNum(t/1000);

    // display information
    MDImain *win = (MDImain*)mainWidget;
    win->writeOutput(unknownList);
    win->writeOutput(outList);
    win->writeOutput("quicklook : dataset read in " + sec + " sec");

    // clear selected labels in case of no chosen
    if (all)
        charList.clear();
}

void FormCompare::on_choose_clicked()
{
    //get the hex and a2l nodes
    A2LFILE *a2l = NULL;
    HexFile *hex = NULL;
    SrecFile *srec = NULL;
    Csv *csv = NULL;
    CdfxFile *cdfx = NULL;
    if (ui->checkBoxSrc->isChecked() && !ui->lineEdit->text().isEmpty())
    {
        a2l = a2l1;
        hex = hex1;
        srec = srec1;
        csv = csv1;
        cdfx = cdfx1;
    }
    else if (ui->checkBoxTrg->isChecked() && !ui->lineEdit_2->text().isEmpty())
    {
        a2l = a2l2;
        hex = hex2;
        srec = srec2;
        csv = csv2;
        cdfx = cdfx2;
    }
    else
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please choose first a dataset in the project list");
        return;
    }

    if (a2l != NULL)
    {
        if (hex)
        {
            ChooseLabel *choose = new ChooseLabel(a2l, hex, this);
            choose->exec();
        }
        else if (srec)
        {
            ChooseLabel *choose = new ChooseLabel(a2l, srec, this);
            choose->exec();
        }
        else if (csv)
        {
            ChooseLabel *choose = new ChooseLabel(a2l, csv, this);
            choose->exec();
        }
        else if (cdfx)
        {
            ChooseLabel *choose = new ChooseLabel(a2l, cdfx, this);
            choose->exec();
        }
    }
    else
    {
        QMessageBox::information(mainWidget,"HEXplorer::chooselabes","please add first a dataset");
        return;
    }

    // launch a quicklook
    on_quicklook_clicked();

}

void FormCompare::checkDroppedFile(QString oldText)
{
    Node *rootNode = treeModel->getRootNode();
    Node *node = rootNode->getNode(ui->lineEdit->text());

    if (node == NULL)
    {
        ui->lineEdit->setText(oldText);
        QMessageBox::information(mainWidget,"HEXplorer::drop file","please drop an Hex, Csv or Cdfx file");
        return;
    }

    QString name = typeid(*node).name();
    if (!name.toLower().endsWith("hexfile") &&
        !name.toLower().endsWith("srecfile") &&
        !name.toLower().endsWith("csv") &&
        !name.toLower().endsWith("cdfxfile"))
    {
        QMessageBox::information(mainWidget,"HEXplorer::drop file","please drop an Hex, Srec, Csv or Cdfx file");
        ui->lineEdit->setText(oldText);
    }
    else
    {
        //first release previous hex, csv or cdfx if exists
        if (hex1)
        {
            hex1->getParentWp()->detach(this);
            hex1->detach(this);

        }
        else if (srec1)
        {
            srec1->getParentWp()->detach(this);
            srec1->detach(this);

        }
        else if (csv1)
        {
            csv1->getParentWp()->detach(this);
            csv1->detach(this);

        }
        else if (cdfx1)
        {
            cdfx1->getParentWp()->detach(this);
            cdfx1->detach(this);
        }

        // attach this to the new dropped hex, csv or cdfx file
        if (name.toLower().endsWith("hexfile"))
        {
            csv1 = NULL;
            cdfx1 = NULL;
            hex1 = (HexFile*)node;
            hex1->attach(this);
            hex1->getParentWp()->attach(this);
            a2l1 = (A2LFILE*)node->getParentNode();
        }
        if (name.toLower().endsWith("srecfile"))
        {
            csv1 = NULL;
            cdfx1 = NULL;
            srec1 = (SrecFile*)node;
            srec1->attach(this);
            srec1->getParentWp()->attach(this);
            a2l1 = (A2LFILE*)node->getParentNode();
        }
        else if(name.toLower().endsWith("csv"))
        {
            hex1 = NULL;
            cdfx1 = NULL;
            csv1 = (Csv*)node;
            csv1->attach(this);
            csv1->getParentWp()->attach(this);
            a2l1 = (A2LFILE*)node->getParentNode();
        }
        else if(name.toLower().endsWith("cdfxfile"))
        {
            hex1 = NULL;
            csv1 = NULL;
            cdfx1 = (CdfxFile*)node;
            cdfx1->attach(this);
            cdfx1->getParentWp()->attach(this);
            a2l1 = (A2LFILE*)node->getParentNode();
        }

        delete tableModel;
        tableModel = 0;
    }
}

void FormCompare::checkDroppedFile_2(QString str)
{
    Node *rootNode = treeModel->getRootNode();
    Node *node = rootNode->getNode(ui->lineEdit_2->text());

    if (node == NULL)
    {
        ui->lineEdit_2->setText(str);
        QMessageBox::information(mainWidget,"HEXplorer::drop File","the dropped file is not an Hex, Csv or Cdfx file");
        return;
    }

    QString name = typeid(*node).name();
    if (!name.toLower().endsWith("hexfile") &&
        !name.toLower().endsWith("srecfile") &&
        !name.toLower().endsWith("csv") &&
        !name.toLower().endsWith("cdfxfile"))
    {
        QMessageBox::information(mainWidget,"HEXplorer::drop File","please drop an Hex, Srec, Csv or Cdfx file");
         ui->lineEdit_2->setText(str);
    }
    else
    {
        //first release previous hex if exists
        if (hex2)
        {
            hex2->detach(this);
            hex2->getParentWp()->detach(this);
        }
        else if (csv2)
        {
            csv2->detach(this);
            csv2->getParentWp()->detach(this);
        }
        else if (cdfx2)
        {
            cdfx2->detach(this);
            cdfx2->getParentWp()->detach(this);
        }

        // attach this to the new dropped hex file
        if (name.toLower().endsWith("hexfile"))
        {
            csv2 = NULL;
            cdfx2 = NULL;
            hex2 = (HexFile*)node;
            hex2->attach(this);
            hex2->getParentWp()->attach(this);
            a2l2 = (A2LFILE*)hex2->getParentNode();
        }
        if (name.toLower().endsWith("srecfile"))
        {
            csv2 = NULL;
            cdfx2 = NULL;
            hex2 = NULL;
            srec2 = (SrecFile*)node;
            srec2->attach(this);
            srec2->getParentWp()->attach(this);
            a2l2 = (A2LFILE*)srec2->getParentNode();
        }
        else if(name.toLower().endsWith("csv"))
        {
            hex2 = NULL;
            cdfx2 = NULL;
            csv2 = (Csv*)node;
            csv2->attach(this);
            csv2->getParentWp()->attach(this);
            a2l2 = (A2LFILE*)node->getParentNode();
        }
        else if(name.toLower().endsWith("cdfxfile"))
        {
            hex2 = NULL;
            csv2 = NULL;
            cdfx2 = (CdfxFile*)node;
            cdfx2->attach(this);
            cdfx2->getParentWp()->attach(this);
            a2l2 = (A2LFILE*)node->getParentNode();
        }

        delete tableModel;
        tableModel = 0;
    }
}

void FormCompare::on_compare_clicked()
{
    // start timer
    double t_ref1 = 0, t_final1 = 0;
    t_ref1= omp_get_wtime();

    //define SOURCE / TARGET
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit_2->text().isEmpty())
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please choose first a dataset in the project list");
        return;
    }

    // COMPARE all if no label chosen
    bool all = false;
    QString moduleName1 = "";
    if (hex1)
    {
        moduleName1 = hex1->getModuleName();
    }
    else if (srec1)
    {
        moduleName1 = srec1->getModuleName();
    }
    else if (csv1)
    {
        moduleName1 = csv1->getModuleName();
    }
    else
    {
        moduleName1 = cdfx1->getModuleName();
    }
    QString moduleName2 = "";
    if (hex2)
    {
        moduleName2 = hex2->getModuleName();
    }
    else if (srec2)
    {
        moduleName2 = srec2->getModuleName();
    }
    else if (csv2)
    {
        moduleName2 = csv2->getModuleName();
    }
    else
    {
        moduleName2 = cdfx2->getModuleName();
    }

    if (charList.isEmpty())
    {
        if (ui->checkBoxSrc->isChecked())
        {
            if (hex1)
            {
                MODULE *module = (MODULE*)a2l1->getProject()->getNode("MODULE/" + moduleName1);
                charList = module->listChar;
            }
            else if (srec1)
            {
                MODULE *module = (MODULE*)a2l1->getProject()->getNode("MODULE/" + moduleName1);
                charList = module->listChar;
            }
            else if (csv1)
            {
                charList = csv1->getListNameData();
            }
            else
            {
                charList = cdfx1->getListNameData();
            }

        }
        else
        {
            if (hex2)
            {
                MODULE *module = (MODULE*)a2l2->getProject()->getNode("MODULE/" + moduleName2);
                charList = module->listChar;
            }
            else if (srec2)
            {
                MODULE *module = (MODULE*)a2l2->getProject()->getNode("MODULE/" + moduleName2);
                charList = module->listChar;
            }
            else if (csv2)
            {
                charList = csv2->getListNameData();
            }
            else
            {
                charList = cdfx2->getListNameData();
            }
        }
        all = true;
    }

    //define progressBar
    int stepMax = 1;
    int progMax = charList.count();
    if (progMax > 10000)
    {
        stepMax = 100;
    }
    ui->progressBar->setMaximum(progMax);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    // read HEX or Csv files : generate List<Data*>
    QList<Data*> *list1 = new QList<Data*>;
    QList<Data*> *list2 = new QList<Data*>;
    QStringList outList1;
    QStringList outList2;
    bool notFound = false;
    QStringList labelNotFoundInHex1;
    labelNotFoundInHex1.append("[Label]");
    QStringList labelNotFoundInHex2;
    labelNotFoundInHex2.append("[Label]");
    if (omp_get_num_procs() > 1 )
    {
        omp_set_num_threads(2);
        #pragma omp parallel
        {
           #pragma omp sections
            {
               #pragma omp section
                {
                    if (hex1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = hex1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action compare : " + str + " not an adjustable into "
                                               + QString(hex1->name));
                                notFound = true;
                                labelNotFoundInHex1.append(str);
                            }
                        }
                    }
                    else if (srec1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = srec1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action compare : " + str + " not an adjustable into "
                                               + QString(srec1->name));
                                notFound = true;
                                labelNotFoundInHex1.append(str);
                            }
                        }
                    }
                    else if (csv1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = csv1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action compare : " + str + " not an adjustable into "
                                               + QString(csv1->name));
                                notFound = true;
                                labelNotFoundInHex1.append(str);
                            }
                        }
                    }
                    else
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = cdfx1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action compare : " + str + " not an adjustable into "
                                               + QString(cdfx1->name));
                                notFound = true;
                                labelNotFoundInHex1.append(str);
                            }
                        }
                    }
                }
               #pragma omp section
                {
                    if (hex2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = hex2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action compare : " + str + " not an adjustable into "
                                               + QString(hex2->name));
                                notFound = true;
                                labelNotFoundInHex2.append(str);
                            }
                        }
                    }
                    else if (srec2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = srec2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action compare : " + str + " not an adjustable into "
                                               + QString(srec2->name));
                                notFound = true;
                                labelNotFoundInHex2.append(str);
                            }
                        }
                    }
                    else if (csv2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = csv2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action compare : " + str + " not an adjustable into "
                                               + QString(csv2->name));
                                notFound = true;
                                labelNotFoundInHex2.append(str);
                            }
                        }
                    }
                    else
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = cdfx2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action compare : " + str + " not an adjustable into "
                                               + QString(cdfx2->name));
                                notFound = true;
                                labelNotFoundInHex2.append(str);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (hex1)
        {
            foreach (QString str, charList)
            {
                Data *data = hex1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action compare : " + str + " not an adjustable into "
                                   + QString(hex1->name));
                    notFound = true;
                    labelNotFoundInHex1.append(str);
                }
            }
        }
        else if (srec1)
        {
            foreach (QString str, charList)
            {
                Data *data = srec1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action compare : " + str + " not an adjustable into "
                                   + QString(srec1->name));
                    notFound = true;
                    labelNotFoundInHex1.append(str);
                }
            }
        }
        else if (csv1)
        {
            foreach (QString str, charList)
            {
                Data *data = csv1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action compare : " + str + " not an adjustable into "
                                   + QString(csv1->name));
                    notFound = true;
                    labelNotFoundInHex1.append(str);
                }
            }
        }
        else
        {
            foreach (QString str, charList)
            {
                Data *data = cdfx1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action compare : " + str + " not an adjustable into "
                                   + QString(cdfx1->name));
                    notFound = true;
                    labelNotFoundInHex1.append(str);
                }
            }
        }

        if (hex2)
        {
            foreach (QString str, charList)
            {
                Data *data = hex2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action compare : " + str + " not an adjustable into "
                                   + QString(hex2->name));
                    notFound = true;
                    labelNotFoundInHex2.append(str);
                }
            }
        }
        else if (srec2)
        {
            foreach (QString str, charList)
            {
                Data *data = srec2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action compare : " + str + " not an adjustable into "
                                   + QString(srec2->name));
                    notFound = true;
                    labelNotFoundInHex2.append(str);
                }
            }
        }
        else if (csv2)
        {
            foreach (QString str, charList)
            {
                Data *data = csv2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action compare : " + str + " not an adjustable into "
                                   + QString(csv2->name));
                    notFound = true;
                    labelNotFoundInHex2.append(str);
                }
            }
        }
        else
        {
            foreach (QString str, charList)
            {
                Data *data = cdfx2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action compare : " + str + " not an adjustable into "
                                   + QString(cdfx2->name));
                    notFound = true;
                    labelNotFoundInHex2.append(str);
                }
            }
        }
    }

    //compare the read values (list1 <> list2)
    QList<Data*> *listCompareSrc = NULL;
    QList<Data*> *listCompareTrg = NULL;
    if (ui->checkBoxSrc->isChecked())
    {
        listCompareSrc = list1;
        listCompareTrg = list2;
    }
    else
    {
        listCompareSrc = list2;
        listCompareTrg = list1;
    }

    int step = 0;
    QList<Data*>::iterator i = listCompareSrc->begin();
    QList<Data*>::iterator j = listCompareTrg->begin();
    while ((i != listCompareSrc->end()) && (j != listCompareTrg->end()))
    {
        Data* di = *i;
        Data* dj = *j;
        if (di->getName() == dj->getName())
        {
            bool different = false;
            bool bl1;
            bool bl2;
            double val1 = 0;
            double val2 = 0;

            //unit
            if (di->getUnit() != dj->getUnit())
                different = true;

            //axisX
            if (!different && di->isAxisXComparable && dj->isAxisXComparable)
            {
                if (di->xCount() == dj->xCount())
                {
                    for (int i = 0; i < di->xCount(); i++)
                    {
                        val1 = di->getX(i).toDouble(&bl1);
                        val2 = dj->getX(i).toDouble(&bl2);
                        if (bl1 && bl2)
                        {
                            if (val1 != val2) {
                                different = true;
                                break;
                            }
                        }
                        else
                        {
                            if (di->getX(i) != dj->getX(i)) {
                                different = true;
                                break;
                            }
                        }
                    }
                }
                else {
                    different = true;
                }
            }

            //axisY
            if (!different && di->isAxisYComparable && dj->isAxisYComparable)
            {
                if (di->yCount() == dj->yCount())
                {
                    for (int i = 0; i < di->yCount(); i++)
                    {
                        val1 = di->getY(i).toDouble(&bl1);
                        val2 = dj->getY(i).toDouble(&bl2);
                        if (bl1 && bl2)
                        {
                            if (val1 != val2) {
                                different = true;
                                break;
                            }
                        }
                        else
                        {
                            if (di->getY(i) != dj->getY(i)) {
                                different = true;
                                break;
                            }
                        }
                    }
                }
                else {
                    different = true;
                }
            }

            //Zvalues
            if (!different && (di->zCount() == dj->zCount()))
            {
                int Nrow = di->yCount();
                int Ncol = di->xCount();

                //check map // curve
                if (Nrow == 0)
                {
                    Nrow = 1;
                }

                if (Ncol == 0)
                {
                    Ncol = 1;
                }

                for (int i = 0; i < Nrow; i++)
                {
                    for (int j = 0; j < Ncol; j++)
                    {
                        val1 = di->getZ(i, j, &bl1);
                        val2 = dj->getZ(i, j, &bl2);

                        if (bl1 && bl2)
                        {
                            if (val1 != val2)
                            {
                                different = true;
                                break;
                            }
                        }
                        else
                        {
                            if (di->getZ(i, j) != dj->getZ(i, j)) {
                                different = true;
                                break;
                            }
                        }

                    }
                }
            }
            else
                different = true;

            if (!different)
            {
                i = listCompareSrc->erase(i);
                j = listCompareTrg->erase(j);
            }
            else
            {
                i++;
                j++;
            }

        }
        else if (di->getName() < dj->getName())
        {
            i = listCompareSrc->erase(i);

        }
        else
        {
            j = listCompareTrg->erase(j);
        }

        //progressBar
        step++;
        if (step == stepMax)
        {
            step = 0;
            ui->progressBar->setValue(ui->progressBar->value() + stepMax);
            qApp->processEvents();
        }
    }

    if (i != listCompareSrc->end() && j == listCompareTrg->end())
    {
        listCompareSrc->erase(i, listCompareSrc->end());
    }
    else if (i == listCompareSrc->end() && j != listCompareTrg->end())
    {
        listCompareTrg->erase(j, listCompareTrg->end());
    }

    //set the progress bar at its maximum value
    ui->progressBar->setValue(ui->progressBar->maximum());

    //create a compareModel to store diff datas
    CompareModel *compareModel = new CompareModel(this);
    delete tableModel;
    tableModel = compareModel;

    //send values to model
    compareModel->setList(listCompareSrc, listCompareTrg);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(50);
    ui->tableView->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView->setModel(compareModel);
    ui->tableView->setColumnWidth(0, 200);

    // stop timer
    t_final1 = omp_get_wtime();
    QString num1;
    num1.setNum(t_final1 - t_ref1);

    //delete previous diffLabels
    if (diffModel)
    {
        treeView_2->setModel(0);
        delete diffModel;
        diffModel = NULL;
    }

    // display diff labels if exist
    if (listCompareSrc->count() > 0)
    {
        // get MODULES
        MODULE *module1 = (MODULE*)a2l1->getProject()->getNode("MODULE/" + moduleName1);
        MODULE *module2 = (MODULE*)a2l2->getProject()->getNode("MODULE/" + moduleName2);

        // create a new treeModel
        diffModel = new A2lTreeModel();
        diffModel->createRootNode();
        if (module1 && module2)
        {
            QStringList listDataName;
            //if (module1 == module2)
            if (module2)
            {
                Node *rootNode1 = diffModel->getRootNode();
                foreach(Data *data, *list1)
                {
                    listDataName.append(data->name);

                    //find subsetName of Data
                    QString subsetName = data->getSubset();

                    // check if subset is already inserted
                    Node *sub = rootNode1->getNode(subsetName);
                    if (sub)
                    {
                        Node *node = new Node(data->name);
                        sub->addChildNode(node);
                        node->setParentNode(sub);
                    }
                    else
                    {
                        // create a subset Node
                        Node *subset = new Node();
                        subset->name = new char[subsetName.toLocal8Bit().count() + 1];
                        strcpy(subset->name, subsetName.toLocal8Bit().data());

                        // add the char to the subset
                        Node *node = new Node(data->name);
                        subset->addChildNode(node);
                        node->setParentNode(subset);

                        diffModel->addNode2RootNode(subset);
                    }
                }
            }
            else
            {

                Node *rootNode1 = 0;
                if (hex1)
                {
                   rootNode1 = new Node(hex1->name);
                }
                else if (srec1)
                {
                   rootNode1 = new Node(srec1->name);
                }
                else if (csv1)
                {
                   rootNode1 = new Node(csv1->name);
                }
                else
                {
                    rootNode1 = new Node(cdfx1->name);
                }
                foreach(Data *data, *list1)
                {
                    listDataName.append(data->name);

                    //find subsetName of Data
                    QString subsetName = data->getSubset();

                    // check if subset is already inserted
                    Node *sub = rootNode1->getNode(subsetName);
                    if (sub)
                    {
                        Node *node = new Node(data->name);
                        sub->addChildNode(node);
                        node->setParentNode(sub);
                    }
                    else
                    {
                        // create a subset Node
                        Node *subset = new Node();
                        subset->name = new char[subsetName.toLocal8Bit().count() + 1];
                        strcpy(subset->name, subsetName.toLocal8Bit().data());
                        rootNode1->addChildNode(subset);
                        subset->setParentNode(rootNode1);
                        rootNode1->sortChildrensName();

                        // add the char to the subset
                        Node *node = new Node(data->name);
                        subset->addChildNode(node);
                        node->setParentNode(subset);

                    }
                }
                diffModel->addNode2RootNode(rootNode1);

                Node *rootNode2 = 0;
                if (hex2)
                {
                   rootNode2 = new Node(hex2->name);
                }
                else if (srec2)
                {
                   rootNode2 = new Node(srec2->name);
                }
                else if (csv2)
                {
                   rootNode2 = new Node(csv2->name);
                }
                else
                {
                    rootNode2 = new Node(cdfx2->name);
                }
                foreach(Data *data, *list2)
                {
                    //find subsetName of Data
                    QString subsetName = data->getSubset();

                    // check if subset is already inserted
                    Node *sub = rootNode2->getNode(subsetName);
                    if (sub)
                    {
                        Node *node = new Node(data->name);
                        sub->addChildNode(node);
                        node->setParentNode(sub);
                    }
                    else
                    {
                        // create a subset Node
                        Node *subset = new Node();
                        subset->name = new char[subsetName.toLocal8Bit().count() + 1];
                        strcpy(subset->name, subsetName.toLocal8Bit().data());
                        rootNode2->addChildNode(subset);
                        subset->setParentNode(rootNode2);
                        rootNode2->sortChildrensName();

                        // add the char to the subset
                        Node *node = new Node(data->name);
                        subset->addChildNode(node);
                        node->setParentNode(subset);

                    }
                }
                diffModel->addNode2RootNode(rootNode2);
            }

            diffModel->setListDataName(listDataName);
        }

        treeView_2->setModel(diffModel);
        treeView_2->setColumnHidden(1, true);
    }
    else
    {
        QMessageBox::information(this, "HEXplorer :: compare", "no different labels");
    }

    // delete the labelList in case no labels were chosen !!
    if (all)
    {
        charList.clear();
    }

    //output information
    MDImain *win = (MDImain*)mainWidget;
    win->writeOutput(outList1);
    win->writeOutput(outList2);
    win->writeOutput("Action compare finished in " + num1 + " sec (found "
                     + QString::number(listCompareSrc->count()) + " different labels)");

    ui->progressBar->setVisible(false);

    //label not found information
    if (notFound)
    {
        int ret = QMessageBox::question(this, "HEXplorer :: compare", "labels not found during compare.\n"
                                  "would you like to export the list?",
                                        QMessageBox::Yes, QMessageBox::No);


        if (ret == QMessageBox::Yes)
        {
            QSettings settings(qApp->organizationName(), qApp->applicationName());
            QString currentLabPath = settings.value("currentLabPath").toString();
            QString fileName = QFileDialog::getSaveFileName(this,"save lab files", currentLabPath,
                                                            "lab files (*.txt);;all files (*.*)");
            if (fileName.isEmpty())
                return;

            if (!fileName.endsWith(".txt"))
            {
                fileName.append(".txt");
            }
            currentLabPath = QFileInfo(fileName).absolutePath();
            settings.setValue("currentLabPath", currentLabPath);


            QFile file(fileName);
            if (!file.open(QFile::WriteOnly)) {
                QMessageBox::warning(this, tr("Application"),
                                     tr("Cannot write file %1:\n%2.")
                                     .arg(fileName)
                                     .arg(file.errorString()));
                return;
            }

            QTextStream out(&file);
            QApplication::setOverrideCursor(Qt::WaitCursor);

            out << "labels not found into : " << ui->lineEdit->text() << "\r\n";
            foreach (QString str, labelNotFoundInHex1)
                out << str << "\r\n";

            out << "labels not found into : " << ui->lineEdit_2->text() << "\r\n";
            foreach (QString str, labelNotFoundInHex2)
                out << str << "\r\n";


            QApplication::restoreOverrideCursor();
        }
        else if (ret == QMessageBox::No)
        {

        }


    }
}

void FormCompare::on_checkBoxSrc_clicked()
{
    if (ui->checkBoxSrc->isChecked())
    {
        ui->checkBoxTrg->setChecked(false);
        ui->checkBoxSrc->setText("Source");
        ui->checkBoxTrg->setText("Target");
    }
    else
    {
        ui->checkBoxTrg->setChecked(true);
        ui->checkBoxSrc->setText("Target");
        ui->checkBoxTrg->setText("Source");
    }
}

void FormCompare::on_checkBoxTrg_clicked()
{

    if (ui->checkBoxTrg->isChecked())
    {
        ui->checkBoxSrc->setChecked(false);
        ui->checkBoxTrg->setText("Source");
        ui->checkBoxSrc->setText("Target");
    }
    else
    {
        ui->checkBoxSrc->setChecked(true);
        ui->checkBoxTrg->setText("Target");
        ui->checkBoxSrc->setText("Source");
    }
}

void FormCompare::on_copy_clicked()
{
    //MessageBox to confirm copy action
    if (ui->checkBoxSrc->isChecked())
    {
        QString source;
        QString target;

        if (hex1)
        {
            source = QString(hex1->name);
        }
        else if (srec1)
        {
            source = QString(srec1->name);
        }
        else if (csv1)
        {
            source = QString(csv1->name);
        }
        else if (cdfx1)
            source = QString(cdfx1->name);


        if (hex2)
        {
            target = QString(hex2->name);
        }
        else if (srec2)
        {
            target = QString(srec2->name);
        }
        else if (csv2)
        {
            target = QString(csv2->name);
        }
        else if (cdfx2)
            target = QString(cdfx2->name);


        int ret = QMessageBox::question(this, "HEXplorer :: copy action",
                                        "Do you really want to copy :\n"
                                        "from : " + source + "\n"
                                        " to  : " + target + "'?",
                                        (QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);

        if (ret == QMessageBox::No)
        {
            return;
        }
    }
    else
    {

        QString source;
        QString target;

        if (hex1)
        {
            target = QString(hex1->name);
        }
        else if (srec1)
            target = QString(srec1->name);
        else if (cdfx1)
            target = QString(cdfx1->name);
        else
            target = QString(csv1->name);

        if (hex2)
        {
            source = QString(hex2->name);
        }
        else if (srec2)
            source = QString(srec2->name);
        else if (cdfx2)
            source = QString(cdfx2->name);
        else
            source = QString(csv2->name);

        int ret = QMessageBox::question(this, "HEXplorer :: copy action",
                                        "Do you really want to copy :\n"
                                        "from : " + source + "\n"
                                        " to  : " + target + " ?",
                                        (QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);

        if (ret == QMessageBox::No)
        {
            return;
        }
    }

    // start timer
    double t_ref1 = 0, t_final1 = 0;
    t_ref1= omp_get_wtime();

    //define SOURCE / TARGET
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit_2->text().isEmpty())
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","source and target Hex files must be defined.");
        return;
    }

    // COMPARE all if no label chosen
    bool all = false;
    QString moduleName1 = "";
    QString moduleName2 = "";
    if (charList.isEmpty())
    {
        if (ui->checkBoxSrc->isChecked())
        {
            if (hex1)
            {
                moduleName1 = hex1->getModuleName();
                MODULE *module = (MODULE*)a2l1->getProject()->getNode("MODULE/" + moduleName1);
                charList = module->listChar;
            }
            else if (srec1)
            {
                moduleName1 = srec1->getModuleName();
                MODULE *module = (MODULE*)a2l1->getProject()->getNode("MODULE/" + moduleName1);
                charList = module->listChar;
            }
            else if (csv1)
            {
                moduleName1 = csv1->getModuleName();
                charList = csv1->getListNameData();
            }
            else
            {
                moduleName1 = cdfx1->getModuleName();
                charList = cdfx1->getListNameData();
            }
        }
        else
        {
            if (hex2)
            {
                moduleName2 = hex2->getModuleName();
                MODULE *module = (MODULE*)a2l2->getProject()->getNode("MODULE/" + moduleName2);
                charList = module->listChar;
            }
            else if (srec2)
            {
                moduleName2 = srec2->getModuleName();
                MODULE *module = (MODULE*)a2l2->getProject()->getNode("MODULE/" + moduleName2);
                charList = module->listChar;
            }
            else if (csv2)
            {
                moduleName2 = csv2->getModuleName();
                charList = csv2->getListNameData();
            }
            else
            {
                moduleName2 = cdfx2->getModuleName();
                charList = cdfx2->getListNameData();
            }
        }
        all = true;
    }

    int stepMax = 1;
    int progMax = charList.count();
    if (progMax > 10000)
    {
        stepMax = 100;
    }
    ui->progressBar->setMaximum(progMax);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    // read HEX or Csv files
    QList<Data*> *list1 = new QList<Data*>;
    QList<Data*> *list2 = new QList<Data*>;
    QStringList outList1;
    QStringList outList2;
    if (omp_get_num_procs() > 1 )
    {
        omp_set_num_threads(2);
        #pragma omp parallel
        {
           #pragma omp sections
            {
               #pragma omp section
                {
                    if (hex1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = hex1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(hex1->name));
                            }
                        }
                    }
                    else if (srec1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = srec1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(srec1->name));
                            }
                        }
                    }
                    else if (csv1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = csv1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(csv1->name));
                            }
                        }
                    }
                    else if (cdfx1)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = cdfx1->getData(str);
                            if (data)
                            {
                                list1->append(data);
                            }
                            else
                            {
                                outList1.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(cdfx1->name));
                            }
                        }
                    }
                 }
               #pragma omp section
                {
                    if (hex2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = hex2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(hex2->name));
                            }
                        }
                    }
                    else if (srec2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = srec2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(srec2->name));
                            }
                        }
                    }
                    else if (csv2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = csv2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(csv2->name));
                            }
                        }
                    }
                    else if (cdfx2)
                    {
                        foreach (QString str, charList)
                        {
                            Data *data = cdfx2->getData(str);
                            if (data)
                            {
                                list2->append(data);
                            }
                            else
                            {
                                outList2.append("Action quicklook : " + str + " not an adjustable into "
                                               + QString(cdfx2->name));
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (hex1)
        {
            foreach (QString str, charList)
            {
                Data *data = hex1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(hex1->name));
                }
            }
        }
        else if (csv1)
        {
            foreach (QString str, charList)
            {
                Data *data = csv1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(csv1->name));
                }
            }
        }
        else if (cdfx1)
        {
            foreach (QString str, charList)
            {
                Data *data = cdfx1->getData(str);
                if (data)
                {
                    list1->append(data);
                }
                else
                {
                    outList1.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(cdfx1->name));
                }
            }
        }

        if (hex2)
        {
            foreach (QString str, charList)
            {
                Data *data = hex2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(hex2->name));
                }
            }
        }
        else if (csv2)
        {
            foreach (QString str, charList)
            {
                Data *data = csv2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(csv2->name));
                }
            }
        }
        else if (cdfx2)
        {
            foreach (QString str, charList)
            {
                Data *data = cdfx2->getData(str);
                if (data)
                {
                    list2->append(data);
                }
                else
                {
                    outList2.append("Action quicklook : " + str + " not an adjustable into "
                                   + QString(cdfx2->name));
                }
            }
        }
    }

    //compare the read values (list1 <> list2) and copy if different
    QList<Data*> *listSrc = NULL;
    QList<Data*> *listTrg = NULL;
    if (ui->checkBoxSrc->isChecked())
    {
        listSrc = list1;
        listTrg = list2;
    }
    else
    {
        listSrc = list2;
        listTrg = list1;
    }

    //get the different data
    int step = 0;
    foreach (Data* data1, *listSrc)
    {
         QList<Data*>::iterator i =  qBinaryFind(listTrg->begin(), listTrg->end(), data1, dataLessThan);
         if (i == listTrg->end())
         {
             listSrc->removeOne(data1);
         }
         else
         {
             bool different = false;
             Data *data2 = *i;
             bool bl1;
             bool bl2;
             double val1 = 0;
             double val2 = 0;

             //axisX
             if (data1->isAxisXComparable && data2->isAxisXComparable)
             {
                 if (data1->xCount() == data2->xCount())
                 {
                     for (int i = 0; i < data1->xCount(); i++)
                     {
                         val1 = data1->getX(i).toDouble(&bl1);
                         val2 = data2->getX(i).toDouble(&bl2);
                         if (bl1 && bl2)
                         {
                             if (val1 != val2)
                             {
                                //data2->setX(i, data1->getX(i));
                                 different = true;
                             }
                         }
                         else
                         {
                             if (data1->getX(i) != data2->getX(i))
                             {
                                //data2->setX(i, data1->getX(i));
                                 different = true;
                             }
                         }
                     }
                 }
             }

             //axisY
             if (data1->isAxisYComparable && data2->isAxisYComparable)
             {
                 if (data1->yCount() == data2->yCount())
                 {
                    for (int i = 0; i < data1->yCount(); i++)
                    {
                        val1 = data1->getY(i).toDouble(&bl1);
                        val2 = data2->getY(i).toDouble(&bl2);
                        if (bl1 && bl2)
                        {
                            if (val1 != val2)
                            {
                               //data2->setY(i, data1->getY(i));
                                different = true;
                            }
                        }
                        else
                        {
                            if (data1->getY(i) != data2->getY(i))
                            {
                               //data2->setY(i, data1->getY(i));
                                different = true;
                            }
                        }
                    }
                 }
             }

             //Zvalues
             if (data1->zCount() == data2->zCount())
             {
                 int Nrow = data1->yCount();
                 int Ncol = data1->xCount();

                 //check map // curve
                 if (Nrow == 0)
                 {
                     Nrow = 1;
                 }

                 if (Ncol == 0)
                 {
                     Ncol = 1;
                 }

                for (int i = 0; i < Nrow; i++)
                 {
                    for (int j = 0; j < Ncol; j++)
                     {
                        val1 = data1->getZ(i, j, &bl1);
                        val2 = data2->getZ(i, j, &bl2);

                        if (bl1 && bl2)
                        {
                            if (val1 != val2)
                            {
                                different = true;
                            }
                        }
                        else
                        {
                            if (data1->getZ(i, j) != data2->getZ(i, j))
                                different = true;
                        }
                    }
                }
             }

             if (!different)
             {
                 listSrc->removeOne(data1);
                 listTrg->removeOne(data2);
             }

             step++;
             if (step == stepMax)
             {
                step = 0;
                ui->progressBar->setValue(ui->progressBar->value() + stepMax);
             }
         }
    }

    //copy the different data from listSrc to listTrg
    int copiedLabels = 0;
    ui->progressBar->setMaximum(progMax + listSrc->count() );
    foreach (Data* data1, *listSrc)
    {
        QList<Data*>::iterator i =  qBinaryFind(listTrg->begin(), listTrg->end(), data1, dataLessThan);
        if (i == listTrg->end())
        {
            listSrc->removeOne(data1);
        }
        else
        {
            Data *data2 = *i;
            data2->copyAllFrom(data1);
            copiedLabels++;

            ui->progressBar->setValue(ui->progressBar->value() + 1);
        }
    }

    //set the progress bar at its maximum value
    ui->progressBar->setValue(ui->progressBar->maximum());

    // stop timer
    t_final1 = omp_get_wtime();
    QString num1;
    num1.setNum(t_final1 - t_ref1);

    //output information
    MDImain *win = (MDImain*)mainWidget;
    win->writeOutput(outList1);
    win->writeOutput(outList2);
    win->writeOutput(QString::number(copiedLabels) + " copied labels");
    win->writeOutput("Action copy finished in " + num1 + " sec");

    // delete the labelList in case no labels were chosen !!
    if (all)
        charList.clear();

    //delete previous diffLabels
    if (diffModel)
    {
        treeView_2->setModel(0);
        delete diffModel;
        diffModel = NULL;
    }

    //reset tableView
    emit ui->tableView->reset();

    //close the prrogress bar
    ui->progressBar->setVisible(false);
}

void FormCompare::on_export_labels_clicked()
{
    //select the right dataset for quicklook
    if ((ui->checkBoxSrc->isChecked() && ui->checkBoxTrg->isChecked()) ||
        (!ui->checkBoxSrc->isChecked() && !ui->checkBoxTrg->isChecked()))
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please select first which dataset");
        return;
    }

    // check if labels to be read are selected or not
    if (charList.isEmpty())
    {
        QMessageBox::information(mainWidget,"HEXplorer::export csv","please choose labels to export");
        return;
    }

    //choose format
    QString exportFormat = "";
    DialogChooseExportFormat *chooseFormat = new DialogChooseExportFormat(&exportFormat, this);
    chooseFormat->exec();

    //get the hex and a2l nodes
    bool exportStatus = true;
    if (ui->checkBoxSrc->isChecked() && !ui->lineEdit->text().isEmpty())
    {
        if (hex1)
        {
            if (exportFormat == "csv")
                exportStatus = hex1->exportDataList2Csv(charList);
            else
                exportStatus = hex1->exportDataList2Cdf(charList);
        }
        else if (srec1)
        {
            if (exportFormat == "csv")
                exportStatus = srec1->exportDataList2Csv(charList);
            else
                exportStatus = srec1->exportDataList2Cdf(charList);
        }
        else if (csv1)
        {
            if (exportFormat == "csv")
                exportStatus = csv1->exportDataList2Csv(charList);
            else
                exportStatus = csv1->exportDataList2Cdf(charList);
        }
        else if (cdfx1)
        {
            if (exportFormat == "csv")
                exportStatus = cdfx1->exportDataList2Csv(charList);
            else
                exportStatus = cdfx1->exportDataList2Cdf(charList);
        }
    }
    else if (ui->checkBoxTrg->isChecked() && !ui->lineEdit_2->text().isEmpty())
    {
        if (hex2)
        {
            if (exportFormat == "csv")
                exportStatus = hex2->exportDataList2Csv(charList);
            else
                exportStatus = hex2->exportDataList2Cdf(charList);
        }
        else if (srec2)
        {
            if (exportFormat == "csv")
                exportStatus = srec2->exportDataList2Csv(charList);
            else
                exportStatus = srec2->exportDataList2Cdf(charList);
        }
        else if (csv2)
        {
            if (exportFormat == "csv")
                exportStatus = csv2->exportDataList2Csv(charList);
            else
                exportStatus = csv2->exportDataList2Cdf(charList);
        }
        else if (cdfx2)
        {
            if (exportFormat == "csv")
                exportStatus = cdfx2->exportDataList2Csv(charList);
            else
                exportStatus = cdfx2->exportDataList2Cdf(charList);
        }
    }
    else
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please choose first a dataset in the project list");
        return;
    }

    //output information
    MDImain *win = (MDImain*)mainWidget;
    if (exportStatus)
        win->writeOutput("Action export labels performed with success");
    else
        win->writeOutput("Action export labels not performed");
}

void FormCompare::on_export_subset_clicked()
{
    //get the hex and a2l nodes
    HexFile *hex = NULL;
    SrecFile *srec = NULL;
    A2LFILE *a2l = NULL;
    QString moduleName;
    if (ui->checkBoxSrc->isChecked() && !ui->lineEdit->text().isEmpty())
    {
        if (hex1)
        {
            hex = hex1;
            moduleName = hex1->getModuleName();
        }
        else if (srec1)
        {
            srec = srec1;
            moduleName = srec1->getModuleName();
        }
        else
        {
            QMessageBox::information(this, "HEXplorer :: export subset", "please first select an HEX or a Srec file.");
            return;
        }
        a2l = a2l1;

    }
    else if (ui->checkBoxTrg->isChecked() && !ui->lineEdit_2->text().isEmpty())
    {
        if (hex2)
        {
            hex = hex2;
            moduleName = hex2->getModuleName();
        }
        else if (srec2)
        {
            srec = srec2;
            moduleName = srec2->getModuleName();
        }
        else
        {
             QMessageBox::information(this, "HEXplorer :: export subset", "please first select an HEX file.");
            return;
        }
        a2l = a2l2;

    }
    else
    {
        QMessageBox::information(mainWidget,"HEXplorer::quicklook","please choose first a dataset in the project list");
        return;
    }

    //choose subset(s)
    QStringList subsetList;
    if (hex)
    {
        ChooseSubset *chooseSubset = new ChooseSubset(a2l, hex, subsetList, this);
        chooseSubset->exec();
    }
    else if (srec)
    {
        ChooseSubset *chooseSubset = new ChooseSubset(a2l, srec, subsetList, this);
        chooseSubset->exec();
    }

    if (subsetList.isEmpty())
        return;

    //choose format
    QString exportFormat = "";
    DialogChooseExportFormat *chooseFormat = new DialogChooseExportFormat(&exportFormat, this);
    chooseFormat->exec();

    //export
    if (exportFormat == "csv")
    {
        if (hex)
            hex->exportSubsetList2Csv(subsetList);
        else if (srec)
            srec->exportSubsetList2Csv(subsetList);

    }
    else if (exportFormat == "cdf")
    {
        if (hex)
            hex->exportSubsetList2Cdf(subsetList);
        else if (srec)
            srec->exportSubsetList2Cdf(subsetList);
    }

    //output information
    MDImain *win = (MDImain*)mainWidget;
    win->writeOutput("Action export subsets performed with success");

}

void FormCompare::resetModel()
{
    if (ui->tableView->selectionModel())
    {
        QModelIndexList listIndex = ui->tableView->selectionModel()->selectedIndexes();
        emit ui->tableView->reset();
        if (!listIndex.isEmpty())
        {
            ui->tableView->selectionModel()->select(listIndex.at(0), QItemSelectionModel::Select);
            ui->tableView->setCurrentIndex(listIndex.at(0));
        }
    }
}

void FormCompare::setDataset1(QString str)
{
    QString oldText = ui->lineEdit->text();
    ui->lineEdit->setText(str);
    checkDroppedFile(oldText);
}

void FormCompare::setDataset2(QString str)
{
    QString oldText = ui->lineEdit_2->text();
    ui->lineEdit_2->setText(str);
    checkDroppedFile_2(oldText);
}

void FormCompare::on_lineEdit_textChanged(QString )
{
    if (diffModel)
    {
        treeView_2->setModel(0);
        delete diffModel;
        diffModel = NULL;
    }
}

void FormCompare::on_lineEdit_2_textChanged(QString )
{
    if (diffModel)
    {
        treeView_2->setModel(0);
        delete diffModel;
        diffModel = NULL;
    }
}

A2lTreeModel *FormCompare::getDiffModel()
{
    return diffModel;
}

HexFile *FormCompare::getHex1()
{
    return hex1;
}

HexFile *FormCompare::getHex2()
{
    return hex2;
}

SrecFile *FormCompare::getSrec1()
{
    return srec1;
}

SrecFile *FormCompare::getSrec2()
{
    return srec2;
}

Csv *FormCompare::getCsv1()
{
    return csv1;
}

Csv *FormCompare::getCsv2()
{
    return csv2;
}

CdfxFile *FormCompare::getCdf1()
{
    return cdfx1;
}

CdfxFile *FormCompare::getCdf2()
{
    return cdfx2;
}
