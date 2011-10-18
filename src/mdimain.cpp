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

#include <QtGui>
#include <QtCore>
#include <cstdarg>
#include <omp.h>
#include <typeinfo>
#include <qtconcurrentrun.h>
#include <QFutureWatcher>
#include <sys/stat.h>

#ifdef Q_WS_WIN32
    #include <QAxObject>
#endif

#include "a2ltreemodel.h"
#include "mdimain.h"
#include "ui_mdimain.h"
#include "formeditor.h"
#include "workproject.h"
#include "chtextedit.h"
#include "a2ltreemodel.h"
#include "lexer.h"
#include "Nodes/a2lfile.h"
#include "Nodes/a2ml.h"
#include "chtextedit.h"
#include "treemodelcompleter.h"
#include "dialog.h"
#include "formcompare.h"
#include "a2lgrammar.h"
#include "data.h"
#include "diffmodel.h"
#include "csv.h"
#include "dialogcsv.h"
#include "dialogupdate.h"
#include "mainwindow.h"
#include "spreadsheetview.h"
#include "sptablemodel.h"
#include "dialogchoosemodule.h"
#include <QtConcurrentRun>
#include "measmodel.h"
#include "charmodel.h"
#include <Qsci/qscilexerjavascript.h>
#include "formscript.h"
#include "cdfxfile.h"
#include <Qsci/qscilexerxml.h>
#include "choosesubset.h"
#include "dialogchooseexportformat.h"
#ifdef Q_WS_WIN32
    #include "LMCons.h"
    #include "windows.h"
#endif
#include "freezetablewidget.h"
#include "dialoghttpupdate.h"
//#include "ciao.h"

#include "qdebug.h"

MDImain::MDImain(QWidget *parent) : QMainWindow(parent), ui(new Ui::MDImain)
{
    //initialize ui
    ui->setupUi(this);

    setStyleSheet(
            "QMainWindow::separator {"
                 "width: 3px;"
                 "height: 3px;}"

             "QMainWindow::separator:hover {"
                 "}"

                 );

    ui->tabWidget->clear();
    ui->tabWidget->setAcceptDrops(true);
    this->readSettings();
    ui->dockWidget->setVisible(false);
    showMaximized();


    // menus / toolbars / statusbar
    createActions();
    createMenus();
    initToolBars();
    progBar = new QProgressBar(this);
    statusBar()->addWidget(progBar, 1);
    statusBar()->hide();
    compareTabs = 0;

    //initialize Treeview
    ui->treeView->header()->hide();
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(resizeColumn0()));
    connect(ui->treeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(resizeColumn0()));
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);

    //initialize Treeview_2 (DiffLabel)
    ui->treeView_2->header()->hide();
    ui->treeView_2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView_2, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu_2(QPoint)));
    connect(ui->treeView_2, SIGNAL(expanded(QModelIndex)), this, SLOT(resizeColumn0()));
    connect(ui->treeView_2, SIGNAL(collapsed(QModelIndex)), this, SLOT(resizeColumn0()));

    //initialize Output dockpanel
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->outputListWidget->resize(20000, 50);

    //Model
    model = new A2lTreeModel(0);

    //completer
    completer = new TreeModelCompleter(this);
    completer->setSeparator(QLatin1String("/"));
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(20);
    ui->lineEdit->setCompleter(completer);
    ui->lineEdit->setFocus();

    //initialize pointers
    projectList = new QMap<QString, WorkProject*>;
    tabList = new QMap<QString, QWidget*>;

    //connect the slots
    connect(ui->tabWidget, SIGNAL(hexDropped(QString)), this, SLOT(checkDroppedFile(QString)));
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(nodeSelected()));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));

    // check for updates
    //connect(this, SIGNAL(check()), this, SLOT(initCheckUpdates()), Qt::QueuedConnection);
    connect(this, SIGNAL(check()), this, SLOT(initCheckHttpUpdates()), Qt::QueuedConnection);
    emit check();

    // check for message
//    connect(this, SIGNAL(adieu()), this, SLOT(say_Ciao()), Qt::QueuedConnection);
//    emit adieu();

}

MDImain::~MDImain()
{
    delete ui;
    delete projectList;
    delete tabList;
}

// ----------------- Menu -----------------------//
void MDImain::closeEvent(QCloseEvent *e)
{
    if (checkChangedHexFiles() && checkChangedCsvFiles())
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        e->accept();
    }
    else
        e->ignore();
}

void MDImain::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MDImain::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MDImain::createActions()
{
    //actions for recent files menu
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    importSubsets = new QAction(tr("import subsets"), this);
    importSubsets->setIcon(QIcon(":/icones/milky_importSubset.png"));
    connect(importSubsets, SIGNAL(triggered()), this, SLOT(import_Subsets()));
    importSubsets->setDisabled(true);

    exportSubsets = new QAction(tr("export subsets"), this);
    exportSubsets->setIcon(QIcon(":/icones/milky_exportSubset.png"));
    connect(exportSubsets, SIGNAL(triggered()), this, SLOT(export_Subsets()));
    exportSubsets->setDisabled(true);

    openJScript = new QAction(tr("run java script"), this);
    openJScript->setIcon(QIcon(":/icones/run.png"));
    connect(openJScript, SIGNAL(triggered()), this, SLOT(newFormScript()));
    openJScript->setDisabled(true);

    editMeasChannels = new QAction(tr("edit measuring channels"), this);
    editMeasChannels->setIcon(QIcon(":/icones/milky_peigne.png"));
    connect(editMeasChannels, SIGNAL(triggered()), this, SLOT(editMeasuringChannels()));
    editMeasChannels->setDisabled(true);

    editCharacteristics = new QAction(tr("edit characteristics"), this);
    editCharacteristics->setIcon(QIcon(":/icones/milky_outils.png"));
    connect(editCharacteristics, SIGNAL(triggered()), this, SLOT(editChar()));
    editCharacteristics->setDisabled(true);

    exportListData = new QAction(tr("export list"), this);
    exportListData->setIcon(QIcon(":/icones/export.png"));
    connect(exportListData, SIGNAL(triggered()), this, SLOT(export_ListData()));
    exportListData->setDisabled(false);

    deleteProject = new QAction(tr("remove project"), this);
    deleteProject->setIcon(QIcon(":/icones/milky_ciseau.png"));
    deleteProject->setShortcut(Qt::Key_Delete);
    connect(deleteProject, SIGNAL(triggered()), this, SLOT(removeWorkProjects()));
    deleteProject->setDisabled(true);

    addHexFile = new QAction(tr("Import Hex file"), this);
    addHexFile->setIcon(QIcon(":/icones/milky_importHEX.png"));
    connect(addHexFile, SIGNAL(triggered()), this, SLOT(addHexFile2Project()));
    addHexFile->setDisabled(true);

    addCsvFile = new QAction(tr("Import Csv file"), this);
    addCsvFile->setIcon(QIcon(":/icones/milky_importCsv.png"));
    connect(addCsvFile, SIGNAL(triggered()), this, SLOT(addCsvFile2Project()));
    addCsvFile->setDisabled(true);

    addCdfxFile = new QAction(tr("Import Cdf file"), this);
    addCdfxFile->setIcon(QIcon(":/icones/milky_importCdfx.png"));
    connect(addCdfxFile, SIGNAL(triggered()), this, SLOT(addCdfxFile2Project()));
    addCdfxFile->setDisabled(true);

    resetAllChangedData = new QAction(tr("reset all changes"), this);
    resetAllChangedData->setIcon(QIcon(":/icones/milky_resetAll.png"));
    connect(resetAllChangedData, SIGNAL(triggered()), this, SLOT(reset_AllChangedData()));
    resetAllChangedData->setDisabled(true);

    sortBySubset = new QAction(tr("sort changes by subset"), this);
    sortBySubset->setIcon(QIcon(":/icones/viewtree.png"));
    connect(sortBySubset, SIGNAL(triggered()), this, SLOT(sort_BySubset()));
    sortBySubset->setDisabled(true);

    editFile = new QAction(tr("edit as text"), this);
    editFile->setIcon(QIcon(":/icones/milky_editText.png"));
    connect(editFile, SIGNAL(triggered()), this, SLOT(editTextFile()));
    editFile->setDisabled(true);

    deleteFile = new QAction(tr("remove file from project"), this);
    deleteFile->setIcon(QIcon(":/icones/milky_ciseau.png"));
    deleteFile->setShortcut(Qt::Key_Delete);
    connect(deleteFile, SIGNAL(triggered()), this, SLOT(deleteFilesFromProject()));
    deleteFile->setDisabled(true);

    saveFile = new QAction(tr("Save"), this);
    saveFile->setIcon(QIcon(":/icones/milky_save.png"));
    connect(saveFile, SIGNAL(triggered()), this, SLOT(save_File()));
    saveFile->setDisabled(true);

    saveAsFile = new QAction(tr("Save as ..."), this);
    saveAsFile->setIcon(QIcon(":/icones/milky_saveas.png"));
    connect(saveAsFile, SIGNAL(triggered()), this, SLOT(saveAs_File()));
    saveAsFile->setDisabled(true);

    compareHexFile = new QAction(tr("compare dataset (HEX)"), this);
    compareHexFile->setIcon(QIcon(":/icones/copy.png"));
    connect(compareHexFile, SIGNAL(triggered()), this, SLOT(compare_HexFile()));
    compareHexFile->setDisabled(false);

    compareA2lFile = new QAction(tr("compare A2L files"), this);
    compareA2lFile->setIcon(QIcon(":/icones/copy.png"));
    connect(compareA2lFile, SIGNAL(triggered()), this, SLOT(compare_A2lFile()));
    compareA2lFile->setDisabled(false);

    quicklook = new QAction(tr("quicklook"), this);
    quicklook->setIcon(QIcon(":/icones/milky_loopHex.png"));
    connect(quicklook, SIGNAL(triggered()), this, SLOT(quicklookFile()));
    quicklook->setDisabled(true);

    resetData = new QAction(tr("reset data"), this);
    resetData->setIcon(QIcon(":/icones/milky_resetAll.png"));
    connect(resetData, SIGNAL(triggered()), this, SLOT(reset_Data()));

    clearOutput = new QAction(tr("clear"), this);
    clearOutput->setIcon(QIcon(":/icones/milky_pinceau.png"));
    connect(clearOutput, SIGNAL(triggered()), this, SLOT(clear_Output()));

    plotData = new QAction(tr("plot"), this);
    plotData->setIcon(QIcon(":/icones/AXIS.bmp"));
    plotData->setStatusTip(tr("plot"));
    connect(plotData, SIGNAL(triggered()), this, SLOT(fplotData()));

    readValuesFromCsv = new QAction(tr("import CSV values"), this);
    readValuesFromCsv->setIcon(QIcon(":/icones/milky_importCsv.png"));
    readValuesFromCsv->setDisabled(true);
    connect(readValuesFromCsv, SIGNAL(triggered()), this, SLOT(read_ValuesFromCsv()));

    readValuesFromCdfx = new QAction(tr("import CDF values"), this);
    readValuesFromCdfx->setIcon(QIcon(":/icones/milky_importCdfx.png"));
    readValuesFromCdfx->setDisabled(true);
    connect(readValuesFromCdfx, SIGNAL(triggered()), this, SLOT(read_ValuesFromCdfx()));

    showParam = new QAction(tr("show fix parameters"), this);
    connect(showParam, SIGNAL(triggered()), this, SLOT(showFixPar()));
    showParam->setDisabled(true);

    childCount = new QAction(tr("count elements"), this);
    connect(childCount, SIGNAL(triggered()), this, SLOT(countChild()));
    childCount->setDisabled(true);

    verify = new QAction(tr("verify"), this);
    connect(verify, SIGNAL(triggered()), this, SLOT(verifyMaxTorque()));
    verify->setDisabled(true);

    checkFmtc = new QAction(tr("check FMTC monotony"), this);
    connect(checkFmtc, SIGNAL(triggered()), this, SLOT(checkFmtcMonotony()));
    checkFmtc->setDisabled(true);

    editChanged = new QAction(tr("edit all changes"), this);
    editChanged->setIcon(QIcon(":/icones/milky_find.png"));
    connect(editChanged, SIGNAL(triggered()), this, SLOT(editChangedLabels()));
    editChanged->setDisabled(true);

    editLabel= new QAction(tr("edit"), this);
    editLabel->setIcon(QIcon(":/icones/eye.png"));
    connect(editLabel, SIGNAL(triggered()), this, SLOT(edit()));
    editLabel->setDisabled(false);

    toolsMenu = new QMenu();
    toolsMenu->setTitle("tools");
    toolsMenu->setIcon(QIcon(":/icones/ToolboxFolder.png"));
    //toolsMenu->setIcon(QIcon(":/icones/milky_outils.png"));
    toolsMenu->addAction(verify);
    toolsMenu->addAction(checkFmtc);
    toolsMenu->addAction(childCount);

    recentProMenu = new QMenu();
    recentProMenu->setTitle("recent projects");

}

