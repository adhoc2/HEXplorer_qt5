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

#include "chooselabel.h"
#include "ui_chooselabel.h"
#include "Nodes/a2lfile.h"
#include "hexfile.h"
#include "csv.h"
#include "dcmfile.h"
#include "Nodes/def_characteristic.h"
#include "formcompare.h"
#include "mdimain.h"
#include "cdfxfile.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QAction>
#include <qdebug.h>

ChooseLabel::ChooseLabel(A2LFILE *_a2l, HexFile *_hex, QWidget *parent) :
    QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels into " + QString(_a2l->name));
    QIcon icon(":/icones/milky_plus.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = _hex;
    srec = NULL;
    csv = NULL;
    cdfx = NULL;
    dcm = NULL;

    // copy the charList from formComapare into choosenList and listView_2
    FormCompare *fc = (FormCompare*)mainWidget;
    ui->listWidget_2->addItems(fc->charList);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a label name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    //clear the long_secription label
    ui->label_3->clear();

    // connect SLOTS
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(searchItem()));
    connect(ui->lineEdit_2, SIGNAL(textChanged(QString)), this, SLOT(searchItem()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectedItem(QListWidgetItem*,QListWidgetItem*)));

    // create actions for shortCuts
    createActions();
    ui->listWidget->addAction(rightSelect);
    ui->listWidget_2->addAction(leftSelect);
}

ChooseLabel::ChooseLabel(A2LFILE *_a2l, SrecFile *_srec, QWidget *parent) :
    QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels into " + QString(_a2l->name));
    QIcon icon(":/icones/milky_plus.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = NULL;
    srec = _srec;
    csv = NULL;
    cdfx = NULL;
    dcm = NULL;

    // copy the charList from formComapare into choosenList and listView_2
    FormCompare *fc = (FormCompare*)mainWidget;
    ui->listWidget_2->addItems(fc->charList);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a label name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    //clear the long_secription label
    ui->label_3->clear();

    // connect SLOTS
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(searchItem()));
    connect(ui->lineEdit_2, SIGNAL(textChanged(QString)), this, SLOT(searchItem()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectedItem(QListWidgetItem*,QListWidgetItem*)));

    // create actions for shortCuts
    createActions();
    ui->listWidget->addAction(rightSelect);
    ui->listWidget_2->addAction(leftSelect);
}

ChooseLabel::ChooseLabel(A2LFILE *_a2l, Csv *_csv, QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels into " + QString(_a2l->name));
    QIcon icon(":/icones/Add record.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = NULL;
    srec = NULL;
    cdfx = NULL;
    dcm = NULL;
    csv = _csv;

    // copy the charList from formComapare into choosenList and listView_2
    FormCompare *fc = (FormCompare*)mainWidget;
    ui->listWidget_2->addItems(fc->charList);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a label name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    //clear the long_secription label
    ui->label_3->clear();

    // connect SLOTS
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->lineEdit_2, SIGNAL(editingFinished()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectedItem(QListWidgetItem*,QListWidgetItem*)));

    // create actions for shortCuts
    createActions();
    ui->listWidget->addAction(rightSelect);
    ui->listWidget_2->addAction(leftSelect);
}

ChooseLabel::ChooseLabel(A2LFILE *_a2l, CdfxFile *_cdfx, QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels into " + QString(_a2l->name));
    QIcon icon(":/icones/Add record.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = NULL;
    srec = NULL;
    csv = NULL;
    dcm = NULL;
    cdfx = _cdfx;

    // copy the charList from formComapare into choosenList and listView_2
    FormCompare *fc = (FormCompare*)mainWidget;
    ui->listWidget_2->addItems(fc->charList);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a label name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    //clear the long_secription label
    ui->label_3->clear();

    // connect SLOTS
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->lineEdit_2, SIGNAL(editingFinished()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectedItem(QListWidgetItem*,QListWidgetItem*)));

    // create actions for shortCuts
    createActions();
    ui->listWidget->addAction(rightSelect);
    ui->listWidget_2->addAction(leftSelect);
}