void MDImain::createMenus()
{
    //window menu
    QMenu* menu = this->createPopupMenu();
    menu->setTitle("Docks and Toolbars");
    ui->menuWindow->addMenu(menu);

    // create recent files menu
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
         ui->menuRecent_Files->addAction(recentFileActs[i]);
        //recentFileActs[i]->setParent(ui->actionRecent_Files);
    }
    updateRecentFileActions();

}

void MDImain::on_listWidget_customContextMenuRequested()
{
    QMenu menu;
    menu.addAction(clearOutput);
    menu.exec(QCursor::pos());
}

void MDImain::initToolBars()
{
    // Project : A2l
    ui->toolBar_a2l->addAction(ui->actionNewA2lProject);    
    ui->toolBar_a2l->addAction(addHexFile);
    ui->toolBar_a2l->addAction(addCsvFile);
    ui->toolBar_a2l->addAction(addCdfxFile);
    ui->toolBar_a2l->addSeparator();
    ui->toolBar_a2l->addAction(deleteProject);
    ui->toolBar_a2l->addAction(editFile);
    ui->toolBar_a2l->addAction(editMeasChannels);
    ui->toolBar_a2l->addAction(editCharacteristics);
    //ui->toolBar_a2l->addAction(openJScript);

    // Data : Hex or Csv or Cdf file
    ui->toolBar_data->addAction(quicklook);
    ui->toolBar_data->addAction(saveFile);
    ui->toolBar_data->addAction(saveAsFile);
    ui->toolBar_data->addAction(deleteFile);
    ui->toolBar_data->addSeparator();
    ui->toolBar_data->addAction(readValuesFromCsv);
    ui->toolBar_data->addAction(resetAllChangedData);
    ui->toolBar_data->addAction(sortBySubset);
    ui->toolBar_data->hide();
}

void MDImain::on_actionExit_triggered()
{
    close();
}

void MDImain::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        //loadFile(action->data().toString());
        if (QFile::exists(action->data().toString()))
        {
            QString qstr = action->data().toString();
            openProject(qstr);
        }
        else
        {
            QMessageBox::information(this, "Hexplorer :: open recent project",
                                     "the file " + action->data().toString() + " cannot be found.");
        }
    }
}

void MDImain::on_treeView_clicked(QModelIndex index)
{
    //get the type of the selected node
    Node *node = model->getNode(index);
    QString name = typeid(*node).name();

    //update the context menu
    if (name.endsWith("A2LFILE"))
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(true);
        addCsvFile->setEnabled(true);
        editChanged->setEnabled(false);
        addHexFile->setEnabled(true);
        deleteProject->setEnabled(true);
        deleteFile->setEnabled(false);
        editFile->setEnabled(true);
        childCount->setEnabled(true);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(false);
        sortBySubset->setEnabled(false);
        saveFile->setEnabled(false);
        saveAsFile->setEnabled(false);
        quicklook->setEnabled(false);
        readValuesFromCsv->setEnabled(false);
        readValuesFromCdfx->setEnabled(false);
        editMeasChannels->setEnabled(true);
        editCharacteristics->setEnabled(true);
        openJScript->setEnabled(true);
    }
    else if (name.endsWith("HexFile"))
    {
        importSubsets->setEnabled(true);
        exportSubsets->setEnabled(true);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        deleteProject->setEnabled(false);
        deleteFile->setEnabled(true);
        editFile->setEnabled(true);
        childCount->setEnabled(true);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(true);
        sortBySubset->setEnabled(true);
        saveFile->setEnabled(true);
        saveAsFile->setEnabled(true);
        quicklook->setIcon(QIcon(":/icones/milky_loopHex.png"));
        quicklook->setEnabled(true);
        readValuesFromCsv->setEnabled(true);
        readValuesFromCdfx->setEnabled(true);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);

        ui->toolBar_data->show();

    }
    else if (name.endsWith("Csv"))
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        deleteProject->setEnabled(false);
        deleteFile->setEnabled(true);
        editFile->setEnabled(true);
        childCount->setEnabled(true);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(true);
        sortBySubset->setEnabled(true);
        saveFile->setEnabled(true);
        saveAsFile->setEnabled(true);
        quicklook->setIcon(QIcon(":/icones/milky_loopCsv.png"));
        quicklook->setEnabled(true);
        readValuesFromCsv->setEnabled(false);
        readValuesFromCdfx->setEnabled(false);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);

        ui->toolBar_data->show();
    }
    else if (name.endsWith("CdfxFile"))
    {
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        deleteProject->setEnabled(false);
        deleteFile->setEnabled(true);
        editFile->setEnabled(true);
        childCount->setEnabled(true);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(true);
        sortBySubset->setEnabled(true);
        saveFile->setEnabled(true);
        saveAsFile->setEnabled(true);
        quicklook->setIcon(QIcon(":/icones/milky_loopCsv.png"));
        quicklook->setEnabled(true);
        readValuesFromCsv->setEnabled(false);
        readValuesFromCdfx->setEnabled(false);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);

        ui->toolBar_data->show();
    }
    else
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        deleteProject->setEnabled(false);
        deleteFile->setEnabled(false);
        editFile->setEnabled(false);
        childCount->setEnabled(true);
        showParam->setEnabled(true);
        resetAllChangedData->setEnabled(false);
        sortBySubset->setEnabled(false);
        saveFile->setEnabled(false);
        saveAsFile->setEnabled(false);
        quicklook->setEnabled(false);
        readValuesFromCsv->setEnabled(false);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);
    }

    //get the full path of the index into treeView
    QString path = QString(model->getNodeName(index).c_str());
    index = model->parent(index);
    while (index.isValid())
    {
        path = QString(model->getNodeName(index).c_str()) + "/" + path;
        index = model->parent(index);
    }
}

void MDImain::showContextMenu(QPoint)
{
    QMenu menu;

    if (model->getRootNode() == 0)
    {
        menu.addAction(ui->actionNewA2lProject);

        for (int i = 0; i < MaxRecentFiles; ++i)
        {
             recentProMenu->addAction(recentFileActs[i]);
        }
        updateRecentFileActions();
        menu.addMenu(recentProMenu);
    }
    else
    {
        QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();
        if (list.count() < 1)
        {
            menu.addAction(ui->actionNewA2lProject);

            for (int i = 0; i < MaxRecentFiles; ++i)
            {
                 recentProMenu->addAction(recentFileActs[i]);
            }
            updateRecentFileActions();
            menu.addMenu(recentProMenu);
        }
        else if (list.count() == 1)
        {
            QModelIndex index  = list.at(0);

            Node *node = model->getNode(index);
            QString name = typeid(*node).name();

            if (name.endsWith("Data"))
            {
                menu.addAction(resetData);
                menu.addAction(editLabel);
                menu.addAction(plotData);                                
            }
            else if (name.toLower().endsWith("hexfile"))
            {                                
                menu.addSeparator();
                menu.addAction(quicklook);
                menu.addAction(editFile);
                menu.addSeparator();
                menu.addAction(readValuesFromCsv);
                menu.addAction(readValuesFromCdfx);
                menu.addSeparator();
                //menu.addAction(importSubsets);
                menu.addAction(exportSubsets);
                menu.addSeparator();
                menu.addAction(editChanged);
                menu.addAction(resetAllChangedData);
                menu.addAction(sortBySubset);                
                menu.addMenu(toolsMenu);
                menu.addSeparator();
                menu.addAction(saveFile);
                menu.addAction(saveAsFile);
                menu.addAction(deleteFile);

                //menu editModified
                Node *node =  model->getNode(index);
                HexFile *hex = dynamic_cast<HexFile *> (node);

                if (hex->getModifiedData().isEmpty())
                {
                    sortBySubset->setDisabled(true);
                    resetAllChangedData->setDisabled(true);
                    editChanged->setDisabled(true);
                }
                else
                {
                    sortBySubset->setDisabled(false);
                    resetAllChangedData->setDisabled(false);
                    editChanged->setDisabled(false);
                }

                //menu tools
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (hex->getParentNode());
                QString projectName = ((PROJECT*)a2l->getProject())->getPar("name");
                projectName = projectName.toLower();

                if (projectName == "c340" || projectName == "c342" || projectName == "p_662")
                {
                    verify->setDisabled(false);
                    checkFmtc->setDisabled(false);
                }
                else
                {
                    verify->setDisabled(true);
                    checkFmtc->setDisabled(true);
                }

            }
            else if (name.toLower().endsWith("a2lfile"))
            {
                A2LFILE *a2lFile = (A2LFILE*)node;
                if (a2lFile->isConform())
                {
                    menu.addAction(ui->actionNewA2lProject);
                    menu.addAction(deleteProject);
                    menu.addAction(editFile);                    
                    menu.addSeparator();
                    menu.addAction(addHexFile);
                    menu.addAction(addCsvFile);
                    menu.addAction(addCdfxFile);
                    menu.addSeparator();
                    menu.addAction(openJScript);
                    menu.addSeparator();
                    menu.addAction(editMeasChannels);
                    menu.addAction(editCharacteristics);
                    //menu.addSeparator();
                    //menu.addAction(showParam);
                    //menu.addAction(childCount);
                }
                else
                {
                    menu.addAction(ui->actionNewA2lProject);
                    menu.addAction(deleteProject);
                    menu.addSeparator();
                    menu.addAction(editFile);
                }
            }
            else if (name.toLower().endsWith("csv"))
            {
                menu.addAction(deleteFile);
                menu.addAction(editFile);
                menu.addSeparator();
                menu.addAction(saveFile);
                menu.addAction(saveAsFile);
                menu.addSeparator();
                menu.addAction(quicklook);
                menu.addAction(editChanged);
                menu.addAction(resetAllChangedData);
                menu.addAction(sortBySubset);

                //menu editModified
                Node *node =  model->getNode(index);
                Csv *csv = dynamic_cast<Csv *> (node);

                if (csv->getModifiedData().isEmpty())
                    editChanged->setDisabled(true);
                else
                    editChanged->setDisabled(false);
            }
            else if (name.toLower().endsWith("cdfxfile"))
            {
                menu.addAction(deleteFile);
                menu.addAction(editFile);
                menu.addSeparator();
                menu.addAction(saveFile);
                menu.addAction(saveAsFile);
                menu.addSeparator();
                menu.addAction(quicklook);
                menu.addAction(editChanged);
                menu.addAction(resetAllChangedData);
                menu.addAction(sortBySubset);

                //menu editModified
                Node *node =  model->getNode(index);
                CdfxFile *cdfx = dynamic_cast<CdfxFile *> (node);

                if (cdfx->getModifiedData().isEmpty())
                    editChanged->setDisabled(true);
                else
                    editChanged->setDisabled(false);
            }
            else
            {
                menu.addAction(ui->actionNewA2lProject);
                menu.addAction(deleteProject);
                menu.addAction(addHexFile);                
                menu.addAction(editFile);
                menu.addSeparator();
                menu.addAction(showParam);
                menu.addAction(childCount);
            }
        }
        else if (list.count() >= 2)
        {
            int weiterData = 0;
            int weiterHex = 0;
            int weiterA2l = 0;
            foreach (QModelIndex index, list)
            {
                Node *node = model->getNode(index);
                QString name = typeid(*node).name();
                if (name.endsWith("Data"))
                {
                    weiterData++;
                }
                else if (name.toLower().endsWith("hexfile"))
                {
                    weiterHex++;
                }
                else if (name.toLower().endsWith("a2lfile"))
                {
                    weiterA2l++;
                }
            }

            if (weiterData == list.count())
            {
                menu.addAction(resetData);
                menu.addAction(editLabel);
                menu.addAction(plotData);
            }
            else if (weiterHex == list.count())
            {                
                menu.addAction(deleteFile);
                if (weiterHex == 2)
                    menu.addAction(compareHexFile);
            }
            else if (weiterA2l == list.count())
            {
                menu.addAction(deleteProject);
                if (weiterA2l == 2)
                    menu.addAction(compareA2lFile);
            }
        }
    }

    menu.exec(QCursor::pos());
}

void MDImain::showContextMenu_2(QPoint)
{
    QMenu menu;

    QModelIndex index  = ui->treeView_2->selectionModel()->currentIndex();
    if (index.isValid())
    {
        menu.addAction(exportListData);
    }

    menu.exec(QCursor::pos());
}

void MDImain::verifyMaxTorque()
{
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();
    HexFile *hex = (HexFile*)model->getNode(index);
    hex->verify();
}

void MDImain::checkFmtcMonotony()
{
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();
    HexFile *hex = (HexFile*)model->getNode(index);
    bool bl;
    QList<int> list = hex->checkFmtcMonotony(&bl);

    if (bl)
    {
        QMessageBox::information(this, "HEXplorer :: FMTC monotony",
                                 "the whole FMTC map is strictly monotuous");
    }
    else
    {
        QString str;
        foreach (int col, list)
        {
            str.append(QString::number(col + 1) + ", ");
        }
        str.chop(2);
        QMessageBox::warning(this, "HEXplorer :: FMTC monotony",
                             "the FMTC map is NOT strictly monotuous : \n"
                             "check column(s) : " + str);
    }
}

void MDImain::on_actionSettings_triggered()
{
    MainWindow *win = new MainWindow();
    win->show();
}

void MDImain::on_actionAbout_triggered()
{
    QString text = "Christophe Hoel"
                   "\n\n"
                   "special thanks to :\n"
                   "Oscar, Niklaus, Jimi, Zack, Eric, Oneyed Jack, lofofora\n"
                   "M, Radio Tarifa, Al, John, Paco, Noir dez, et tous les autres...\n\n"
                   "build " + qApp->applicationVersion() + " compiled with MSVC2010\n\n"
                   "This software uses external libraries :\n"
                   "   - Qt framework 4.7.4 (Nokia)\n"
                   "   - Quex 0.59.7 (as efficient lexical analyser generator)\n"
                   "   - QScintilla (as efficient text editor)\n"
                   "   - Qwt (as 2D graph plotter)\n"
                   "   - Qwtplot3D (as 3D graph plotter)\n"
                   "   - Winhoard (as efficient malloc-replacement for windows)\n\n"
                   "Please visit the following link for more information :\n"
            "http://code.google.com/p/hexplorer/";
    QMessageBox::about(this, tr("HEXplorer :: About"), tr(text.toLocal8Bit().data()));
}

void MDImain::on_actionCreate_database_triggered()
{
    //openInit();
}

void MDImain::on_actionSave_session_triggered()
{
    //serialize();
}

void MDImain::modelUpdate()
{
    model->update();
}

void MDImain::doubleClicked(QModelIndex)
{
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();
    Node *node = model->getNode(index);

    QString name = typeid(*node).name();
    if (name.toLower().endsWith("hexfile"))
    {
        quicklookFile();
    }
}