ChooseLabel::ChooseLabel(A2LFILE *_a2l, Dcm *_dcm, QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels into " + QString(_a2l->name));
    QIcon icon(":/icones/Add record.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = NULL;
    srec = NULL;
    cdfx = NULL;
    csv = NULL;
    dcm = _dcm;

    // copy the charList from formComapare into choosenList and listView_2
    FormCompare *fc = (FormCompare*)mainWidget;
    ui->listWidget_2->addItems(fc->charList);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a label name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    //clear the long_secription label
    ui->label_3->clear();

    // connect SLOTS
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->lineEdit_2, SIGNAL(editingFinished()), this, SLOT(on_lineEdit_textChanged()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectedItem(QListWidgetItem*,QListWidgetItem*)));

    // create actions for shortCuts
    createActions();
    ui->listWidget->addAction(rightSelect);
    ui->listWidget_2->addAction(leftSelect);
}

ChooseLabel::~ChooseLabel()
{
    delete ui;
}

void ChooseLabel::createActions()
{
    QList<QKeySequence> listShortCutsRight;
    listShortCutsRight.append(Qt::Key_Left);
    listShortCutsRight.append(Qt::Key_Delete);

    leftSelect = new QAction(ui->listWidget_2);
    leftSelect->setShortcutContext(Qt::WidgetShortcut);
    leftSelect->setShortcuts(listShortCutsRight);
    connect(leftSelect, SIGNAL(triggered()), this, SLOT(on_leftButton_clicked()));

    QList<QKeySequence> listShortCutsLeft;
    listShortCutsLeft.append(Qt::Key_Right);
    listShortCutsLeft.append(Qt::Key_Space);

    rightSelect = new QAction(ui->listWidget);
    rightSelect->setShortcutContext(Qt::WidgetShortcut);
    rightSelect->setShortcuts(listShortCutsLeft);
    connect(rightSelect, SIGNAL(triggered()), this, SLOT(on_rightButton_clicked()));
}