bool MDImain::checkChangedHexFiles()
{
    bool bl = true;

    // check if Hex files have changes
    foreach (WorkProject *wp, projectList->values())
    {
        foreach (HexFile *hex, wp->hexFiles().values())
        {
            if (hex->childNodes.count() != 0)
            {
                int ret = QMessageBox::question(this, "HEXplorer :: exit",
                                      "Save changes to " + hex->fullName(),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

                if (ret == QMessageBox::Yes)
                {
                    QModelIndex index = ((A2lTreeModel*)ui->treeView->model())->getIndex(hex);
                    save_HexFile(index);
                    bl = true;
                }
                else if (ret == QMessageBox::No)
                {
                    bl = true;
                }
                else
                {
                    return false;
                }

            }
        }
    }

    return bl;
}

bool MDImain::checkChangedCsvFiles()
{
    bool bl = true;

    // check if Hex files have changes
    foreach (WorkProject *wp, projectList->values())
    {
        foreach (Csv *csv, wp->csvFiles().values())
        {
            if (csv->childNodes.count() != 0)
            {
                int ret = QMessageBox::question(this, "HEXplorer :: exit",
                                      "Save changes to " + csv->fullName(),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

                if (ret == QMessageBox::Yes)
                {
                    QModelIndex index = ((A2lTreeModel*)ui->treeView->model())->getIndex(csv);
                    save_CsvFile(index);
                    bl = true;
                }
                else if (ret == QMessageBox::No)
                {
                    bl = true;
                }
                else
                {
                    return false;
                }

            }
        }
    }

    return bl;
}

// ------------------ A2lFile ----------------------//

void MDImain::on_actionNewA2lProject_triggered()
{
    QSettings settings;
    QString path = settings.value("currentA2lPath").toString();

    const QStringList files =
        QFileDialog::getOpenFileNames(this,
                                      tr("select an Asap file (A2L)"), path,
                                      tr("ASAP files (*.a2l);;all files (*.*)"));

    if (files.isEmpty())
    {
       statusBar()->showMessage(tr("Loading canceled"), 2000);
       writeOutput("action open new project : no file selected");
       return;
    }
    else
    {
        //intialize projectList if necessary
       if (projectList == NULL)
            projectList = new QMultiMap<QString, WorkProject*>;

       //check if the project is already open
       foreach (QString fileName, files)
       {
           openProject(fileName);
           ui->treeView->resizeColumnToContents(0);
       }
   }
}

void MDImain::openProject(QString &fullFileName)
{
    if (projectList->contains(fullFileName))
    {
        QMessageBox::information(this, "HEXplorer", fullFileName + " already open !");
        return;
    }

    //update currentA2lPath
    QSettings settings;
    QString currentA2lPath = QFileInfo(fullFileName).absolutePath();
    settings.setValue("currentA2lPath", currentA2lPath);

    // create a new Wp
    WorkProject *wp = new WorkProject(fullFileName, this->model, this);

    // display status bar
    statusBar()->show();
    progBar->reset();
    connect(wp, SIGNAL(incProgressBar(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

    // parse the a2l file
    wp->parse();
    wp->attach(this);

    // hide the statusbar
    statusBar()->hide();
    progBar->reset();

    //update the ui->treeView
    model->addNode2RootNode(wp->a2lFile);
    ui->treeView->setModel(model);
    ui->treeView->setColumnHidden(1, true);

    //set completer
    completer->setModel(model);

    //insert the new created project into the projectList
    projectList->insert(fullFileName, wp);

    //messagebox in case of a2l parser error
    if (!wp->isOk())
    {
        QMessageBox::warning(this, "HEXplorer" ,"ASAP file parser error");
    }

    //update output console
    ui->listWidget->addItems(wp->_outputList());
    ui->listWidget->scrollToBottom();

    //update current file
    setCurrentFile(fullFileName);
}

void MDImain::addHexFile2Project()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::add hex file to project", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a project");
            writeOutput("action open new dataset cancelled: no project first selected");
            return;
        }
        else
        {
            QSettings settings;
            QString path = settings.value("currentHexPath").toString();

            QStringList files =
                    QFileDialog::getOpenFileNames(this,
                                              tr("select a dataset (HEX)"), path,
                                              tr("HEX files (*.hex | *.hex_trimmed);;all files (*.*)"));


            if (files.isEmpty())
            {
               statusBar()->showMessage(tr("Loading canceled"), 2000);
               writeOutput("action open new dataset : no HEX file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(index);

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                if (wp)  //to prevent any crash of the aplication
                {
                    // if no MOD_COMMON in ASAP file
                    if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                    {
                        QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                        writeOutput("action open new dataset : no MOD_COMMON in ASAP file");
                        return;
                    }

                    // check if Hexfile already in project
                    foreach (QString fullHexName, files)
                    {
                        //if the selected Hex file is already into the project => exit
                        if (wp->hexFiles().contains(fullHexName))
                        {
                            QMessageBox::information(this, "HEXplorer", "HEX file : " + fullHexName
                                                     + "\nalready included into the selected project");
                            writeOutput("action open new dataset : HEX file already in project");
                            files.removeOne(fullHexName);
                        }
                    }

                    //Open hex files
                    foreach (QString fullHexName, files)
                    {
                        //update currentHexPath
                        QSettings settings;
                        QString currentHexPath = QFileInfo(fullHexName).absolutePath();
                        settings.setValue("currentHexPath", currentHexPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project (module)
                        HexFile *hex = NULL;
                        QList<MODULE*> list = wp->a2lFile->getProject()->listModule();
                        if (list.count() == 0)
                        {
                            writeOutput("action open new dataset : no Module into A2l file !");
                            return;
                        }
                        else if (list.count() == 1)
                        {
                            hex = new HexFile(fullHexName, wp, QString(list.at(0)->name));
                        }
                        else
                        {
                            // select a module
                            QString module;
                            DialogChooseModule *diag = new DialogChooseModule(&module);
                            QStringList listModuleName;
                            foreach (MODULE* module, list)
                            {
                                listModuleName.append(module->name);
                            }
                            diag->setList(listModuleName);
                            int ret = diag->exec();

                            if (ret == QDialog::Accepted)
                            {
                                hex = new HexFile(fullHexName, wp, module);
                            }
                            else
                            {
                                writeOutput("action open new dataset : no module chosen !");
                                return;
                            }
                        }

                        // display status bar
                        statusBar()->show();
                        progBar->reset();
                        //connect(hex, SIGNAL(lineParsed(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);
                        connect(hex, SIGNAL(progress(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                        if (hex->read())
                            wp->addHex(hex);
                        else
                            delete hex;

                        // hide the statusbar
                        statusBar()->hide();
                        progBar->reset();

                        //stop timer
                        double tf = omp_get_wtime();

                        //update the treeView model
                        ui->treeView->expand(index);
                        ui->treeView->resizeColumnToContents(0);

                        writeOutput("action open new dataset : HEX file add to project in " + QString::number(tf-ti) + " sec");

                    }
                }
            }
        }
    }
}

void MDImain::addCsvFile2Project()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::add csv file to project", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a project");
            writeOutput("action open new csv file cancelled: no project first selected");
            return;
        }
        else
        {
            QSettings settings;
            QString path = settings.value("currentCsvPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
                                              tr("select CSV files"), path,
                                              tr("CSV files (*.csv);;all files (*.*)"));

            if (files.isEmpty())
            {
               writeOutput("action open new CSV file : no CSV file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(index);

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                if (wp)  //to prevent any crash of the aplication
                {
                    // if no MOD_COMMON in ASAP file
                    if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                    {
                        QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                        writeOutput("action open new CSV file : no MOD_COMMON in ASAP file");
                        return;
                    }

                    // check if Csv already in project
                    foreach (QString fullCsvName, files)
                    {
                        //if the selected Hex file is already into the project => exit
                        if (wp->csvFiles().contains(fullCsvName))
                        {
                            QMessageBox::information(this, "HEXplorer", tr("file already in project"));
                            writeOutput("action open new CSV file : CSV file already in project");
                            files.removeOne(fullCsvName);
                            return;
                        }
                    }

                    // Read CSV files
                    QList<Csv*> list;
                    foreach (QString fullCsvName, files)
                    {
                        //update currentHexPath
                        QSettings settings;
                        QString currentCsvPath = QFileInfo(fullCsvName).absolutePath();
                        settings.setValue("currentCsvPath", currentCsvPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project
                        QString moduleName;
                        QList<MODULE*> listModule = wp->a2lFile->getProject()->listModule();
                        if (listModule.count() == 0)
                        {
                            writeOutput("action open new CSV : no Module into A2l file !");
                            return;
                        }
                        else if (listModule.count() == 1)
                        {
                            moduleName = listModule.at(0)->name;
                        }
                        else
                        {
                            // select a module
                            DialogChooseModule *diag = new DialogChooseModule(&moduleName);
                            QStringList listModuleName;
                            foreach (MODULE* module, listModule)
                            {
                                listModuleName.append(module->name);
                            }
                            diag->setList(listModuleName);
                            int ret = diag->exec();

                            if (ret == QDialog::Accepted)
                            {
                                writeOutput("action open new CSV : module " + moduleName + " selected.");
                            }
                            else
                            {
                                writeOutput("action open new CSV : no module chosen !");
                                return;
                            }
                        }

                        Csv *csv = new Csv(fullCsvName, wp, moduleName);

                        // display status bar
                        statusBar()->show();
                        progBar->reset();
                        connect(csv, SIGNAL(incProgressBar(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                        if (csv->readFile())
                        {
                            //add csv to the workProject
                            wp->addCsv(csv);
                            list.append(csv);

                            //stop timer
                            double tf = omp_get_wtime();

                            //update the treeView model
                            ui->treeView->expand(index);
                            ui->treeView->resizeColumnToContents(0);

                            writeOutput("CSV file " + fullCsvName + " successfully added to the project " + QString::number(tf-ti) + " sec");

                        }
                        else
                        {
                            //remove csv from the workProject
                             wp->removeCsv(csv);
                             writeOutput("CSV file " + fullCsvName + "  NOT added to project due to wrong format ");
                        }

                        // hide the statusbar
                        statusBar()->hide();
                        progBar->reset();

                    }
                }
            }
        }
    }
}

void MDImain::addCdfxFile2Project()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::add cdfx file to project", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a project");
            writeOutput("action open new cdfx file cancelled: no project first selected");
            return;
        }
        else
        {
            QSettings settings;
            QString path = settings.value("currentCdfxPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
                                              tr("select CDFX files"), path,
                                              tr("CDFX files (*.cdfx);;all files (*.*)"));

            if (files.isEmpty())
            {
               writeOutput("action open new CDFX file : no CDFX file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(index);

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                if (wp)  //to prevent any chrash of the aplication
                {
                    // if no MOD_COMMON in ASAP file
                    if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                    {
                        QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                        writeOutput("action open new CDFX file : no MOD_COMMON in ASAP file");
                        return;
                    }

                    // check if Csv already in project
                    foreach (QString fullCdfxName, files)
                    {
                        //if the selected Hex file is already into the project => exit
                        if (wp->cdfxFiles().contains(fullCdfxName))
                        {
                            QMessageBox::information(this, "HEXplorer", tr("file already in project"));
                            writeOutput("action open new CDFX file : CDFX file already in project");
                            files.removeOne(fullCdfxName);
                            return;
                        }
                    }

                    // Read CDFX files
                    QList<CdfxFile*> list;
                    foreach (QString fullCdfxName, files)
                    {
                        //update currentHexPath
                        QSettings settings;
                        QString currentCdfxPath = QFileInfo(fullCdfxName).absolutePath();
                        settings.setValue("currentCdfxPath", currentCdfxPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project
                        QString moduleName;
                        QList<MODULE*> listModule = wp->a2lFile->getProject()->listModule();
                        if (listModule.count() == 0)
                        {
                            writeOutput("action open new dataset : no Module into A2l file !");
                            return;
                        }
                        else if (listModule.count() == 1)
                        {
                            moduleName = listModule.at(0)->name;
                        }
                        else
                        {
                            // select a module
                            DialogChooseModule *diag = new DialogChooseModule(&moduleName);
                            QStringList listModuleName;
                            foreach (MODULE* module, listModule)
                            {
                                listModuleName.append(module->name);
                            }
                            diag->setList(listModuleName);
                            int ret = diag->exec();

                            if (ret == QDialog::Accepted)
                            {
                                writeOutput("action open new Cdfx : module " + moduleName + " selected.");
                            }
                            else
                            {
                                writeOutput("action open new Cdfx : no module chosen !");
                                return;
                            }
                        }


                        CdfxFile *cdfx = new CdfxFile(fullCdfxName, wp, moduleName);
                        if (cdfx->isRead)
                        {
                            //add csv to the workProject
                            wp->addCdfx(cdfx);
                            list.append(cdfx);

                            //stop timer
                            double tf = omp_get_wtime();

                            //update the treeView model
                            ui->treeView->expand(index);
                            ui->treeView->resizeColumnToContents(0);

                            writeOutput(cdfx->getInfoList());
                            writeOutput("CDFX file " + fullCdfxName + " successfully added to the project " + QString::number(tf-ti) + " sec");

                        }
                        else
                        {
                             // display error
                             writeOutput(cdfx->getErrorList());
                             writeOutput("CDFX file " + fullCdfxName + "  NOT added to project due to wrong format ");

                             //remove cdfx from the workProject
                             wp->removeCdfxFile(cdfx);
                        }
                    }
                }
            }
        }
    }
}

void MDImain::deleteFilesFromProject()
{
    int n = ui->treeView->selectionModel()->selectedIndexes().count();

    if (n == 1)
    {
        QModelIndex index = ui->treeView->selectionModel()->selectedIndexes().at(0);

        if (index.isValid())
        {
            //get a pointer on the selected item
            Node *node =  model->getNode(index);
            QString name = typeid(*node).name();

            if (name.endsWith("HexFile") || name.endsWith("Csv") || name.endsWith("CdfxFile"))
            {
                int r = QMessageBox::question(this, "HEXplorer::question",
                                              "Remove : " + QString(node->name) +
                                              "\nfrom the project : " + QString(node->getParentNode()->name) + " ?",
                                               QMessageBox::Yes, QMessageBox::No);

                if (r ==  QMessageBox::Yes)
                {
                    deleteFileFromProject(index);
                }
            }
            else
            {
                QMessageBox::warning(this, "HEXplorer::remove file from project", "Please select an hex, csv or cdfx file.",
                                                 QMessageBox::Ok);
                return;
            }
        }
    }
    else
    {
        int r = QMessageBox::question(this, "HEXplorer::question",
                                      "Remove all the selected files ?",
                                       QMessageBox::Yes, QMessageBox::No);

        if (r ==  QMessageBox::Yes)
        {
            foreach (QModelIndex index, ui->treeView->selectionModel()->selectedIndexes())
            {

                deleteFileFromProject(index);
            }
        }

    }

}

void MDImain::deleteFileFromProject(QModelIndex index)
{
    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QModelIndex indexParent = index.parent();
        QString name = typeid(*node).name();

        if (name.endsWith("HexFile"))
        {
            //ask for save changes
            HexFile *hex = dynamic_cast<HexFile *> (node);
            if (hex->childNodes.count() != 0)
            {
                // ask for saving changes
                int r = QMessageBox::question(this, "HEXplorer::question", tr("Save changes ?"),
                                      QMessageBox::Yes, QMessageBox::No);

                if (r ==  QMessageBox::Yes)
                {
                    if (!save_HexFile(index))
                    {
                        return;
                    }
                }

                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (hex->getParentNode());
                a2l->removeChildNode(hex);

                //get the parentWp of the HexFile and delete Hex
                WorkProject *wp = hex->getParentWp();
                wp->removeHexFile(hex); // delete hex included

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }
            else
            {
                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (hex->getParentNode());
                a2l->removeChildNode(hex);

                //get the parentWp of the HexFile
                WorkProject *wp = hex->getParentWp();
                wp->removeHexFile(hex);

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }

            //hide toolbar hex
            ui->toolBar_data->hide();
        }
        else if (name.endsWith("Csv"))
        {
            //ask for save changes
            Csv *csv = dynamic_cast<Csv *> (node);
            if (!csv->getModifiedData().isEmpty())
            {
                // ask for saving changes
                int r = QMessageBox::question(this, "HEXplorer::question", tr("Save changes ?"),
                                      QMessageBox::Yes, QMessageBox::No);

                if (r ==  QMessageBox::Yes)
                {
                    if (!save_CsvFile(index))
                    {
                        return;
                    }
                }

                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (csv->getParentNode());
                a2l->removeChildNode(csv);

                //get the parentWp of the HexFile
                WorkProject *wp = csv->getParentWp();
                wp->removeCsv(csv);

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }
            else
            {
                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (csv->getParentNode());
                a2l->removeChildNode(csv);

                //get the parentWp of the HexFile
                WorkProject *wp = csv->getParentWp();
                wp->removeCsv(csv);

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }

            //hide toolbar hex
            ui->toolBar_data->hide();
        }
        else if (name.endsWith("CdfxFile"))
        {
            //ask for save changes
            CdfxFile *cdfx = dynamic_cast<CdfxFile *> (node);
            if (!cdfx->getModifiedData().isEmpty())
            {
                // ask for saving changes
                int r = QMessageBox::question(this, "HEXplorer::question", tr("Save changes ?"),
                                      QMessageBox::Yes, QMessageBox::No);

                if (r ==  QMessageBox::Yes)
                {
                    if (!save_CdfxFile(index))
                    {
                        return;
                    }
                }

                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (cdfx->getParentNode());
                a2l->removeChildNode(cdfx);

                //get the parentWp of the HexFile
                WorkProject *wp = cdfx->getParentWp();
                wp->removeCdfxFile(cdfx);

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }
            else
            {
                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (cdfx->getParentNode());
                a2l->removeChildNode(cdfx);

                //get the parentWp of the HexFile
                WorkProject *wp = cdfx->getParentWp();
                wp->removeCdfxFile(cdfx);

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }

            //hide toolbar hex
            ui->toolBar_data->hide();
        }
        else
        {
            QMessageBox::warning(this, "HEXplorer::remove file from project", "Please select first an hex or csv file.",
                                             QMessageBox::Ok);
            return;
        }
    }

}

void MDImain::reAppendProject(WorkProject *wp)
{
    wp->attach(this);

    //insert the new created project into the projectList
    projectList->insert(wp->getFullA2lFileName().c_str(), wp);

    //update the ui->treeView
    model->addNode2RootNode(wp->a2lFile);
    ui->treeView->setModel(model);
    ui->treeView->setColumnHidden(1, true);
}

void MDImain::showFixPar()
{
    //search the index of the current node in the Model
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //create a dialog window
    Dialog *diag = new Dialog();

    QMap<std::string, std::string> *param = model->getPar(index);


        QMap<std::string, std::string>::const_iterator i = param->constBegin();
        while (i != param->constEnd())
        {
                diag->addItem(QString((i.key()).c_str()) + " : " + QString((i.value()).c_str()));
                ++i;
        }

        diag->setWindowTitle(QString((param->value("Name")).c_str()));

        delete param;

    diag->show();
}

void MDImain::countChild()
{
    //get the index of the selected hex file
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    QString str;
    str.setNum(model->rowCount(index));

    ui->listWidget->addItem(str + " nodes");
}

void MDImain::editTextFile()
{
    // check if a file is selected in treeWidget
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a file to be edited");
            writeOutput("action edit  file cancelled: no project first selected");
            return;
        }
        else
        {
            if (name.endsWith("HexFile") || name.endsWith("Csv") ||
                name.endsWith("CdfxFile") || name.endsWith("A2LFILE"))
            {
                //Get the selected file name
                QString fullFileName = model->name(index);
                QFile file(fullFileName);

                //update output
                QsciLexer *lexer = 0;
                if (name.endsWith("CdfxFile"))
                {
                    lexer = new QsciLexerXML();
                }

                if (editTextFile(file, lexer))
                    writeOutput("action edit File : file " + fullFileName + " edit with success.");
                else
                    writeOutput("action edit File : file " + fullFileName + " cannot be edited.");
            }
            else
            {
                QMessageBox::warning(this, "HEXplorer::edit file", "Please select first an hex, csv or Cdfx file.",
                                                 QMessageBox::Ok);
                return;
            }
        }
    }
}

bool MDImain::editTextFile(QFile &file, QsciLexer *lexer)
{
       //open and read the file
       if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       {
          QMessageBox::warning(this, tr("HEXplorer"),
                                tr("Cannot read file %1 :\n%2.")
                                .arg(file.fileName())
                                .arg(file.errorString()));

          return false;
       }

       //get the name of the file
       QString fileName = file.fileName();

       //intialize tabList if necessary
       if (tabList == NULL)
       {
            tabList = new QMap<QString, QWidget*>;
        }

       //check if the file is already open in a Tab
       if (tabList->contains(fileName))
           QMessageBox::warning(this, tr("information"), tr("file already open"));
       else
       {
           //create the textEditor (Qscintilla)
           ChTextEdit *textEdit = new ChTextEdit;
           textEdit->loadFile(fileName);

           //set the lexer if necessary
           if (lexer)
           {
               textEdit->setLexer(lexer);
           }

           //add the Qscintilla editor to listTab
           tabList->insert(fileName, textEdit);

           //draw the tab in tabWidget
           QIcon icon;
           if (fileName.endsWith(".hex") || fileName.endsWith(".HEX"))
               icon.addFile(":/icones/ram.png");
           else if (fileName.endsWith(".a2l") || fileName.endsWith(".A2L"))
               icon.addFile(":/icones/milky_cartable.png");
           else if (fileName.endsWith(".js"))
               icon.addFile("");

           ui->tabWidget->addTab(tabList->value(fileName), icon, fileName);

           //set new FormCompare as activated
           ui->tabWidget->setCurrentWidget(tabList->value(fileName));
        }

       return true;
   }

bool MDImain::editTextFile(QString &text)
{

       //create the textEditor (Qscintilla)
       ChTextEdit *textEdit = new ChTextEdit;
       QFont font;
       font.setFixedPitch(true);
       textEdit->setFont(font);
       textEdit->setText(text);

       //add the Qscintilla editor to listTab
       tabList->insert("hex", textEdit);

       //draw the tab in tabWidget
       ui->tabWidget->addTab(tabList->value("hex"), "hex");

       return true;
}

void MDImain::removeWorkProjects()
{
    int n = ui->treeView->selectionModel()->selectedIndexes().count();

    if (n == 1)
    {
        QModelIndex index = ui->treeView->selectionModel()->selectedIndexes().at(0);

        if (index.isValid())
        {
            //get a pointer on the selected item
            Node *node =  model->getNode(index);
            QString name = typeid(*node).name();

            if (!name.endsWith("A2LFILE"))
            {
                QMessageBox::warning(this, "HEXplorer::remove project", "Please select first a project.",
                                                 QMessageBox::Ok);
                return;
            }


            int r = QMessageBox::question(this, "HEXplorer::question", "Are you sure you want to delete the project " +
                                          QString(node->name)  + " ?",
                                  QMessageBox::Yes, QMessageBox::No);

            if (r ==  QMessageBox::Yes)
            {
                    removeWorkProject(index);
            }
        }
    }
    else
    {
        int r = QMessageBox::question(this, "HEXplorer::question",
                                      "Remove all the selected projects ?",
                                       QMessageBox::Yes, QMessageBox::No);

        if (r ==  QMessageBox::Yes)
        {
            foreach (QModelIndex index, ui->treeView->selectionModel()->selectedIndexes())
            {

                removeWorkProject(index);
            }
        }
    }
}

void MDImain::removeWorkProject(QModelIndex index)
{

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::remove project", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        //As the selected node is an A2l file we can cast the node into its real type : A2LFILE
        A2LFILE *a2lfile = dynamic_cast<A2LFILE *> (node);

        //ask to save the modified nodes (hex or Csv)
        foreach (Node *node, a2lfile->childNodes)
        {
            QString name = typeid(*node).name();
            if (name.endsWith("HexFile"))
            {
                HexFile *hex = (HexFile*)node;
                if (!hex->getModifiedData().isEmpty())
                {
                      int r = QMessageBox::question(this, "HEXplorer::question",
                                                    "Save changes in " + QString(hex->name) + "?",
                                                    QMessageBox::Yes, QMessageBox::No);
                      if (r == QMessageBox::Yes)
                      {
                          QModelIndex hexIndex = model->getIndex(hex);
                          save_HexFile(hexIndex);
                      }
                }
            }
            else if (name.endsWith("Csv"))
            {
                Csv *csv = (Csv*)node;
                if (!csv->getModifiedData().isEmpty())
                {
                    int r = QMessageBox::question(this, "HEXplorer::question",
                                                  "Save changes in " + QString(csv->name) + "?",
                                                  QMessageBox::Yes, QMessageBox::No);
                    if (r == QMessageBox::Yes)
                    {
                      QModelIndex csvIndex = model->getIndex(csv);
                      save_CsvFile(csvIndex);
                    }
                }
            }
        }

        //remove the node from treeView model
        model->removeChildNode(node);

        //update the treeView
        model->update();
        ui->treeView->resizeColumnToContents(0);

        //get the project
        WorkProject *wp = projectList->value(a2lfile->fullName());

        //remove the project from the this->projectList
        projectList->remove(a2lfile->fullName());

        //delete the selected project
        //delete wp;
        wp->detach(this);
    }
}

void MDImain::export_ListData()
{
    QStringList strList;
    strList.append("[Label]");

    QStringList listData = ((A2lTreeModel*)ui->treeView_2->model())->getListDataName();
    strList.append(listData);

    QSettings settings;
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
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach (QString str, strList)
        out << str << "\r\n";
    QApplication::restoreOverrideCursor();
}

void MDImain::editMeasuringChannels()
{
    // check if a file is selected in treeWidget
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::edit measuring channels", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a file to be edited");
            writeOutput("action edit  file cancelled: no project first selected");
            return;
        }
        else
        {
            //Get the selected file name
            QString fullFileName = model->name(index);

            //create a pointer on the WorkProject
            WorkProject *wp = projectList->value(fullFileName);

            // get the list of MEASUREMET
            QList<Node*> list;
            Node *module = wp->a2lFile->getProject()->getNode("MODULE");
            if (module == NULL)
            {
                return;
            }
            else
            {
                Node *dim = module->getNode("DIM");
                if (dim == NULL)
                {
                    return;
                }
                else
                {
                    Node *measurment = dim->getNode("MEASUREMENT");
                    if (measurment)
                    {
                        list = measurment->childNodes;
                    }
                    else
                    {
                        return;
                    }
                }
            }

            //display the measuring channels in view
            MeasModel *measModel = new MeasModel();
            measModel->setList(list);

            //create a new spreadSheet
            SpreadsheetView *view = new SpreadsheetView();
            view->setModel(measModel);

//            FreezeTableWidget *view = new FreezeTableWidget(measModel);
            view->setAlternatingRowColors(true);

            //add a new tab with the spreadsheet
            QIcon icon;
            icon.addFile(":/icones/milky_peigne.png");
            ui->tabWidget->addTab(view, icon, fullFileName);

            //set new FormCompare as activated
            ui->tabWidget->setCurrentWidget(view);
        }

        //write output
        writeOutput("measurment labels edited.");

    }
}

void MDImain::editChar()
{
    // check if a file is selected in treeWidget
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("A2LFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::edit measuring channels", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a file to be edited");
            writeOutput("action edit  file cancelled: no project first selected");
            return;
        }
        else
        {
            //Get the selected file name
            QString fullFileName = model->name(index);

            //create a pointer on the WorkProject
            WorkProject *wp = projectList->value(fullFileName);

            // get the list of MEASUREMET
            QList<Node*> list;
            Node *module = wp->a2lFile->getProject()->getNode("MODULE");
            if (module == NULL)
            {
                return;
            }
            else
            {
                Node *dim = module->getNode("DIM");
                if (dim == NULL)
                {
                    return;
                }
                else
                {
                    Node *charact = dim->getNode("CHARACTERISTIC");
                    if (charact)
                    {
                        list = charact->childNodes;
                    }
                    else
                    {
                        return;
                    }
                }
            }

            //display the measuring channels in view
            CharModel *charModel = new CharModel();
            charModel->setList(list);

            //create a new spreadSheet
            SpreadsheetView *view = new SpreadsheetView();
            view->setModel(charModel);

//            FreezeTableWidget *view = new FreezeTableWidget(charModel);
            view->setAlternatingRowColors(true);

            //add a new tab with the spreadsheet
            QIcon icon;
            icon.addFile(":/icones/milky_peigne.png");
            ui->tabWidget->addTab(view, icon, fullFileName);

            //set new FormCompare as activated
            ui->tabWidget->setCurrentWidget(view);
        }

        //write output
        writeOutput("measurment labels edited.");

    }
}

void MDImain::newFormScript()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //Get the project name
    QString fullA2lName = model->name(index);

    //create a pointer on the WorkProject
    WorkProject *wp = projectList->value(fullA2lName);

    FormScript *form = new FormScript(wp, this);
    form->show();
}