void ChooseLabel::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void ChooseLabel::on_lineEdit_textChanged(QString str)
{
    //get the corrisponding a2l module name
    QString moduleName = "";
    if (hex)
    {
        moduleName = hex->getModuleName();
    }
    else if (srec)
    {
        moduleName = srec->getModuleName();
    }
    else if (csv)
    {
         moduleName = csv->getModuleName();
    }
    else if (dcm)
    {
         moduleName = dcm->getModuleName();
    }
    else if (cdfx)
    {
        moduleName = cdfx->getModuleName();
    }

    // display labels from subset
    if (ui->lineEdit->text().startsWith('/') && ui->lineEdit->text().size() >= 2)
    {
        Node *fun = a2l->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");
        Node *group = a2l->getProject()->getNode("MODULE/" + moduleName + "/GROUP");
        if (fun)
        {
            ui->listWidget->clear();

            QStringList strList;
            QString subsetName;
            QStringList tutu = ui->lineEdit->text().split('/');
            tutu.removeFirst();
            QString text = tutu.at(0);
            if (text.isEmpty())
            {
                ui->listWidget->clear();
                return;
            }

            foreach (Node *subset, fun->childNodes)
            {
                subsetName = subset->name;

                //QString text = ui->lineEdit->text().mid(1);
                if (subsetName.toLower().startsWith(text.toLower()))
                {
                    //get the label list from A2l
                    QStringList listLabelSubset;
                    DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)subset->getNode("DEF_CHARACTERISTIC");
                    REF_CHARACTERISTIC *ref_char = (REF_CHARACTERISTIC*)subset->getNode("REF_CHARACTERISTIC");
                    if (def_char)
                        listLabelSubset = def_char->getCharList();
                    else if (ref_char)
                        listLabelSubset = ref_char->getCharList();
                    else
                    {
                        if (group)
                        {
                            GGROUP *grp = (GGROUP*)group->getNode(subset->name);
                            if (grp)
                            {
                                DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)grp->getNode("DEF_CHARACTERISTIC");
                                REF_CHARACTERISTIC *ref_char = (REF_CHARACTERISTIC*)grp->getNode("REF_CHARACTERISTIC");
                                if (def_char)
                                    listLabelSubset = def_char->getCharList();
                                else if (ref_char)
                                    listLabelSubset = ref_char->getCharList();
                            }
                        }
                    }


                    if (csv)
                    {
                        QStringList csvLabelList = csv->getListNameData();
                        foreach (QString labelName, listLabelSubset)
                        {
                            if (!csvLabelList.contains(labelName))
                            {
                                listLabelSubset.removeOne(labelName);
                            }
                        }
                    }
                    else if (dcm)
                    {
                        QStringList dcmLabelList = dcm->getListNameData();
                        foreach (QString labelName, listLabelSubset)
                        {
                            if (!dcmLabelList.contains(labelName))
                            {
                                listLabelSubset.removeOne(labelName);
                            }
                        }
                    }
                    else if (cdfx)
                    {
                        QStringList cdfxLabelList = cdfx->getListNameData();
                        foreach (QString labelName, listLabelSubset)
                        {
                            if (!cdfxLabelList.contains(labelName))
                            {
                                listLabelSubset.removeOne(labelName);
                            }
                        }
                    }

                    // create a list of already chosen labels
                    QStringList choosenList;
                    for (int i = 0; i < ui->listWidget_2->count(); i++)
                        choosenList.append(ui->listWidget_2->item(i)->text());

                    // display only label not already chosen
                    QStringList::iterator i;
                    foreach(QString str, listLabelSubset)
                    {
                        //check if the labels are already chosen or not
                        //i = qBinaryFind(choosenList.begin(), choosenList.end(), str);
                        i = std::lower_bound(choosenList.begin(), choosenList.end(), str);
                        if (i == choosenList.end())
                            strList.append(str);
                        else
                        {
                            if (str.compare(*i) != 0)
                            {
                                strList.append(str);
                            }
                        }
                    }

                    // select the description
                    MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
                    if (ui->lineEdit_2->isEnabled() && !ui->lineEdit_2->text().isEmpty() && ui->lineEdit_2->text() != "*")
                    {
                        // search in CHARACTERISTICS
                        Node *nodeChar = module->getNode("CHARACTERISTIC");
                        if (nodeChar)
                        {
                            foreach(QString str, strList)
                            {
                               CHARACTERISTIC *charac = (CHARACTERISTIC*)nodeChar->getNode(str);
                               if (charac)
                               {
                                   QString text = ui->lineEdit_2->text();
                                   text.replace("*", ".*");
                                   QString descr(charac->fixPar("LongIdentifier").c_str());
                                   if (!descr.contains(QRegExp("^\"" + text, Qt::CaseInsensitive)))
                                   {
                                       strList.removeOne(str);
                                   }
                               }
                               else
                               {
                                   // search in AXIS_PTS
                                   Node *nodeAxis = module->getNode("AXIS_PTS");
                                   if (nodeAxis )
                                   {
                                      AXIS_PTS *axisPts = (AXIS_PTS*)nodeAxis->getNode(str);
                                      if (axisPts)
                                      {
                                          QString descr(axisPts->fixPar("LongIdentifier").c_str());
                                          if (!descr.contains(QRegExp("^" + ui->lineEdit_2->text(), Qt::CaseInsensitive)))
                                          {
                                              strList.removeOne(str);
                                          }
                                      }
                                   }
                               }
                            }
                        }
                    }
                }

            }

            if (tutu.count() == 2)
            {
                QString text = tutu.at(1);
                text.replace(".", "\\.");
                text.replace("*", ".*");
                QStringList result = strList.filter(QRegExp("^" + text, Qt::CaseInsensitive));
                ui->listWidget->addItems(result);
            }
            else
            {
                ui->listWidget->addItems(strList);
            }
        }
        else
        {
            ui->listWidget->clear();
        }
    }

    // diplay labels
    else if (ui->lineEdit->text().size() >= 1)
    {
        ui->listWidget->clear();
        MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
        if (module)
        {
            // search the labels
            QStringList list;
            if (csv)
                list = csv->getListNameData();
            else if (cdfx)
                list = cdfx->getListNameData();
            else if (dcm)
                list = dcm->getListNameData();
            else
                list = module->listChar;

            QString text = ui->lineEdit->text();
            text.replace(".", "\\.");
            text.replace("*", ".*");
            QStringList result = list.filter(QRegExp("^" + text, Qt::CaseInsensitive));

            // remove already chosen label from previous result list
            QStringList choosenList;
            for (int i = 0; i < ui->listWidget_2->count(); i++)
                choosenList.append(ui->listWidget_2->item(i)->text());
            foreach (QString str, choosenList)
            {
                result.removeOne(str);
            }

            // select the description
            if (ui->lineEdit_2->isEnabled() && !ui->lineEdit_2->text().isEmpty() && ui->lineEdit_2->text() != "*")
            {
                // search in CHARACTERISTICS
                Node *nodeChar = module->getNode("CHARACTERISTIC");
                if (nodeChar)
                {
                    foreach(QString str, result)
                    {
                       CHARACTERISTIC *charac = (CHARACTERISTIC*)nodeChar->getNode(str);
                       if (charac)
                       {
                           QString text = ui->lineEdit_2->text();
                           text.replace("*", ".*");
                           QString descr(charac->fixPar("LongIdentifier").c_str());
                           if (!descr.contains(QRegExp("^\"" + text, Qt::CaseInsensitive)))
                           {
                               result.removeOne(str);
                           }
                       }
                       else
                       {
                           // search in AXIS_PTS
                           Node *nodeAxis = module->getNode("AXIS_PTS");
                           if (nodeAxis )
                           {
                              AXIS_PTS *axisPts = (AXIS_PTS*)nodeAxis->getNode(str);
                              if (axisPts)
                              {
                                  QString descr(axisPts->fixPar("LongIdentifier").c_str());
                                  if (!descr.contains(QRegExp("^" + ui->lineEdit_2->text(), Qt::CaseInsensitive)))
                                  {
                                      result.removeOne(str);
                                  }
                              }

                           }
                       }
                    }
                }
            }

            ui->listWidget->addItems(result);
        }

    }
    else
    {
        ui->listWidget->clear();
    }
}