void MDImain::compare_A2lFile()
{
    //get a2lFiles path
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();

    if (list.count() == 2)
    {
        QString moduleName = "DIM";

        // get a2l file path
        QString str1 = model->name(list.at(0));
        QString str2 = model->name(list.at(1));


        // get listChar1
        WorkProject *wp1 = projectList->value(str1);
        QStringList list1;
        if (wp1)  //to prevent any crash of the aplication
        {
            // if MODULE in ASAP file
            if (wp1->a2lFile->getProject()->getNode("MODULE") != NULL)
            {
                MODULE *mod = (MODULE*)wp1->a2lFile->getProject()->getNode("MODULE/" + moduleName);
                list1 = mod->listChar;
            }
        }

        // get CHAR listChar2
        WorkProject *wp2 = projectList->value(str2);
        QStringList list2;
        if (wp2)  //to prevent any crash of the aplication
        {
            // if MODULE in ASAP file
            if (wp2->a2lFile->getProject()->getNode("MODULE") != NULL)
            {
                list2 = ((MODULE*)wp2->a2lFile->getProject()->getNode("MODULE/" + moduleName))->listChar;
            }
        }

        //Missing labels
        QStringList missingLabels;
        foreach (QString str, list1)
        {
            QStringList::iterator i = qBinaryFind(list2.begin(), list2.end(), str);
            if (i == list2.end())
            {
                missingLabels.append(str);
            }
        }

        //New labels
        QStringList newLabels;
        foreach (QString str, list2)
        {
            QStringList::iterator i = qBinaryFind(list1.begin(), list1.end(), str);
            if (i == list1.end())
            {
                newLabels.append(str);
            }
        }

        //Missing, new, modified subsets
        Node *fun1 = wp1->a2lFile->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");
        Node *fun2 = wp2->a2lFile->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");
        QStringList missingSubsets;
        QStringList newSubsets;
        QStringList modifiedSubsets;
        if (fun1 && fun2)
        {
            QStringList listSubset1;
            foreach (Node *subset, fun1->childNodes)
            {
                listSubset1.append(subset->name);
            }
            listSubset1.sort();
            QStringList listSubset2;
            foreach (Node *subset, fun2->childNodes)
            {
                listSubset2.append(subset->name);

            }
            listSubset2.sort();

            //missing subsets
            foreach (QString str, listSubset1)
            {
                if (!listSubset2.contains(str))
                    missingSubsets.append(str);
            }

            //new subsets
            foreach (QString str, listSubset2)
            {
                if (!listSubset1.contains(str))
                    newSubsets.append(str);
            }

            //modified subsets
            foreach (Node *subset1, fun1->childNodes)
            {
                //get the label list from subset1
                DEF_CHARACTERISTIC *node_char1 = (DEF_CHARACTERISTIC*)subset1->getNode("DEF_CHARACTERISTIC");
                if (node_char1)
                {
                    //get ListChar_subset1
                    QStringList listChar_subset1 = node_char1->getCharList();

                    //get Node into listSubsets2
                    Node *subset2 = fun2->getNode(subset1->name);

                    //get the label list from subset2
                    if (subset2)
                    {
                        DEF_CHARACTERISTIC *node_char2 = (DEF_CHARACTERISTIC*)subset2->getNode("DEF_CHARACTERISTIC");

                        if (node_char2)
                        {
                            //get ListChar_subset2
                            QStringList listChar_subset2 = node_char2->getCharList();

                            //compare listChar_Subset
                            bool same = true;
                            foreach (QString str, listChar_subset1)
                            {
                                if (!listChar_subset2.contains(str))
                                    same = false;
                            }

                            if (same)
                            {
                                foreach (QString str, listChar_subset2)
                                {
                                    if (!listChar_subset1.contains(str))
                                        same = false;
                                }
                            }

                            if (!same)
                            {
                                modifiedSubsets.append(subset1->name);
                            }


                        }

                    }
                }

            }

        }

        //create a string to load into QScintilla
        QString text;

        //summary
        text.append("--------------- Labels ----------------\n");
        text.append(QString::number(missingLabels.count()) + " missing labels into " + str2 + "\n");
        text.append(QString::number(newLabels.count()) + " new labels into " + str2 + "\n");
        text.append("-------------- Subsets ----------------\n");
        text.append(QString::number(missingSubsets.count()) + " missing subsets into " + str2 + "\n");
        text.append(QString::number(newSubsets.count()) + " new subsets into " + str2 + "\n");
        text.append(QString::number(modifiedSubsets.count()) + " modified subsets\n");

        //missing labels
        text.append("\n");
        text.append(QString::number(missingLabels.count()) + " missing labels into " + str2 + " : \n");
        foreach (QString str, missingLabels)
        {
            text.append("\t" + str + "\n");
        }

        //new labels
        text.append("\n");
        text.append(QString::number(newLabels.count()) + " new labels into " + str2 + " : \n");
        foreach (QString str, newLabels)
        {
            text.append("\t" + str + "\n");
        }

        //missing subsets
        text.append("\n");
        text.append(QString::number(missingSubsets.count()) + " missing subsets into " + str2 + " : \n");
        foreach (QString str, missingSubsets)
        {
            text.append("\t" + str + "\n");
        }

        //new subsets
        text.append("\n");
        text.append(QString::number(newSubsets.count()) + " new subsets into " + str2 + " : \n");
        foreach (QString str, newSubsets)
        {
            text.append("\t" + str + "\n");
        }

        //modified subsets
        text.append("\n");
        text.append(QString::number(modifiedSubsets.count()) + " modified subsets\n");
        foreach (QString str, modifiedSubsets)
        {
            text.append("\t" + str + "\n");
        }

        //create the textEditor (Qscintilla)
        ChTextEdit *textEdit = new ChTextEdit;
        textEdit->setText(text);

        //add the Qscintilla editor to listTab
        tabList->insert("fileName", textEdit);

        //draw the tab in tabWidget
        QIcon icon;
        icon.addFile(":/icones/milky_cartable.png");

        ui->tabWidget->addTab(tabList->value("fileName"), icon, "compare A2L");

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(tabList->value("fileName"));



    }
}

// ----------------- Data containers -----------------------//

void MDImain::clone_HexFile()
{
    //get the index of the selected hex file
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);

    //As the selected node is an Hex file we can cast the node into its real type : HexFile
    HexFile *hex = dynamic_cast<HexFile *> (node);

    //get the parentWp of the HexFile
    WorkProject *wp = hex->getParentWp();

    //add to the project
     //HexFile *clone = new HexFile(hex->fullName() + "_copy", wp);
     //wp->addHex(clone);

     //update the treeView model
     model->update();
     A2lTreeModel *mod = (A2lTreeModel*)ui->treeView->model();
     index = mod->getIndex(wp->a2lFile);
     ui->treeView->expand(index);
     ui->treeView->resizeColumnToContents(0);

     writeOutput("action clone dataset : performed with success");
}

void MDImain::sort_BySubset()
{
    //get the index of the selected hex file
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);    

    QString name = typeid(*node).name();
    if (name.endsWith("HexFile"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        HexFile *hex = dynamic_cast<HexFile *> (node);

        //call sort by subset
        if (hex->isSortedBySubsets())
            hex->sortModifiedDataBySubset(false);
        else
            hex->sortModifiedDataBySubset(true);
    }
    else if (name.endsWith("Csv"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        Csv *csv = dynamic_cast<Csv *> (node);

        //call sort by subset
        if (csv->isSortedBySubsets())
            csv->sortModifiedDataBySubset(false);
        else
            csv->sortModifiedDataBySubset(true);
    }
    else if (name.endsWith("CdfxFile"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        CdfxFile *cdfx = dynamic_cast<CdfxFile *> (node);

        //call sort by subset
        if (cdfx->isSortedBySubsets())
            cdfx->sortModifiedDataBySubset(false);
        else
            cdfx->sortModifiedDataBySubset(true);
    }
    else
    {
        QMessageBox::warning(this, "HEXplorer::sort by subset", "Please select first an hex, csv or Cdfx file.",
                                         QMessageBox::Ok);
        return;
    }
}

void MDImain::reset_Data()
{
    QModelIndexList listIndex = ui->treeView->selectionModel()->selectedIndexes();

    foreach (QModelIndex index, listIndex)
    {
        Data *data = (Data*)model->getNode(index);
        data->resetAll();
        updateView();
    }
}

void MDImain::read_ValuesFromCsv()
{       
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("HexFile"))
        {
            QMessageBox::warning(this, "HEXplorer::import csv into hex file", "Please select first an hex file.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select a project");
            writeOutput("action open new dataset cancelled: no project first selected");
            return;
        }
        else
        {
            HexFile *hex  = (HexFile*)model->getNode(index);

            QSettings settings;
            QString path = settings.value("currentCsvPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
                                              tr("select CSV files"), path,
                                              tr("CSV files (*.csv);;all files (*.*)"));

            if (files.isEmpty())
            {
               writeOutput("action open new CSV file : no CSV file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(model->parent(index));

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                // if no MOD_COMMON in ASAP file
                if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                {
                    QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                    writeOutput("action open new dataset : no MOD_COMMON in ASAP file");
                    return;
                }

                // check if Csv already in project
                foreach (QString fullCsvName, files)
                {
                    //if the selected Hex file is already into the project => exit
                    if (wp->csvFiles().contains(fullCsvName))
                    {
                        QMessageBox::information(this, "HEXplorer", tr("file already in project"));
                        writeOutput("action open new CSV file : CSV file already in project");
                        files.removeOne(fullCsvName);
                        return;
                    }
                }

                // Read CSV files
                QList<Csv*> list;
                foreach (QString fullCsvName, files)
                {
                    //update currentHexPath
                    QSettings settings;
                    QString currentCsvPath = QFileInfo(fullCsvName).absolutePath();
                    settings.setValue("currentCsvPath", currentCsvPath);

                    //start a timer
                    double ti = omp_get_wtime();

                    //add the file to the project
                    Csv *csv = new Csv(fullCsvName, wp, hex->getModuleName());

                    // display status bar
                    statusBar()->show();
                    progBar->reset();
                    connect(csv, SIGNAL(incProgressBar(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                    if (csv->readFile())
                    {
                        list.append(csv);

                        //stop timer
                        double tf = omp_get_wtime();

                        writeOutput("action open new CSV file : CSV file add to project in " + QString::number(tf-ti) + " sec");
                    }
                    else
                    {
                         writeOutput("action open new CSV file : CSV file NOT added to project due to wrong format ");
                    }

                    // hide the statusbar
                    statusBar()->hide();
                    progBar->reset();
                }

                // display CSV files
                if (list.isEmpty())
                {
                    return;
                }
                else
                {
                    QStringList listLabel;
                    A2lTreeModel *mod = new A2lTreeModel();
                    DialogCsv *w = new DialogCsv(this, &listLabel);

                    foreach (Csv *csv, list)
                    {
                        csv->addLabelsAsChild();
                        mod->addNode2RootNode(csv);
                    }
                    w->setModel(mod);
                    w->exec();

                    // copy the selected data
                    Node *root = mod->getRootNode();
                    foreach (QString str, listLabel)
                    {
                        Data *data1 = (Data*)root->getNode(str);
                        if (data1)
                        {
                            // get the data from destination container
                            Data* data2 = hex->getData(data1->name);

                            // if destination data exists perform copy
                            if (data2)
                            {
                                data2->copyAllFrom(data1);
                            }
                        }
                    }

                    //delete csv files
                    delete mod;
                }
            }
        }
    }
}

void MDImain::read_ValuesFromCdfx()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("HexFile"))
        {
            QMessageBox::warning(this, "HEXplorer::import csv into hex file", "Please select first an hex file.",
                                             QMessageBox::Ok);
            return;
        }

        int row = index.row();
        if ( row < 0)
        {
            QMessageBox::information(this,"HEXplorer","please first select an Hex file");
            writeOutput("action read CDFX file cancelled: no Hex file selected");
            return;
        }
        else
        {
            HexFile *hex  = (HexFile*)model->getNode(index);

            QSettings settings;
            QString path = settings.value("currentCdfxPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
                                              tr("select CDFX files"), path,
                                              tr("CDFX files (*.cdfx);;all files (*.*)"));

            if (files.isEmpty())
            {
               writeOutput("action read CDFX values : no CDFX file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(model->parent(index));

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                // if no MOD_COMMON in ASAP file
                if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                {
                    QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                    writeOutput("action open new dataset : no MOD_COMMON in ASAP file");
                    return;
                }

                // check if Cdfx already in project
                foreach (QString fullCdfxName, files)
                {
                    //if the selected Hex file is already into the project => exit
                    if (wp->csvFiles().contains(fullCdfxName))
                    {
                        QMessageBox::information(this, "HEXplorer", tr("file already in project"));
                        writeOutput("action read CDFX values : CDFX file already in project");
                        files.removeOne(fullCdfxName);
                        return;
                    }
                }

                // Read CSV files
                QList<CdfxFile*> list;
                foreach (QString fullCdfxName, files)
                {
                    //update currentHexPath
                    QSettings settings;
                    QString currentCdfxPath = QFileInfo(fullCdfxName).absolutePath();
                    settings.setValue("currentCdfxPath", currentCdfxPath);

                    //start a timer
                    double ti = omp_get_wtime();

                    //add the file to the project
                    CdfxFile *cdfx = new CdfxFile(fullCdfxName, wp, hex->getModuleName());
                    if (cdfx->isRead)
                    {
                        list.append(cdfx);

                        //stop timer
                        double tf = omp_get_wtime();

                        writeOutput("read CDFX values : CDFX file read in " + QString::number(tf-ti) + " sec");
                    }
                    else
                    {
                         writeOutput("read CDFX values : CDFX file NOT read due to wrong format ");
                    }

                }

                // display CSV files
                if (list.isEmpty())
                {
                    return;
                }
                else
                {
                    QStringList listLabel;
                    A2lTreeModel *mod = new A2lTreeModel();
                    DialogCsv *w = new DialogCsv(this, &listLabel);

                    foreach (CdfxFile *cdfx, list)
                    {
                        cdfx->addLabelsAsChild();
                        mod->addNode2RootNode(cdfx);
                    }
                    w->setModel(mod);
                    w->exec();

                    // copy the selected data
                    Node *root = mod->getRootNode();
                    foreach (QString str, listLabel)
                    {
                        Data *data1 = (Data*)root->getNode(str);
                        if (data1)
                        {
                            // get the data into destination container
                            Data* data2 = hex->getData(data1->name);

                            // if destination data exists perform copy
                            if (data2)
                            {
                                data2->copyAllFrom(data1);
                            }
                        }
                    }

                    //delete cdfx files
                    delete mod;
                }
            }
        }
    }
}

void MDImain::reset_AllChangedData()
{
    //get the index of the selected hex file
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);

    QString name = typeid(*node).name();
    if (name.endsWith("HexFile"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        HexFile *hex = dynamic_cast<HexFile *> (node);

        //reset all labels;
        hex->resetAllModifiedData();
    }
    else if (name.endsWith("Csv"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        Csv *csv = dynamic_cast<Csv *> (node);

        //reset all labels;
        csv->resetAllModifiedData();
    }
    else if (name.endsWith("CdfxFile"))
    {
        //As the selected node is an Hex file we can cast the node into its real type : HexFile
        CdfxFile *cdfx = dynamic_cast<CdfxFile *> (node);

        //reset all labels;
        cdfx->resetAllModifiedData();
    }
    else
    {
        QMessageBox::warning(this, "HEXplorer::reset all changed data", "Please select first an hex or csv file.",
                                         QMessageBox::Ok);
        return;
    }

    //update all the table views
    updateView();
}

bool MDImain::save_File()
{
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);

    QString name = typeid(*node).name();
    if (name.endsWith("HexFile"))
    {
        return save_HexFile(index);
    }
    else if (name.endsWith("Csv"))
    {
        return save_CsvFile(index);
    }
    else if (name.endsWith("CdfxFile"))
    {
        return save_CdfxFile(index);
    }
    else
    {
        QMessageBox::warning(this, "HEXplorer::save file", "Please select first an hex,csv or cdfx file.",
                                         QMessageBox::Ok);
        return false;
    }
}

bool MDImain::save_CsvFile(QModelIndex index)
{

    Csv *csv = (Csv*)model->getNode(index);

    int ret = QMessageBox::question(0, "HEXplorer :: save Csv file",
                          "overwrite " + csv->fullName() + " ?", QMessageBox::Yes, QMessageBox::Cancel);

    if (ret == QMessageBox::Yes)
    {
        return csv->save();
    }
    else
    {
        return false;
    }
}

bool MDImain::save_CdfxFile(QModelIndex index)
{

    CdfxFile *cdfx = (CdfxFile*)model->getNode(index);

    int ret = QMessageBox::question(0, "HEXplorer :: save Cdfx file",
                          "overwrite " + cdfx->fullName() + " ?", QMessageBox::Yes, QMessageBox::Cancel);

    if (ret == QMessageBox::Yes)
    {
        return cdfx->save();
    }
    else
    {
        return false;
    }
}

bool MDImain::save_HexFile(QModelIndex index)
{
    double ti = omp_get_wtime();

    HexFile *hex = (HexFile*)model->getNode(index);

    int ret = QMessageBox::question(0, "HEXplorer :: save Hex file",
                          "overwrite " + hex->fullName() + " ?", QMessageBox::Yes, QMessageBox::Cancel);

    if (ret == QMessageBox::Yes)
    {
        // display status bar
        statusBar()->show();
        progBar->reset();

        // write all
        QStringList list = hex->writeHex();

        if (list.isEmpty())
        {
            writeOutput("action save dataset : cancelled");
            return false;
        }

        QString fileName = hex->fullName();
        if (fileName.isEmpty())
            return false;

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly))
        {
            QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);        

        // write into file
        QTextStream out(&file);
        int i = progBar->value();
        int max = progBar->maximum();
        foreach (QString str, list)
        {            
            out << str << "\r\n";
            i++;
            setValueProgressBar(i, max);
        }

        // hide the statusbar
        statusBar()->hide();
        progBar->reset();

        QApplication::restoreOverrideCursor();

        double tf = omp_get_wtime();
        writeOutput("action save dataset : performed with success in " + QString::number(tf - ti) + " s");

        file.close();
        return true;
    }
    else
    {
        writeOutput("action save dataset : cancelled");
        return false;
    }


}