void ChooseLabel::on_rightButton_clicked()
{
    foreach(QListWidgetItem *item, ui->listWidget->selectedItems())
    {
        QListWidgetItem *newItem = item->clone();
        ui->listWidget_2->addItem(newItem);
        delete item;
    }
    ui->listWidget_2->sortItems();
}

void ChooseLabel::on_remove_all_clicked()
{
    ui->listWidget_2->clear();
    QString str;
    on_lineEdit_textChanged(str);
}

void ChooseLabel::on_add_all_clicked()
{
    ui->listWidget->selectAll();
    on_rightButton_clicked();
    ui->listWidget_2->sortItems();
}

void ChooseLabel::on_leftButton_clicked()
{
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
    {
        delete item;
    }
    QString str;
    on_lineEdit_textChanged(str);
}

void ChooseLabel::on_buttonBox_accepted()
{
    FormCompare *fc = (FormCompare*)mainWidget;

    fc->charList.clear();
    ui->listWidget_2->selectAll();
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
        fc->charList.append(item->text());

    // sort the list CASE SENSITIVE
    std::sort(fc->charList.begin(), fc->charList.end());

}

void ChooseLabel::on_import_2_clicked()
{
    //select a File from disk
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString currentLabPath = settings.value("currentLabPath").toString();
    QString filename = QFileDialog::getOpenFileName(this,
                                      tr("select a lab file (lab)"), currentLabPath,
                                      tr("ASAP files (*.lab);all files (*.*)"));
    if (filename.isEmpty())
    {
        return;
    }

    currentLabPath = QFileInfo(filename).absolutePath();
    settings.setValue("currentLabPath", currentLabPath);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    //read the file and save it into a QStringList
    QStringList lines;
    while (!in.atEnd())
       lines << in.readLine();

    file.close();

    if (lines.count() != 0)
    {
        if (lines.at(0) != "[Label]")
            return;
        else
        {
            QString moduleName = "";
            if (hex)
            {
                moduleName = hex->getModuleName();
            }
            else if (srec)
            {
                moduleName = srec->getModuleName();
            }
            else if (csv)
            {
                 moduleName = csv->getModuleName();
            }
            else if (cdfx)
            {
                 moduleName = cdfx->getModuleName();
            }

            MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
            if (module)
            {
                QStringList choosenList;
                for (int i = 0; i < ui->listWidget_2->count(); i++)
                    choosenList.append(ui->listWidget_2->item(i)->text());

                QStringList strList;

                for (int i = 1;i < lines.count(); i++)
                {
                    QString str = lines.at(i);
                    if (module->listChar.contains(str))
                    {
                        //check if label already in chosenList
                        //QStringList::iterator it = qBinaryFind(choosenList.begin(), choosenList.end(), lines.at(i));
                        QStringList::iterator it = std::lower_bound(choosenList.begin(), choosenList.end(), str);
                        if (it == choosenList.end())
                            strList.append(str);
                        else
                        {
                            if (str.compare(*it) != 0)
                            {
                                strList.append(str);
                            }
                        }
                    }
                }
                ui->listWidget_2->addItems(strList);
            }
        }
    }
    else
        return;
}