void MDImain::saveAs_File()
{
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);

    QString name = typeid(*node).name();
    if (name.endsWith("HexFile"))
    {
        return saveAs_HexFile(index);
    }
    else if (name.endsWith("Csv"))
    {
        return saveAs_CsvFile(index);
    }
    else if (name.endsWith("CdfxFile"))
    {
        return saveAs_CdfxFile(index);
    }
    else
    {
        QMessageBox::warning(this, "HEXplorer::save as file", "Please select first an hex or csv file.",
                                         QMessageBox::Ok);
        return;
    }
}

void MDImain::saveAs_HexFile(QModelIndex index)
{
    // get the HexFile Node
    HexFile *hex = (HexFile*)model->getNode(index);

    // ask for new HexFile name
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QFileInfo(hex->fullName()).fileName(),
                                                    QFileInfo(hex->fullName()).absolutePath(),
                                                    tr("hex files (*.hex);;all files (*.*)"));
    if (fileName.isEmpty())
    {
        writeOutput("action save dataset : cancelled");
        return;
    }
    if ( QFileInfo(fileName).suffix().toLower() != "hex" &&
         QFileInfo(fileName).suffix().toLower() != "hex_trimmed")
    {
        fileName.append(".hex");
    }

    // display status bar
    statusBar()->show();
    progBar->reset();

    // write the file
    QStringList list = hex->writeHex();
    if (list.isEmpty())
    {
        writeOutput("action save dataset : cancelled");
        QApplication::restoreOverrideCursor();
        return;
    }

    // check if the new file can be written
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));

        writeOutput("action save dataset : impossible because file not ready for writing");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextStream out(&file);
    int i = progBar->value();
    int max = progBar->maximum();
    foreach (QString str, list)
    {
        out << str << "\r\n";
        i++;
        setValueProgressBar(i, max);
    }

    // hide the statusbar
    statusBar()->hide();
    progBar->reset();

    QApplication::restoreOverrideCursor();

    // update Node name with the new file name
    model->renameNode(index, QFileInfo(fileName).fileName());
    hex->setFullName(fileName);

    // log
    writeOutput("action save dataset : performed with success ");
}

void MDImain::saveAs_CsvFile(QModelIndex index)
{
    // get the HexFile Node
    Csv *csv = (Csv*)model->getNode(index);

    // ask for new HexFile name
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QFileInfo(csv->fullName()).fileName(),
                                                    QFileInfo(csv->fullName()).absolutePath(),
                                                    tr("CSV files (*.csv);;all files (*.*)"));
    if (fileName.isEmpty())
    {
        writeOutput("action save CSV file : cancelled");
        return;
    }
    if ( QFileInfo(fileName).suffix().toLower() != "csv")
    {
        fileName.append(".csv");
    }

    // write the file
    //QApplication::setOverrideCursor(Qt::WaitCursor);
    if (csv->save(fileName))
    {
        // update Node name with the new file name
        model->renameNode(index, QFileInfo(fileName).fileName());
        csv->setFullName(fileName);

        // log
        writeOutput("action save CSV file : performed with success ");
    }
    else
    {
        // log
        writeOutput("action save CSV file : not performed ");
    }

    //QApplication::restoreOverrideCursor();


}

void MDImain::saveAs_CdfxFile(QModelIndex index)
{
    // get the HexFile Node
    CdfxFile *cdfx = (CdfxFile*)model->getNode(index);

    // ask for new HexFile name
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QFileInfo(cdfx->fullName()).fileName(),
                                                    QFileInfo(cdfx->fullName()).absolutePath(),
                                                    tr("CDFX files (*.cdfx);;all files (*.*)"));
    if (fileName.isEmpty())
    {
        writeOutput("action save CDFX file : cancelled");
        return;
    }
    if ( QFileInfo(fileName).suffix().toLower() != "cdfx")
    {
        fileName.append(".cdfx");
    }

    // check if the new file can be written
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));

        writeOutput("action save CDFX file : impossible because file not ready for writing");
        return;
    }

    // write the file
    QApplication::setOverrideCursor(Qt::WaitCursor);
    cdfx->save(fileName);
    QApplication::restoreOverrideCursor();

    // update Node name with the new file name
    model->renameNode(index, QFileInfo(fileName).fileName());
    cdfx->setFullName(fileName);

    // log
    writeOutput("action save CDFX : performed with success ");
}

void MDImain::compare_HexFile()
{
    //get hexFiles path
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();

    if (list.count() == 2)
    {
        QString str1 = model->getFullNodeName(list.at(0));
        QString str2 = model->getFullNodeName(list.at(1));

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);
        fComp->setDataset2(str2);
        fComp->on_compare_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);

        //set fComp as one of the hexfile owner
        HexFile *hex1 = (HexFile*)model->getNode(list.at(0));
        hex1->attach(fComp);

        //set fComp as one of the hexfile owner
        HexFile *hex2 = (HexFile*)model->getNode(list.at(1));
        hex2->attach(fComp);
    }
}

void MDImain::quicklookFile()
{
    //get hexFiles path
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();
        if (name.endsWith("HexFile") || name.endsWith("Csv") || name.endsWith("CdfxFile"))
        {
            QString str1 = model->getFullNodeName(index);

            //create a new FormCompare
            FormCompare *fComp = on_actionCompare_dataset_triggered();
            fComp->setDataset1(str1);
            fComp->on_quicklook_clicked();

            //set new FormCompare as activated
            ui->tabWidget->setCurrentWidget(fComp);
        }
        else
        {
            QMessageBox::warning(this, "HEXplorer::quicklook", "Please select first an hex or csv file.",
                                             QMessageBox::Ok);
            return;
        }
    }
}

void MDImain::editChangedLabels()
{
    //get hexFiles path
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //get a pointer on the selected item
    Node *node =  model->getNode(index);

    QString name = typeid(*node).name();
    if (name.endsWith("HexFile"))
    {
        HexFile *hex = (HexFile*)model->getNode(index);
        QString str1 = model->getFullNodeName(index);

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);

        // set the list to edit
        fComp->charList.clear();
        foreach(Data *data, hex->getModifiedData())
        {
            fComp->charList.append(data->name);
        }
        fComp->on_quicklook_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);

        //set fComp as one of the hexfile owner
        hex->attach(fComp);
    }
    else if (name.endsWith("Csv"))
    {
        Csv *csv = (Csv*)model->getNode(index);
        QString str1 = model->getFullNodeName(index);

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);

        // set the list to edit
        fComp->charList.clear();
        foreach(Data *data, csv->getModifiedData())
        {
            fComp->charList.append(data->name);
        }
        fComp->on_quicklook_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);

        //set fComp as one of the hexfile owner
        csv->attach(fComp);
    }
    else if (name.endsWith("CdfxFile"))
    {
        CdfxFile *cdfx = (CdfxFile*)model->getNode(index);
        QString str1 = model->getFullNodeName(index);

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);

        // set the list to edit
        fComp->charList.clear();
        foreach(Data *data, cdfx->getModifiedData())
        {
            fComp->charList.append(data->name);
        }
        fComp->on_quicklook_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);

        //set fComp as one of the hexfile owner
        cdfx->attach(fComp);
    }
    else
    {
        QMessageBox::warning(this, "HEXplorer :: edit changed data", "Please select first an hex, csv or Cdfx file.",
                                         QMessageBox::Ok);
        return;
    }
}

void MDImain::fplotData()
{
    QModelIndexList listIndex = ui->treeView->selectionModel()->selectedIndexes();

    foreach (QModelIndex index, listIndex)
    {
        Data *data = (Data*)model->getNode(index);
        data->plot(this);
    }
}

void MDImain::edit()
{
    QModelIndexList listIndex = ui->treeView->selectionModel()->selectedIndexes();

    foreach (QModelIndex index, listIndex)
    {
        Data *data = (Data*)model->getNode(index);

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

        QString parentName = "";
        if (data->getHexParent())
            parentName = data->getHexParent()->name;
        else if (data->getCsvParent())
            parentName = data->getCsvParent()->name;
        else if (data->getCdfxParent())
            parentName = data->getCdfxParent()->name;
        win->setWindowTitle("HEXplorer :: " + QString(data->name) + " (" + parentName + ")");

        win->show();
        win->resize(600, 400);

    }
}

FormCompare *MDImain::on_actionCompare_dataset_triggered()
{
    //create a new formCompare
    FormCompare *fComp = new FormCompare(model, ui->treeView, ui->treeView_2, this);

    //draw the tab in tabWidget
    QIcon icon;
    icon.addFile(":/icones/milky_loopHex.png");

    //add the widget into tabList
    QString nameTab = "quicklook (" + QString::number(compareTabs) + ")";
    tabList->insert(nameTab, fComp);
    ui->tabWidget->addTab(fComp, icon, nameTab);

    ui->dockWidget->setVisible(true);

    compareTabs++;

    return fComp;
}

void MDImain::export_Subsets()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("HexFile"))
        {
            QMessageBox::warning(this, "HEXplorer::export subsets from Hex file", "Please select first an hex file.",
                                             QMessageBox::Ok);
            return;
        }
        else
        {
            HexFile *hex  = (HexFile*)model->getNode(index);
            A2LFILE *a2l = hex->getParentWp()->a2lFile;

            //choose subset(s)
            QStringList subsetList;
            ChooseSubset *chooseSubset = new ChooseSubset(a2l, hex, subsetList, this);
            chooseSubset->exec();

            if (subsetList.isEmpty())
                return;
            else
            {
                //choose format
                QString exportFormat = "";
                DialogChooseExportFormat *chooseFormat = new DialogChooseExportFormat(&exportFormat, this);
                chooseFormat->exec();

                // display status bar
                statusBar()->show();
                progBar->reset();
                connect(hex, SIGNAL(lineParsed(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                //export
                if (exportFormat == "csv")
                    hex->exportSubsetList2Csv(subsetList);
                else if (exportFormat == "cdf")
                    hex->exportSubsetList2Cdf(subsetList);


                // hide the statusbar
                statusBar()->hide();
                progBar->reset();
            }
        }
    }
}

void MDImain::import_Subsets()
{
    // check if a project is selected in treeView
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.endsWith("HexFile"))
        {
            QMessageBox::warning(this, "HEXplorer::export subsets from Hex file", "Please select first an hex file.",
                                             QMessageBox::Ok);
            return;
        }
        else
        {
            HexFile *hex  = (HexFile*)model->getNode(index);

            QSettings settings;
            QString path = settings.value("currentCsvPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
                                              tr("select Subset files"), path,
                                              tr("CSV files (*.csv);;Cdfx files(*.cdfx)"));

            if (files.isEmpty())
            {
               writeOutput("action open new Subset file : no subset file selected");
               return;
            }
            else
            {
                //Get the project (A2l) name
                QString fullA2lName = model->name(model->parent(index));

                //create a pointer on the WorkProject
                WorkProject *wp = projectList->value(fullA2lName);

                // if no MODULE in ASAP file
                if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                {
                    QMessageBox::information(this, "HEXplorer", tr("no MODULE in ASAP file"));
                    writeOutput("action open new subset : no MODULE in ASAP file");
                    return;
                }

                // Read CSV files
                QMap<QString, Csv*> list;
                foreach (QString fullCsvName, files)
                {
                    //update currentHexPath
                    QSettings settings;
                    QString currentCsvPath = QFileInfo(fullCsvName).absolutePath();
                    settings.setValue("currentCsvPath", currentCsvPath);

                    //start a timer
                    double ti = omp_get_wtime();

                    //add the file to the project
                    Csv *csv = new Csv(fullCsvName, wp, hex->getModuleName());

                    // display status bar
                    statusBar()->show();
                    progBar->reset();
                    connect(csv, SIGNAL(incProgressBar(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                    if (csv->readFile())
                    {
                        QString nameCsv = QFileInfo(fullCsvName).baseName();
                        QStringList listName = nameCsv.split("_");
                        list.insert(listName.at(0), csv);

                        //stop timer
                        double tf = omp_get_wtime();

                        writeOutput("action open new CSV file : CSV file add to project in " + QString::number(tf-ti) + " sec");
                    }
                    else
                    {
                         writeOutput("action open new CSV file : CSV file NOT added to project due to wrong format ");
                    }

                    // hide the statusbar
                    statusBar()->hide();
                    progBar->reset();
                }

                // copy subset
                if (list.isEmpty())
                {
                    return;
                }
                else
                {
                    // copy subset if compatible
                    foreach (QString str, list.keys())
                    {
                        Csv *csv = list.value(str);

                        bool missingLabels = false;
                        bool newLabels = false;
                    }

                }
            }
        }
    }
}

//-------------------- TabList ---------------------//

void MDImain::removeTab( int index )
{
    // get the name of the tab
    QString _name = ui->tabWidget->tabText(index);

    //remove tab from tabWidget
    ui->tabWidget->removeTab(index);

    //delete the widget
    //if (_name.contains("quicklook"))
    //    compareTabs--;

    delete tabList->value(_name);

    //remove widget from tabList
    tabList->remove(_name);

    if (tabList->count() == 0)
    {
        ui->dockWidget->hide();
    }
}

void MDImain::resetAllTableView()
{
    updateView();
}

void MDImain::updateView()
{
    foreach(QWidget *widget, tabList->values())
    {
        QString name = typeid(*widget).name();
        if (name.toLower().endsWith("formcompare"))
        {
            FormCompare *fcomp = (FormCompare*)widget;
            fcomp->resetModel();
        }
    }
}

void MDImain::writeOutput(QString str)
{
    ui->listWidget->addItem(str);
    ui->listWidget->scrollToBottom();
}

void MDImain::writeOutput(QStringList list)
{
    ui->listWidget->addItems(list);
    ui->listWidget->scrollToBottom();
}

void MDImain::nodeSelected()
{
    //QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //int line = model->line(index).toInt() -1;
    //int length = editor->lineLength(line);
    //editor->setCursorPosition(line, 0);

    //editor->setSelection(line , 0, line , length - 1);
}

void MDImain::resizeColumn0()
{
    ui->treeView->resizeColumnToContents(0);
}

void MDImain::on_tabWidget_currentChanged(QWidget* widget)
{
    if (widget)
    {
        QString name = typeid(*widget).name();
        if (name.toLower().endsWith("formcompare"))
        {
            FormCompare *fComp = (FormCompare*)widget;
            if (fComp->getDiffModel())
            {
                ui->treeView_2->setModel(fComp->getDiffModel());
                ui->treeView_2->setColumnHidden(1, true);
            }
            else
            {
                ui->treeView_2->setModel(0);
            }
        }
        else
        {            
            ui->treeView_2->setModel(0);
        }
    }
}

void MDImain::clear_Output()
{
    ui->listWidget->clear();
}

bool MDImain::FileExists(string strFilename)
{
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0)
  {
    blnReturn = true;
  }
  else
  {
    blnReturn = false;
  }

  return blnReturn;
}

void MDImain::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MDImain *mainWin = qobject_cast<MDImain *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MDImain::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    //separatorAct->setVisible(numRecentFiles > 0);
}

QString MDImain::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MDImain::on_actionCheck_for_updates_triggered()
{
    //remove the old zip file used for previous Update
    QFile::remove(qApp->applicationDirPath() + "/update_HEXplorer.exe");

    //DialogUpdate updater("//ivcharb07/pc$imf/USER/Hoel/HEXplorer/update.xml", true, this);
    QUrl url("http://hexplorer.googlecode.com/svn/trunk/src/update.xml");
    DialogHttpUpdate updater(url, this);
}


void MDImain::initCheckHttpUpdates()
{
   //remove the old zip file used for previous Update
   QFile::remove(qApp->applicationDirPath() + "/update_HEXplorer.exe");

   QUrl url("http://hexplorer.googlecode.com/svn/trunk/src/update.xml");
   DialogHttpUpdate updater(url, this);
}


bool MDImain::registerVersion()
{
    QString str("//ivcharb07/pc$imf/USER/Hoel/HEXplorer/HEXplorer_UserList.txt");
    QFileInfo info(str);

    //check if the file exists
    if (info.isFile())
    {
        //open the file in ReadWrite Mode
        QFile file(str);
        QTime timer;
        timer.start();
        bool isFileOpen = false;
        while( !isFileOpen && (timer.elapsed() < 3000) )
        {
            isFileOpen = file.open(QIODevice::ReadWrite);
        }

        //once the file is open, do the job
        if (isFileOpen)
        {
            //read the file into listPc
            QStringList listPC;
            while(!file.atEnd())
            {
                listPC.append(file.readLine());
            }

            //check if the pc is already into the listPc
            //and if true update the listPc
            int countPc = listPC.count();
            bool isPc = false;
            for (int i = 0; i < countPc; i++)
            {
                if (listPC[i].contains(QHostInfo::localHostName()))
                {
                    isPc = true;
                }
            }

            //if pc is not into the listPc, add a line to the file
            if (!isPc)
            {
                QString line = QHostInfo::localHostName() + "\t" +
                               getUserName() + "\t" +
                               QDateTime::currentDateTime().toString() + "\t" +
                               qApp->applicationVersion() + "\t" +
                               QString::number(countPc + 1) + "\n";
                listPC.append(line);
            }

            //copy the listPc into the file
            file.seek(0);
            foreach (QString line, listPC)
            {
                file.write(line.toLocal8Bit().data());
            }

            file.close();
            return true;
        }
    }

    return false;
}

void MDImain::expandNode(Node *node)
{
    QModelIndex index = model->getIndex(node);
    ui->treeView->expand(index);
}

void MDImain::on_actionOpen_Saphir_file_triggered()
{
    #ifdef Q_WS_WIN32
        QAxObject *saphir = new QAxObject( "SaphirComModul.TestDataForKristallExport");
        saphir->dynamicCall("GetKristallFile(\"\",\"D:/Matlab/API/Calibro/Data/\")");
    #endif
}

void MDImain::exception(int, const QString &, const QString &, const QString &)
{
    QMessageBox::information(0, "HEXplorer :: open Saphir",
                             "Saphir is not installed on this computer.");
}

void MDImain::checkDroppedFile(QString str)
{
    Node *rootNode = model->getRootNode();
    Node *node = rootNode->getNode(str);

    if (node == NULL)
    {
        QMessageBox::information(this,"HEXplorer::drop file","please drop an Hex or Csv file");
        return;
    }

    QString name = typeid(*node).name();
    // attach this to the new dropped hex file
    if (name.toLower().endsWith("hexfile"))
    {
        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str);
        fComp->on_quicklook_clicked();

    }
    else if(name.toLower().endsWith("csv"))
    {
        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str);
        fComp->on_quicklook_clicked();
    }
    else if(name.toLower().endsWith("a2lfile"))
    {
        //create a textEditor
        A2LFILE *a2l = (A2LFILE*)node;
        QFile qfile(a2l->fullName());
        editTextFile(qfile);
    }
    else if (name.toLower().endsWith("cdfxfile"))
    {
        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str);
        fComp->on_quicklook_clicked();
    }
    else
    {
        QMessageBox::information(this,"HEXplorer::drop file","please drop an A2l,Hex or Csv file");
    }
}

void MDImain::setValueProgressBar(int n, int max)
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    progBar->setMaximum(max);
    progBar->setValue(n);
}

QString MDImain::getUserName()
{
    #ifdef Q_WS_WIN32
        QString userName = "";
        #if defined(UNICODE)
        if ( QSysInfo::windowsVersion () ==  QSysInfo::WV_NT)
        {
            TCHAR winUserName[UNLEN + 1]; // UNLEN is defined in LMCons.h
            DWORD winUserNameSize = sizeof(winUserName);
            GetUserName( winUserName, &winUserNameSize );
            userName = QString::fromUtf16((ushort*)winUserName );
        } else
        #endif
        {
            char winUserName[UNLEN + 1]; // UNLEN is defined in LMCons.h
            DWORD winUserNameSize = sizeof(winUserName);
            GetUserNameA( winUserName, &winUserNameSize );
            userName = QString::fromLocal8Bit( winUserName );
        }
        return userName;
    #endif

}