void ChooseLabel::on_export_2_clicked()
{
    QStringList strList;
    strList.append("[Label]");
    ui->listWidget_2->selectAll();
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
        strList.append(item->text());

    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString currentLabPath = settings.value("currentLabPath").toString();
    QString fileName = QFileDialog::getSaveFileName(this,"save lab files", currentLabPath,
                                                    "lab files (*.lab);;all files (*.*)");
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".lab"))
    {
        fileName.append(".lab");
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
    foreach (QString str, strList)
        out << str << "\r\n";
    QApplication::restoreOverrideCursor();
}

void ChooseLabel::selectedItem(QListWidgetItem* item,QListWidgetItem*)
{
    if (!item)
    {
        ui->label_3->clear();
        return;
    }

    QString str = item->text();

    // search in CHARACTERISTICS
    QString moduleName = "";
    if (hex)
    {
        moduleName = hex->getModuleName();
    }
    else if (srec)
    {
        moduleName = srec->getModuleName();
    }
    else if (csv)
    {
         moduleName = csv->getModuleName();
    }
    else if (cdfx)
    {
         moduleName = cdfx->getModuleName();
    }
    else
        return;

    MODULE *module = (MODULE*)a2l->getProject()->getNode("MODULE/" + moduleName);
    Node *nodeChar = module->getNode("CHARACTERISTIC");
    if (nodeChar)
    {
       CHARACTERISTIC *charac = (CHARACTERISTIC*)nodeChar->getNode(str);
       if (charac)
       {
           QString descr(charac->fixPar("LongIdentifier").c_str());
           ui->label_3->setText(descr);
       }
       else
       {
           // search in AXIS_PTS
           Node *nodeAxis = module->getNode("AXIS_PTS");
           if (nodeAxis )
           {
                  AXIS_PTS *axisPts = (AXIS_PTS*)nodeAxis->getNode(str);
                  if (axisPts)
                  {
                      QString descr(axisPts->fixPar("LongIdentifier").c_str());
                      ui->label_3->setText(descr);
                  }
           }
       }
    }
}

void ChooseLabel::searchItem()
{
    timer.stop();
    timer.start(750);
}

