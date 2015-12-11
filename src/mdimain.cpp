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

#include <QtWidgets>
#include <QtCore>
#include <cstdarg>
#include <omp.h>
#include <typeinfo>
#include <qtconcurrentrun.h>
#include <QFutureWatcher>
#include <sys/stat.h>
#include <QSqlDriver>
#include <QSqlTableModel>
#include <QSqlQuery>
#include "sqlite3.h"


#ifdef Q_OS_WIN32
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
#include "Nodes/dbfile.h"
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
#include "comparemodel.h"
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
#ifdef Q_OS_WIN32
    #include "LMCons.h"
    #include "windows.h"
#endif
#include "freezetablewidget.h"
#include "dialoghttpupdate.h"
#include "workingdirectory.h"

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
    ui->Differentlabels_Dock->setVisible(false);
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
    connect(ui->tabWidget, SIGNAL(textDropped(QString)), this, SLOT(checkDroppedFile(QString)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidget_currentChanged(int)));
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(nodeSelected()));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));

    //check for updates
    connect(this, SIGNAL(check()), this, SLOT(initCheckHttpUpdates()), Qt::QueuedConnection);
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    if ((settings.value("Update/automatic") == true) || (!settings.contains("Update/automatic")))
        emit check();

    //create settings variables for multi_thread and lexer type
    if (!settings.contains("openMP"))
        settings.setValue("openMP", 1);
    if (!settings.contains("lexer"))
        settings.setValue("lexer", "Quex");

    // initialize working directory
    int autoWD = settings.value("autoWD").toInt();
    if (autoWD == 1)
    {
        workingDirectory = settings.value("currentWDPath").toString();
        QStringList list = workingDirectory.split(";");
        workingDirectory = "";
        foreach (QString _str, list)
        {
            if (!_str.isEmpty())
            {
                QDir dir(_str);
                if (dir.exists())
                {
                    openWorkingDirectory(_str);
                    workingDirectory.append(_str + ";");
                }

            }
        }
        settings.setValue("currentWDPath", workingDirectory);
    }
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
    if (checkChangedHexFiles() && checkChangedCsvFiles()
            && checkChangedSrecFiles() && checkChangedCdfxFiles())
    {
        QSettings settings(qApp->organizationName(), qApp->applicationName());
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        e->accept();
    }
    else
        e->ignore();
}

void MDImain::readSettings()
{
    QSettings settings(qApp->organizationName(), qApp->applicationName());
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

    addSrecFile = new QAction(tr("Import Srec file"), this);
    addSrecFile->setIcon(QIcon(":/icones/milky_importS19.png"));
    connect(addSrecFile, SIGNAL(triggered()), this, SLOT(addSrecFile2Project()));
    addSrecFile->setDisabled(true);

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

    compareHexFile = new QAction(tr("compare dataset"), this);
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

    editLabelCompare= new QAction(tr("edit"), this);
    editLabelCompare->setIcon(QIcon(":/icones/eye.png"));
    connect(editLabelCompare, SIGNAL(triggered()), this, SLOT(editCompare()));
    editLabelCompare->setDisabled(false);

    saveA2lDB = new QAction(tr("save A2l into DB"), this);
    saveA2lDB->setIcon(QIcon(":/icones/milky_exportBaril.png"));
    connect(saveA2lDB, SIGNAL(triggered()), this, SLOT(exportA2lDb()));
    saveA2lDB->setDisabled(true);

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
    ui->toolBar_a2l->addAction(ui->actionOpen_Working_Directory);
    ui->toolBar_a2l->addAction(ui->actionNewA2lProject);
    //ui->toolBar_a2l->addAction(ui->actionLoad_DB);
    ui->toolBar_a2l->addAction(addHexFile);
    ui->toolBar_a2l->addAction(addSrecFile);
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
        addSrecFile->setEnabled(true);
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
        saveA2lDB->setEnabled(true);
        ui->actionClose_Working_Directory->setEnabled(false);
    }
    else if (name.endsWith("DBFILE"))
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        editChanged->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
        deleteProject->setEnabled(true);
        deleteFile->setEnabled(false);
        editFile->setEnabled(false);
        childCount->setEnabled(true);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(false);
        sortBySubset->setEnabled(false);
        saveFile->setEnabled(false);
        saveAsFile->setEnabled(false);
        quicklook->setEnabled(false);
        readValuesFromCsv->setEnabled(false);
        readValuesFromCdfx->setEnabled(false);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(false);
    }
    else if (name.endsWith("HexFile"))
    {
        importSubsets->setEnabled(true);
        exportSubsets->setEnabled(true);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
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
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(false);

        ui->toolBar_data->show();

    }
    else if (name.endsWith("SrecFile"))
    {
        importSubsets->setEnabled(true);
        exportSubsets->setEnabled(true);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
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
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(false);

        ui->toolBar_data->show();

    }
    else if (name.endsWith("Csv"))
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
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
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(false);

        ui->toolBar_data->show();
    }
    else if (name.endsWith("CdfxFile"))
    {
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
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
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(false);

        ui->toolBar_data->show();
    }
    else if (name.toLower().endsWith("workingdirectory"))
    {        
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        editChanged->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
        deleteProject->setEnabled(false);
        deleteFile->setEnabled(false);
        editFile->setEnabled(false);
        childCount->setEnabled(false);
        showParam->setEnabled(false);
        resetAllChangedData->setEnabled(false);
        sortBySubset->setEnabled(false);
        saveFile->setEnabled(false);
        saveAsFile->setEnabled(false);
        quicklook->setEnabled(false);
        readValuesFromCsv->setEnabled(false);
        readValuesFromCdfx->setEnabled(false);
        editMeasChannels->setEnabled(false);
        editCharacteristics->setEnabled(false);
        openJScript->setEnabled(false);
        saveA2lDB->setEnabled(false);
        ui->actionClose_Working_Directory->setEnabled(true);

    }
    else
    {
        importSubsets->setEnabled(false);
        exportSubsets->setEnabled(false);
        addCdfxFile->setEnabled(false);
        addCsvFile->setEnabled(false);
        addHexFile->setEnabled(false);
        addSrecFile->setEnabled(false);
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
        saveA2lDB->setEnabled(false);
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

        menu.addAction(ui->actionOpen_Working_Directory);
        menu.addSeparator();
        menu.addAction(ui->actionNewA2lProject);

        for (int i = 0; i < MaxRecentFiles; ++i)
        {
             recentProMenu->addAction(recentFileActs[i]);
        }
        updateRecentFileActions();
        menu.addMenu(recentProMenu);
        menu.addSeparator();
        menu.addAction(ui->actionLoad_DB);
    }
    else
    {
        QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();
        if (list.count() < 1)
        {
            menu.addAction(ui->actionOpen_Working_Directory);
            menu.addSeparator();
            menu.addAction(ui->actionNewA2lProject);

            for (int i = 0; i < MaxRecentFiles; ++i)
            {
                 recentProMenu->addAction(recentFileActs[i]);
            }
            updateRecentFileActions();
            menu.addMenu(recentProMenu);

            menu.addSeparator();
            menu.addAction(ui->actionLoad_DB);
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
//                A2LFILE *a2l = dynamic_cast<A2LFILE *> (hex->getParentNode());
//                QString projectName = ((PROJECT*)a2l->getProject())->getPar("name");
//                projectName = projectName.toLower();

//                if (projectName == "c340" || projectName == "c342" || projectName == "p_662")
//                {
//                    verify->setDisabled(false);
//                    checkFmtc->setDisabled(false);
//                }
//                else
//                {
//                    verify->setDisabled(true);
//                    checkFmtc->setDisabled(true);
//                }

                verify->setDisabled(true);
                checkFmtc->setDisabled(true);

            }
            else if (name.toLower().endsWith("srecfile"))
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
                SrecFile *srec = dynamic_cast<SrecFile *> (node);

                if (srec->getModifiedData().isEmpty())
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
            }
            else if (name.toLower().endsWith("a2lfile"))
            {
                A2LFILE *a2lFile = (A2LFILE*)node;
                if (a2lFile->isParsed())
                {
                    if (a2lFile->isConform())
                    {
                        menu.addAction(ui->actionNewA2lProject);
                        menu.addAction(ui->actionLoad_DB);
                        menu.addAction(deleteProject);
                        menu.addAction(editFile);
                        menu.addSeparator();
                        menu.addAction(addHexFile);
                        menu.addAction(addSrecFile);
                        menu.addAction(addCsvFile);
                        menu.addAction(addCdfxFile);
                        menu.addSeparator();
                        menu.addAction(openJScript);
                        menu.addSeparator();
                        menu.addAction(editMeasChannels);
                        menu.addAction(editCharacteristics);
                        menu.addSeparator();
                        menu.addAction(saveA2lDB);
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
                else
                {
                    menu.addAction(ui->actionNewA2lProject);
                    menu.addAction(ui->actionLoad_DB);
                    menu.addAction(deleteProject);
                    menu.addAction(editFile);
                    menu.addSeparator();
                    menu.addAction(addHexFile);
                    menu.addAction(addSrecFile);
                    menu.addAction(addCsvFile);
                    menu.addAction(addCdfxFile);
                    menu.addSeparator();
                    menu.addAction(openJScript);
                    menu.addSeparator();
                    menu.addAction(editMeasChannels);
                    menu.addAction(editCharacteristics);
                    menu.addSeparator();
                    menu.addAction(saveA2lDB);
                }
             }
            else if (name.toLower().endsWith("dbfile"))
            {
                menu.addAction(ui->actionNewA2lProject);
                menu.addAction(ui->actionLoad_DB);
                menu.addAction(deleteProject);
                //menu.addAction(editFile);
                menu.addSeparator();
                menu.addAction(addHexFile);
                menu.addAction(addSrecFile);
                menu.addAction(addCsvFile);
                menu.addAction(addCdfxFile);
                menu.addSeparator();
                menu.addAction(openJScript);
                //menu.addSeparator();
                //menu.addAction(editMeasChannels);
                //menu.addAction(editCharacteristics);
                //menu.addSeparator();
                //menu.addAction(saveA2lDB);
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
            else if (name.toLower().endsWith("workingdirectory"))
            {
                menu.addAction(ui->actionClose_Working_Directory);
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
            int weiterWD = 0;
            foreach (QModelIndex index, list)
            {
                Node *node = model->getNode(index);
                QString name = typeid(*node).name();
                if (name.endsWith("Data"))
                {
                    weiterData++;
                }
                else if (name.toLower().endsWith("hexfile") || name.toLower().endsWith("srecfile"))
                {
                    weiterHex++;
                }
                else if (name.toLower().endsWith("a2lfile"))
                {
                    weiterA2l++;
                }
                else if (name.toLower().endsWith("workingdirectory"))
                {
                    weiterWD++;
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
            else if (weiterWD == list.count())
            {
                menu.addAction(ui->actionClose_Working_Directory);
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
        menu.addAction(editLabelCompare);
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
                                 "the whole FMTC map is strictly monotonic");
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
                             "the FMTC map is NOT strictly monotonic : \n"
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
    QByteArray compiler = "GNU compiler.";
    #ifdef CL_COMPILER
        compiler = "msvc2013.";
    #endif
    QByteArray encodedString = "Christophe Hoël"
                   "\n\n"
                   "special thanks to :\n"
                   "Oscar, Niklaus, Jimi, Zack, Eric, Oneyed Jack, lofofora\n"
                   "M, Radio Tarifa, Al, John, Paco, Noir dez, et tous les autres...pour le bon son.\n\n"
                   "build " + qApp->applicationVersion().toLocal8Bit() + " compiled with " + compiler + "\n\n"
                   "This software uses external libraries :\n"
                   "   - Qt framework " + QT_VERSION_STR + "\n"
                   "   - Quex 0.65.4 (as efficient lexical analyser generator)\n"
                   "   - QScintilla 2.9 (as efficient text editor)\n"
                   "   - Qwt 6.1.2 (as 2D graph plotter)\n"
                   "   - MathGL 2.3.3 (as 3D graph plotter)\n\n"
                   "Please visit the following link for more information :\n"
                    "http://lmbhoc1.github.io/HEXplorer/";

    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-1");
    QString string = codec->toUnicode(encodedString);

    QMessageBox::about(this, tr("HEXplorer :: About"), string);
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
    if (name.toLower().endsWith("hexfile") || name.toLower().endsWith("srecfile") ||
        name.toLower().endsWith("csv") || name.toLower().endsWith("cdfxfile"))
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

bool MDImain::checkChangedSrecFiles()
{
    bool bl = true;

    // check if Hex files have changes
    foreach (WorkProject *wp, projectList->values())
    {
        foreach (SrecFile *srec, wp->srecFiles().values())
        {
            if (srec->childNodes.count() != 0)
            {
                int ret = QMessageBox::question(this, "HEXplorer :: exit",
                                      "Save changes to " + srec->fullName(),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

                if (ret == QMessageBox::Yes)
                {
                    QModelIndex index = ((A2lTreeModel*)ui->treeView->model())->getIndex(srec);
                    save_SrecFile(index);
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

bool MDImain::checkChangedCdfxFiles()
{
    bool bl = true;

    // check if Hex files have changes
    foreach (WorkProject *wp, projectList->values())
    {
        foreach (CdfxFile *cdfx, wp->cdfxFiles().values())
        {
            if (cdfx->childNodes.count() != 0)
            {
                int ret = QMessageBox::question(this, "HEXplorer :: exit",
                                      "Save changes to " + cdfx->fullName(),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

                if (ret == QMessageBox::Yes)
                {
                    QModelIndex index = ((A2lTreeModel*)ui->treeView->model())->getIndex(cdfx);
                    save_CdfxFile(index);
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
    QSettings settings(qApp->organizationName(), qApp->applicationName());
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
    QSettings settings(qApp->organizationName(), qApp->applicationName());
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

void MDImain::insertWp(WorkProject *wp)
{
    this->projectList->insert(QString(wp->getFullA2lFileName().c_str()), wp);
}

void MDImain::on_actionOpen_Working_Directory_triggered()
{
    //path
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString path = settings.value("currentWDPath").toString();

    //QFileDialog to select multiple directories
    QFileDialog w;
    w.setFileMode(QFileDialog::Directory);
    w.setOption(QFileDialog::DontUseNativeDialog,true);
    w.setOption(QFileDialog::ShowDirsOnly, true);
    w.setDirectory(QDir(path));
    QListView *l = w.findChild<QListView*>("listView");
    if (l)
    {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    QTreeView *t = w.findChild<QTreeView*>();
    if (t)
    {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    QStringList pathList;
    if (w.exec())
        pathList = w.selectedFiles();


//    QString dir = QFileDialog::getExistingDirectory(this, tr("please select directories"),
//                                                    path,
//                                                    QFileDialog::ShowDirsOnly
//                                                    | QFileDialog::DontResolveSymlinks);


    if (pathList.isEmpty())
    {
       statusBar()->showMessage(tr("no Working Directory selected"), 2000);
       writeOutput("action open working directory : no folder selected");
       return;
    }
    else
    {
        //update currentWDPath
        int ret = QMessageBox::question(this, "HEXplorer :: Working Directory",
                              "Do you want HEXplorer to automatically open this directory at application launch?",
                              QMessageBox::Yes, QMessageBox::No);

        if (ret == QMessageBox::Yes)
        {
               settings.setValue("autoWD", 1);

               QStringList currentPathList = workingDirectory.split(";");
               foreach (QString str, pathList)
               {
                   if (!currentPathList.contains(str))
                   {
                        workingDirectory.append(str + ";");
                        openWorkingDirectory(str);
                   }
               }
               settings.setValue("currentWDPath", workingDirectory);
        }
        else if (ret == QMessageBox::No)
        {
            QStringList currentPathList = workingDirectory.split(";");
            foreach (QString str, pathList)
            {
                if (!currentPathList.contains(str))
                {
                     openWorkingDirectory(str);
                }
            }
        }
    }
}

void MDImain::openWorkingDirectory(QString rootPath)
{
    //create a new WorkingDirectory instance
    WorkingDirectory* nodeWd = new WorkingDirectory(rootPath, model, this);

    //update the model and treeview
    model->addNode2RootNode(nodeWd);

    //expand node in treeView
    ui->treeView->setModel(model);
    completer->setModel(model);
    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setExpanded(model->getIndex(nodeWd), true);

}

void MDImain::on_actionClose_Working_Directory_triggered()
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

            if (!name.toLower().endsWith("workingdirectory"))
            {
                QMessageBox::warning(this, "HEXplorer::remove project", "Please select first a working directory.",
                                                 QMessageBox::Ok);
                return;
            }


            int r = QMessageBox::question(this, "HEXplorer::question", "Are you sure you want to remove the working directory " +
                                          QString(node->name)  + " ?",
                                  QMessageBox::Yes, QMessageBox::No);

            if (r ==  QMessageBox::Yes)
            {
                    removeWorkingDirectory(index);
            }
        }
    }
    else
    {
        int r = QMessageBox::question(this, "HEXplorer::question",
                                      "Remove all the selected working directories ?",
                                       QMessageBox::Yes, QMessageBox::No);

        if (r ==  QMessageBox::Yes)
        {
            foreach (QModelIndex index, ui->treeView->selectionModel()->selectedIndexes())
            {

                removeWorkingDirectory(index);
            }
        }
    }
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

        if (!name.endsWith("A2LFILE") && !name.endsWith("DBFILE"))
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
            QSettings settings(qApp->organizationName(), qApp->applicationName());
            QString path = settings.value("currentHexPath").toString();

            QStringList files = QFileDialog::getOpenFileNames(this,
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

                if (wp && name.endsWith("A2LFILE"))
                {
                    //if the a2lFile is not yet parsed, parse.
                    if (!wp->a2lFile->isParsed())
                    {
                        readA2l(wp);
                    }

                    //if no MOD_COMMON in ASAP file
                    if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                    {
                        QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                        writeOutput("action open new dataset : no MOD_COMMON in ASAP file");
                        return;
                    }

                    //check if Hexfile already in project
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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                        {
                            wp->addHex(hex);
                        }
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
                else if (wp && name.endsWith("DBFILE"))
                {
                    //TBD : perform a check of the DB to ensure contains everything to open Hexfile

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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
                        QString currentHexPath = QFileInfo(fullHexName).absolutePath();
                        settings.setValue("currentHexPath", currentHexPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project (module)
                        HexFile *hex = new HexFile(fullHexName, wp);

                        // display status bar
                        statusBar()->show();
                        progBar->reset();
                        connect(hex, SIGNAL(progress(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                        //read hexFile and add node to parent Node
                        if (hex->read_db())
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

void MDImain::addSrecFile2Project()
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
            QSettings settings(qApp->organizationName(), qApp->applicationName());
            QString path = settings.value("currentSrecPath").toString();

            QStringList files =
                    QFileDialog::getOpenFileNames(this,
                                              tr("select a dataset (Srec)"), path,
                                              tr("Srec files (*.s19 | *.s32);;all files (*.*)"));


            if (files.isEmpty())
            {
               statusBar()->showMessage(tr("Loading canceled"), 2000);
               writeOutput("action open new dataset : no Srec file selected");
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
                    //if the a2lFile is not yet parsed, parse.
                    if (!wp->a2lFile->isParsed())
                    {
                        //get a pointer on old a2lFile
                        A2LFILE *a2lfile = wp->a2lFile;

                        //remove old a2lFile from childnodes
                        model->removeChildNodeFromRoot(wp->a2lFile);


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

                        //add to treemodel
                        model->addNode2RootNode(wp->a2lFile);

                        //finally delete previous/old a2lFile
                        //delete a2lfile;
                    }

                    // if no MOD_COMMON in ASAP file
                    if (wp->a2lFile->getProject()->getNode("MODULE") == NULL)
                    {
                        QMessageBox::information(this, "HEXplorer", tr("no MOD_COMMON in ASAP file"));
                        writeOutput("action open new dataset : no MOD_COMMON in ASAP file");
                        return;
                    }

                    // check if Hexfile already in project
                    foreach (QString fullSrecName, files)
                    {
                        //if the selected Srec file is already into the project => exit
                        if (wp->hexFiles().contains(fullSrecName))
                        {
                            QMessageBox::information(this, "HEXplorer", "Srec file : " + fullSrecName
                                                     + "\nalready included into the selected project");
                            writeOutput("action open new dataset : Srec file already in project");
                            files.removeOne(fullSrecName);
                        }
                    }

                    //Open Srec files
                    foreach (QString fullSrecName, files)
                    {
                        //update currentSrecPath
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
                        QString currentSrecPath = QFileInfo(fullSrecName).absolutePath();
                        settings.setValue("currentSrecPath", currentSrecPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project (module)
                        SrecFile *srec = NULL;
                        QList<MODULE*> list = wp->a2lFile->getProject()->listModule();
                        if (list.count() == 0)
                        {
                            writeOutput("action open new dataset : no Module into A2l file !");
                            return;
                        }
                        else if (list.count() == 1)
                        {
                            srec = new SrecFile(fullSrecName, wp, QString(list.at(0)->name));
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
                                srec = new SrecFile(fullSrecName, wp, module);
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
                        connect(srec, SIGNAL(progress(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                        if (srec->read())
                        {
                            wp->addSrec(srec);
                        }
                        else
                            delete srec;

                        // hide the statusbar
                        statusBar()->hide();
                        progBar->reset();

                        //stop timer
                        double tf = omp_get_wtime();

                        //update the treeView model
                        ui->treeView->expand(index);
                        ui->treeView->resizeColumnToContents(0);

                        writeOutput("action open new dataset : Srec file add to project in " + QString::number(tf-ti) + " sec");

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
            QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                    //if the a2lFile is not yet parsed, parse.
                    if (!wp->a2lFile->isParsed())
                    {
                        readA2l(wp);
                    }

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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
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
            QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                    //if the a2lFile is not yet parsed, parse.
                    if (!wp->a2lFile->isParsed())
                    {
                        readA2l(wp);
                    }

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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
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

            if (name.endsWith("HexFile") || name.endsWith("SrecFile") || name.endsWith("Csv") || name.endsWith("CdfxFile"))
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
                QMessageBox::warning(this, "HEXplorer::remove file from project", "Please select an hex, Srec,csv or cdfx file.",
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

        model->beginReset();

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
               // model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }

            //hide toolbar hex
            ui->toolBar_data->hide();
        }
        else if (name.endsWith("SrecFile"))
        {
            //ask for save changes
            SrecFile *srec = dynamic_cast<SrecFile *> (node);
            if (srec->childNodes.count() != 0)
            {
                // ask for saving changes
                int r = QMessageBox::question(this, "HEXplorer::question", tr("Save changes ?"),
                                      QMessageBox::Yes, QMessageBox::No);

                if (r ==  QMessageBox::Yes)
                {
                    if (!save_SrecFile(index))
                    {
                        return;
                    }
                }

                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (srec->getParentNode());
                a2l->removeChildNode(srec);

                //get the parentWp of the HexFile and delete Hex
                WorkProject *wp = srec->getParentWp();
                wp->removeSrecFile(srec); // delete hex included

                //update the treeView
                model->update();
                ui->treeView->expand(indexParent);
                ui->treeView->resizeColumnToContents(0);
            }
            else
            {
                //get the parentNode of the HexFile (which must be an A2LFILE !!)
                A2LFILE *a2l = dynamic_cast<A2LFILE *> (srec->getParentNode());
                a2l->removeChildNode(srec);

                //get the parentWp of the HexFile
                WorkProject *wp = srec->getParentWp();
                wp->removeSrecFile(srec);

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

        model->endReset();

        //expand treeView
        ui->treeView->expand(indexParent);
        ui->treeView->expand(indexParent.parent());


    }

}

void MDImain::reAppendProject(WorkProject *wp)
{
    wp->attach(this);

    //insert the new created project into the projectList
    projectList->insert(wp->getFullA2lFileName().c_str(), wp);

    //update model
    Node* parentNode = wp->a2lFile->getParentNode();
    model->beginReset();
    parentNode->addChildNode(wp->a2lFile);
    parentNode->sortChildrensName();
    wp->a2lFile->setParentNode(parentNode);
    model->endReset();

    //update treeView
    QModelIndex parentIndex = model->getIndex(parentNode);
    ui->treeView->expand(parentIndex);
    QModelIndex index = model->getIndex(wp->a2lFile);
    ui->treeView->expand(index);
    ui->treeView->setColumnHidden(1, true);
}

void MDImain::showFixPar()
{
    //search the index of the current node in the Model
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();

    //create a dialog window
    QMap<std::string, std::string> *param = model->getPar(index);

    if (param)
    {
        Dialog *diag = new Dialog();

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
            if (name.endsWith("HexFile") || name.endsWith("Csv") || name.endsWith("SrecFile") ||
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

void MDImain:: removeWorkProjects()
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

            if (!name.endsWith("A2LFILE") && !name.endsWith("DBFILE"))
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

        QModelIndex indexParent = index.parent();

        if (!name.endsWith("A2LFILE") && !name.endsWith("DBFILE"))
        {
            QMessageBox::warning(this, "HEXplorer::remove project", "Please select first a project.",
                                             QMessageBox::Ok);
            return;
        }

        if (name.endsWith("A2LFILE"))
        {
            //As the selected node is an A2l file we can cast the node into its real type : A2LFILE
            A2LFILE *a2lfile = dynamic_cast<A2LFILE *> (node);

            //ask to save the modified nodes (hex or Csv)
            foreach (Node *node, a2lfile->childNodes)
            {
                QString name = typeid(*node).name();
                if (name.toLower().endsWith("hexfile"))
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
                else if (name.toLower().endsWith("srecfile"))
                {
                    SrecFile *srec = (SrecFile*)node;
                    if (!srec->getModifiedData().isEmpty())
                    {
                        int r = QMessageBox::question(this, "HEXplorer::question",
                                                      "Save changes in " + QString(srec->name) + "?",
                                                      QMessageBox::Yes, QMessageBox::No);
                        if (r == QMessageBox::Yes)
                        {
                          QModelIndex srecIndex = model->getIndex(srec);
                          save_SrecFile(srecIndex);
                        }
                    }
                }
                else if (name.toLower().endsWith("csv"))
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
                else if (name.toLower().endsWith("cdfxfile"))
                {
                    CdfxFile *cdfx = (CdfxFile*)node;
                    if (!cdfx->getModifiedData().isEmpty())
                    {
                        int r = QMessageBox::question(this, "HEXplorer::question",
                                                      "Save changes in " + QString(cdfx->name) + "?",
                                                      QMessageBox::Yes, QMessageBox::No);
                        if (r == QMessageBox::Yes)
                        {
                          QModelIndex cdfxIndex = model->getIndex(cdfx);
                          save_CdfxFile(cdfxIndex);
                        }
                    }
                }
            }

            //remove the node from treeView model
            model->beginReset();
            Node* nodeParent = node->getParentNode();
            nodeParent->removeChildNode(node);
            model->endReset();

            //update the treeView
            //model->update();
            ui->treeView->resizeColumnToContents(0);

            //get the project
            WorkProject *wp = projectList->value(a2lfile->fullName());

            //remove the project from the this->projectList
            projectList->remove(a2lfile->fullName());

            //delete the selected project
            //delete wp;
            wp->detach(this);
        }
        else if (name.endsWith("DBFILE"))
        {
            //As the selected node is an A2l file we can cast the node into its real type : A2LFILE
            DBFILE *dbfile = dynamic_cast<DBFILE *> (node);

            //ask to save the modified nodes (hex or Csv)
            foreach (Node *node, dbfile->childNodes)
            {
                QString name = typeid(*node).name();
                if (name.toLower().endsWith("hexfile"))
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
                else if (name.toLower().endsWith("srecfile"))
                {
                    SrecFile *srec = (SrecFile*)node;
                    if (!srec->getModifiedData().isEmpty())
                    {
                        int r = QMessageBox::question(this, "HEXplorer::question",
                                                      "Save changes in " + QString(srec->name) + "?",
                                                      QMessageBox::Yes, QMessageBox::No);
                        if (r == QMessageBox::Yes)
                        {
                          QModelIndex srecIndex = model->getIndex(srec);
                          save_SrecFile(srecIndex);
                        }
                    }
                }
                else if (name.toLower().endsWith("csv"))
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
                else if (name.toLower().endsWith("cdfxfile"))
                {
                    CdfxFile *cdfx = (CdfxFile*)node;
                    if (!cdfx->getModifiedData().isEmpty())
                    {
                        int r = QMessageBox::question(this, "HEXplorer::question",
                                                      "Save changes in " + QString(cdfx->name) + "?",
                                                      QMessageBox::Yes, QMessageBox::No);
                        if (r == QMessageBox::Yes)
                        {
                          QModelIndex cdfxIndex = model->getIndex(cdfx);
                          save_CdfxFile(cdfxIndex);
                        }
                    }
                }
            }

            //remove the node from treeView model
            model->removeChildNodeFromRoot(node);

            //update the treeView
            model->update();
            ui->treeView->resizeColumnToContents(0);

            //get the project
            WorkProject *wp = projectList->value(dbfile->fullName());

            //remove the project from the this->projectList
            projectList->remove(dbfile->fullName());

            //remove connection to database and close the DB
            QSqlDatabase::database(dbfile->getSqlConnection(), true).close();
            QSqlDatabase::removeDatabase(dbfile->getSqlConnection());
            ui->connectionWidget->refresh();

            //delete the selected project
            //delete wp;
            wp->detach(this);
        }

        ui->treeView->expand(indexParent);

    }
}

void MDImain::removeWorkingDirectory(QModelIndex index)
{
    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);
        QString name = typeid(*node).name();

        if (!name.toLower().endsWith("workingdirectory"))
        {
            QMessageBox::warning(this, "HEXplorer::remove project", "Please select first a working directory.",
                                             QMessageBox::Ok);
            return;
        }
        else
        {
            //As the selected node is an A2l file we can cast the node into its real type
            WorkingDirectory *wd = dynamic_cast<WorkingDirectory *> (node);

            //remove all wprk projects into working directory
            foreach (Node *_node, wd->childNodes)
            {
                QString name = typeid(*_node).name();
                if (name.toLower().endsWith("a2lfile"))
                {
                    removeWorkProject(model->getIndex(_node));
                }
            }

            //remove the node from treeView model
            model->beginReset();
            Node* nodeParent = node->getParentNode();
            nodeParent->removeChildNode(node);
            model->endReset();

            //remove working directory from list
            QStringList listPath  = workingDirectory.split(";");
            listPath.removeAll(wd->getFullPath());
            workingDirectory = "";
            foreach (QString str, listPath)
            {
                workingDirectory.append(str + ";");
            }
            QSettings settings(qApp->organizationName(), qApp->applicationName());
            settings.setValue("currentWDPath", workingDirectory);
        }
    }
}

void MDImain::export_ListData()
{
    QStringList strList;
    strList.append("[Label]");

    QStringList listData = ((A2lTreeModel*)ui->treeView_2->model())->getListDataName();
    strList.append(listData);

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


            //if the a2lFile is not yet parsed, parse.
            if (!wp->a2lFile->isParsed())
            {
                readA2l(wp);
            }

            // get the list of MEASUREMET
            QList<Node*> list;
            Node *module = wp->a2lFile->getProject()->getNode("MODULE");
            if (module == NULL)
            {
                return;
            }
            else
            {
                QString _module;
                QList<MODULE*> listModule = wp->a2lFile->getProject()->listModule();
                if (listModule.count() == 0)
                {
                    writeOutput("action edit measurement channels : no Module into A2l file !");
                    return;
                }
                else if (listModule.count() == 1)
                {
                    _module = QString(listModule.at(0)->name);
                }
                else
                {
                    // select a module
                    QString module;
                    DialogChooseModule *diag = new DialogChooseModule(&module);
                    QStringList listModuleName;
                    foreach (MODULE* module, listModule)
                    {
                        listModuleName.append(module->name);
                    }
                    diag->setList(listModuleName);
                    int ret = diag->exec();

                    if (ret == QDialog::Accepted)
                    {
                         _module = module;
                    }
                    else
                    {
                        writeOutput("action open new dataset : no module chosen !");
                        return;
                    }
                }

                Node *dim = module->getNode(_module);
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

            //if the a2lFile is not yet parsed, parse.
            if (!wp->a2lFile->isParsed())
            {
                readA2l(wp);
            }

            // get the list of MEASUREMET
            QList<Node*> list;
            Node *module = wp->a2lFile->getProject()->getNode("MODULE");
            if (module == NULL)
            {
                return;
            }
            else
            {
                QString _module;
                QList<MODULE*> listModule = wp->a2lFile->getProject()->listModule();
                if (listModule.count() == 0)
                {
                    writeOutput("action edit characteristics : no Module into A2l file !");
                    return;
                }
                else if (listModule.count() == 1)
                {
                    _module = QString(listModule.at(0)->name);
                }
                else
                {
                    // select a module
                    QString module;
                    DialogChooseModule *diag = new DialogChooseModule(&module);
                    QStringList listModuleName;
                    foreach (MODULE* module, listModule)
                    {
                        listModuleName.append(module->name);
                    }
                    diag->setList(listModuleName);
                    int ret = diag->exec();

                    if (ret == QDialog::Accepted)
                    {
                         _module = module;
                    }
                    else
                    {
                        writeOutput("action open new dataset : no module chosen !");
                        return;
                    }
                }

                Node *dim = module->getNode(_module);
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

            //display the characterisitc channels in view
            CharModel *charModel = new CharModel();
//            charModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
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
        writeOutput("characteristics edited.");

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
        //QString moduleName = "DIM";
        QString moduleName1;
        QString moduleName2;

        // get a2l file path
        QString str1 = model->name(list.at(0));
        QString str2 = model->name(list.at(1));


        // get listChar1
        WorkProject *wp1 = projectList->value(str1);
        QStringList list1;
        if (wp1)  //to prevent any crash of the aplication
        {
            //if the a2lFile is not yet parsed, parse.
            if (!wp1->a2lFile->isParsed())
            {
                readA2l(wp1);
            }

            // if MODULE in ASAP file
            if (wp1->a2lFile->getProject()->getNode("MODULE") != NULL)
            {
                //choose the module
                QList<MODULE*> list = wp1->a2lFile->getProject()->listModule();
                if (list.count() == 0)
                {
                    writeOutput("action comapre a2l cancelled : no Module into A2l file !");
                    return;
                }
                else if (list.count() == 1)
                {
                    moduleName1 = QString(list.at(0)->name);
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
                       moduleName1 = QString(list.at(0)->name);
                    }
                    else
                    {
                        writeOutput("action compare a2l : no module chosen !");
                        return;
                    }
                }

                MODULE *mod = (MODULE*)wp1->a2lFile->getProject()->getNode("MODULE/" + moduleName1);
                list1 = mod->listChar;
            }
        }

        // get CHAR listChar2
        WorkProject *wp2 = projectList->value(str2);
        QStringList list2;
        if (wp2)  //to prevent any crash of the aplication
        {
            //if the a2lFile is not yet parsed, parse.
            if (!wp2->a2lFile->isParsed())
            {
                readA2l(wp2);
            }

            // if MODULE in ASAP file
            if (wp2->a2lFile->getProject()->getNode("MODULE") != NULL)
            {
                //choose the module
                QList<MODULE*> list = wp2->a2lFile->getProject()->listModule();
                if (list.count() == 0)
                {
                    writeOutput("action comapre a2l cancelled : no Module into A2l file !");
                    return;
                }
                else if (list.count() == 1)
                {
                    moduleName2 = QString(list.at(0)->name);
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
                       moduleName2 = QString(list.at(0)->name);
                    }
                    else
                    {
                        writeOutput("action compare a2l : no module chosen !");
                        return;
                    }
                }

                list2 = ((MODULE*)wp2->a2lFile->getProject()->getNode("MODULE/" + moduleName2))->listChar;
            }
        }

        //Missing labels
        QStringList missingLabels;
        foreach (QString str, list1)
        {
            //QStringList::iterator i = qBinaryFind(list2.begin(), list2.end(), str);
            QStringList::iterator i = std::lower_bound(list2.begin(), list2.end(), str);
            if (i == list2.end())
            {
                missingLabels.append(str);
            }
            else
            {
                if (str.compare(*i) != 0)
                {
                    missingLabels.append(str);
                }
            }
        }

        //New labels
        QStringList newLabels;        
        foreach (QString str, list2)
        {
            //QStringList::iterator i = qBinaryFind(list1.begin(), list1.end(), str);
            QStringList::iterator i = std::lower_bound(list1.begin(), list1.end(), str);
            //if (i == list1.end())
            if (i == list1.end())
            {
                newLabels.append(str);
            }
            else
            {
                if (str.compare(*i) != 0)
                {
                    newLabels.append(str);
                }
            }
        }
        qDebug() << "list labels 1 : " << list1.count();
        qDebug() << "list labels 2 : " << list2.count();
        qDebug() << "missing into 1: " << missingLabels.count();
        qDebug() << "new labels into 2: " << newLabels.count();

        //Missing, new, modified subsets
        Node *fun1 = wp1->a2lFile->getProject()->getNode("MODULE/" + moduleName1 + "/FUNCTION");
        Node *fun2 = wp2->a2lFile->getProject()->getNode("MODULE/" + moduleName2 + "/FUNCTION");
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
                //get Node into listSubsets2
                Node *subset2 = fun2->getNode(subset1->name);
                if (subset2)
                {

                    //get the label list from subset1
                    QStringList listChar_subset1;
                    DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)subset1->getNode("DEF_CHARACTERISTIC");
                    REF_CHARACTERISTIC *ref_char = (REF_CHARACTERISTIC*)subset1->getNode("REF_CHARACTERISTIC");
                    if (def_char)
                        listChar_subset1 = def_char->getCharList();
                    else if (ref_char)
                        listChar_subset1 = ref_char->getCharList();
                    else
                    {
                        Node *group = wp1->a2lFile->getProject()->getNode("MODULE/" + moduleName1 + "/GROUP");
                        if (group)
                        {
                            GGROUP *grp = (GGROUP*)group->getNode(subset1->name);
                            if (grp)
                            {
                                DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)grp->getNode("DEF_CHARACTERISTIC");
                                REF_CHARACTERISTIC *ref_char = (REF_CHARACTERISTIC*)grp->getNode("REF_CHARACTERISTIC");
                                if (def_char)
                                    listChar_subset1 = def_char->getCharList();
                                else if (ref_char)
                                    listChar_subset1 = ref_char->getCharList();
                            }
                        }
                    }


                    //get the label list from subset2
                    QStringList listChar_subset2;
                    def_char = (DEF_CHARACTERISTIC*)subset2->getNode("DEF_CHARACTERISTIC");
                    ref_char = (REF_CHARACTERISTIC*)subset2->getNode("REF_CHARACTERISTIC");
                    if (def_char)
                        listChar_subset2 = def_char->getCharList();
                    else if (ref_char)
                        listChar_subset2 = ref_char->getCharList();
                    else
                    {
                        Node *group = wp2->a2lFile->getProject()->getNode("MODULE/" + moduleName2 + "/GROUP");
                        if (group)
                        {
                            GGROUP *grp = (GGROUP*)group->getNode(subset1->name);
                            if (grp)
                            {
                                DEF_CHARACTERISTIC *def_char = (DEF_CHARACTERISTIC*)grp->getNode("DEF_CHARACTERISTIC");
                                REF_CHARACTERISTIC *ref_char = (REF_CHARACTERISTIC*)grp->getNode("REF_CHARACTERISTIC");
                                if (def_char)
                                    listChar_subset2 = def_char->getCharList();
                                else if (ref_char)
                                    listChar_subset2 = ref_char->getCharList();
                            }
                        }
                    }


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

void MDImain::readA2l(WorkProject* wp)
{

    //get a pointer on old a2lFile
    A2LFILE *a2lfile = wp->a2lFile;
    Node* parentNode = wp->a2lFile->getParentNode();

    // display status bar
    statusBar()->show();
    progBar->reset();
    connect(wp, SIGNAL(incProgressBar(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

    // parse the a2l file
    wp->parse();
    //wp->attach(this); alredy attached

    // hide the statusbar
    statusBar()->hide();
    progBar->reset();

    //add to treemodel with previously created childnodes
    foreach(Node* node,a2lfile->childNodes)
    {
        wp->a2lFile->addChildNode(node);
        node->setParentNode(wp->a2lFile);
        a2lfile->removeChildNode(node);
    }
    wp->a2lFile->sortChildrensName();

    //update model
    model->beginReset();
    parentNode->removeChildNode(a2lfile);
    parentNode->addChildNode(wp->a2lFile);
    parentNode->sortChildrensName();
    wp->a2lFile->setParentNode(parentNode);
    model->endReset();

    //expand parsed a2l
    ui->treeView->setExpanded(model->getIndex(parentNode), true);

    QModelIndex index = model->getIndex(wp->a2lFile);
    ui->treeView->setExpanded(index, true);

    //finally delete old a2lfile
    delete a2lfile;
}

HexFile* MDImain::readHexFile(HexFile *hex)
{
    //start timer
    double ti = omp_get_wtime();

    //get a pointer on old hex
    HexFile* oldHex = hex;

    //read hex file if not read
    QString fullName = hex->fullName();

    //get parent WP
    WorkProject *wp = hex->getParentWp();

    //if the a2lFile is not yet parsed, parse.
    if (!wp->a2lFile->isParsed())
    {
        readA2l(wp);
    }

    //get the index of the orginal hex node in treeview
    QModelIndex index = model->getIndex(hex);

    //read hex file
    QList<MODULE*> list = wp->a2lFile->getProject()->listModule();
    if (list.count() == 0)
    {
        writeOutput("action open new dataset : no Module into A2l file !");
        return 0;
    }
    else if (list.count() == 1)
    {
        //delete hex;
        hex = new HexFile(fullName, wp, QString(list.at(0)->name));
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
            hex = new HexFile(fullName, wp, module);
        }
        else
        {
            writeOutput("action open new dataset : no module chosen !");
            return 0;
        }
    }

    // display status bar
    statusBar()->show();
    progBar->reset();
    connect(hex, SIGNAL(progress(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

    if (hex->read())
    {
        //remove original hex file from node/tree
        deleteFileFromProject(index);
        //add new hex node in WP
        wp->addHex(hex);
    }
    else
        delete hex;

    // hide the statusbar
    statusBar()->hide();
    progBar->reset();

    //stop timer
    double tf = omp_get_wtime();

    //update the treeView model
    QModelIndex indexNew = model->getIndex(hex);
    ui->treeView->expand(indexNew.parent().parent());
    ui->treeView->expand(indexNew.parent());
    ui->treeView->resizeColumnToContents(0);

    writeOutput("action open new dataset : HEX file add to project in " + QString::number(tf-ti) + " sec");

    return hex;
}

SrecFile* MDImain::readSrecFile(SrecFile* srec)
{
    //start timer
    double ti = omp_get_wtime();

    QString fullName = srec->fullName();

    //get parent WP
    WorkProject *wp = srec->getParentWp();

    //if the a2lFile is not yet parsed, parse.
    if (!wp->a2lFile->isParsed())
    {
        readA2l(wp);
    }

    //get the index of the orginal hex node in treeview
    QModelIndex index = model->getIndex(srec);

    //read hex file
    QList<MODULE*> list = wp->a2lFile->getProject()->listModule();
    if (list.count() == 0)
    {
        writeOutput("action open new dataset : no Module into A2l file !");
        return 0;
    }
    else if (list.count() == 1)
    {
        srec = new SrecFile(fullName, wp, QString(list.at(0)->name));
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
            srec = new SrecFile(fullName, wp, module);
        }
        else
        {
            writeOutput("action open new dataset : no module chosen !");
            return 0;
        }
    }

    // display status bar
    statusBar()->show();
    progBar->reset();
    connect(srec, SIGNAL(progress(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

    if (srec->read())
    {
        //remove original srec file from node/tree
        deleteFileFromProject(index);
        //add new hex node in WP
        wp->addSrec(srec);
    }
    else
        delete srec;

    // hide the statusbar
    statusBar()->hide();
    progBar->reset();

    //stop timer
    double tf = omp_get_wtime();

    //update the treeView model
    QModelIndex indexNew = model->getIndex(srec);
    ui->treeView->expand(indexNew.parent().parent());
    ui->treeView->expand(indexNew.parent());
    ui->treeView->resizeColumnToContents(0);

    writeOutput("action open new dataset : HEX file add to project in " + QString::number(tf-ti) + " sec");

    return srec;
}

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
    if (name.endsWith("SrecFile"))
    {
        //As the selected node is an Srec file we can cast the node into its real type : SrecFile
        SrecFile *srec = dynamic_cast<SrecFile *> (node);

        //call sort by subset
        if (srec->isSortedBySubsets())
            srec->sortModifiedDataBySubset(false);
        else
            srec->sortModifiedDataBySubset(true);
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

        if (name.endsWith("HexFile"))
        {
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

                QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
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
        else if (name.endsWith("SrecFile"))
        {
            int row = index.row();
            if ( row < 0)
            {
                QMessageBox::information(this,"HEXplorer","please first select a project");
                writeOutput("action open new dataset cancelled: no project first selected");
                return;
            }
            else
            {
                SrecFile *srec  = (SrecFile*)model->getNode(index);

                QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
                        QString currentCsvPath = QFileInfo(fullCsvName).absolutePath();
                        settings.setValue("currentCsvPath", currentCsvPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project
                        Csv *csv = new Csv(fullCsvName, wp, srec->getModuleName());

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
                                Data* data2 = srec->getData(data1->name);

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
        else
        {
            QMessageBox::warning(this, "HEXplorer::read csv file values", "Please select first an hex or Srec file.",
                                             QMessageBox::Ok);
            return;
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

        if (name.endsWith("HexFile"))
        {
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

                QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
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
        else if (name.endsWith("SrecFile"))
        {
            int row = index.row();
            if ( row < 0)
            {
                QMessageBox::information(this,"HEXplorer","please first select an Srec file");
                writeOutput("action read CDFX file cancelled: no Srec file selected");
                return;
            }
            else
            {
                SrecFile *srec  = (SrecFile*)model->getNode(index);

                QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                        //if the selected srec file is already into the project => exit
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
                        QSettings settings(qApp->organizationName(), qApp->applicationName());
                        QString currentCdfxPath = QFileInfo(fullCdfxName).absolutePath();
                        settings.setValue("currentCdfxPath", currentCdfxPath);

                        //start a timer
                        double ti = omp_get_wtime();

                        //add the file to the project
                        CdfxFile *cdfx = new CdfxFile(fullCdfxName, wp, srec->getModuleName());
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
                                Data* data2 = srec->getData(data1->name);

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
        else
        {
            QMessageBox::warning(this, "HEXplorer::read cdfx values", "Please select first an Hex or an Srec file.",
                                             QMessageBox::Ok);
            return;
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
    else if (name.endsWith("SrecFile"))
    {
        //As the selected node is an Srec file we can cast the node into its real type : SrecFile
        SrecFile *srec = dynamic_cast<SrecFile *> (node);

        //reset all labels;
        srec->resetAllModifiedData();
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
    else if (name.endsWith("SrecFile"))
    {
        return save_SrecFile(index);
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

bool MDImain::save_SrecFile(QModelIndex index)
{
    double ti = omp_get_wtime();

    SrecFile *srec = (SrecFile*)model->getNode(index);

    int ret = QMessageBox::question(0, "HEXplorer :: save Srec file",
                          "overwrite " + srec->fullName() + " ?", QMessageBox::Yes, QMessageBox::Cancel);

    if (ret == QMessageBox::Yes)
    {
        // display status bar
        statusBar()->show();
        progBar->reset();

        // write all
        QStringList list = srec->writeHex();

        if (list.isEmpty())
        {
            writeOutput("action save dataset : cancelled");
            return false;
        }

        QString fileName = srec->fullName();
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
    else if (name.endsWith("SrecFile"))
    {
        return saveAs_SrecFile(index);
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
        QMessageBox::warning(this, "HEXplorer::save as file", "Please select first an hex, Srec, Csv or or Cdfx file.",
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

    if (hex->getParentWp()->containsHex(fileName))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("file %1 is already open.\nFirst close the open version or enter another file name.")
                             .arg(fileName));
        return;
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

void MDImain::saveAs_SrecFile(QModelIndex index)
{
    // get the HexFile Node
    SrecFile *Srec = (SrecFile*)model->getNode(index);

    // ask for new HexFile name
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QFileInfo(Srec->fullName()).fileName(),
                                                    QFileInfo(Srec->fullName()).absolutePath(),
                                                    tr("Srec files (*.s19);;all files (*.*)"));
    if (fileName.isEmpty())
    {
        writeOutput("action save dataset : cancelled");
        return;
    }
    if ( QFileInfo(fileName).suffix().toLower() != "s19" &&
         QFileInfo(fileName).suffix().toLower() != "s32")
    {
        fileName.append(".s19");
    }

    //check if the file is already open in HEXplorer
    if (Srec->getParentWp()->containsSrec(fileName))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("file %1 is already open.\nFirst close the open version or enter another file name.")
                             .arg(fileName));
        return;
    }

    // display status bar
    statusBar()->show();
    progBar->reset();

    // write the file
    QStringList list = Srec->writeHex();
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
    Srec->setFullName(fileName);

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

    //check if the file is already open in HEXplorer
    if (csv->getParentWp()->containsCsv(fileName))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("file %1 is already open.\nFirst close the open version or enter another file name.")
                             .arg(fileName));
        return;
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

    //check if the file is already open in HEXplorer
    if (cdfx->getParentWp()->containsCdfx(fileName))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("file %1 is already open.\nFirst close the open version or enter another file name.")
                             .arg(fileName));
        return;
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

        //get node names
        QString str1 = model->getFullNodeName(list.at(0));
        QString str2 = model->getFullNodeName(list.at(1));

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);
        fComp->setDataset2(str2);
        fComp->on_compare_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);
    }
}

void MDImain::quicklookFile()
{
    //get hexFiles path
    QModelIndex index  = ui->treeView->selectionModel()->currentIndex();
    QString str1 = model->getFullNodeName(index);

    if (index.isValid())
    {
        //get a pointer on the selected item
        Node *node =  model->getNode(index);

        //open DataContainer for quicklook
        QString name = typeid(*node).name();

        //check if file and a2l are read
        if (name.endsWith("HexFile"))
        {
            //read hex file if not read
            HexFile *hex = dynamic_cast<HexFile*>(node);          
            if (!hex->isRead())
            {
                readHexFile(hex);
            }
        }
        else if (name.endsWith("SrecFile"))
        {
            //read hex file if not read
            SrecFile *srec = dynamic_cast<SrecFile*>(node);

            if (!srec->isRead())
            {
                readSrecFile(srec);
            }
        }


        if (name.endsWith("HexFile") || name.endsWith("SrecFile") || name.endsWith("Csv") || name.endsWith("CdfxFile"))
        {

            //create a new FormCompare
            FormCompare *fComp = on_actionCompare_dataset_triggered();
            fComp->setDataset1(str1);

            //perform a quicklook
            fComp->on_quicklook_clicked();

            //set new FormCompare as activated
            ui->tabWidget->setCurrentWidget(fComp);
        }
        else
        {
            QMessageBox::warning(this, "HEXplorer::quicklook", "Please select first an hex,Srec Cdfx or csv file.",
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
    }
    else if (name.endsWith("SrecFile"))
    {
        SrecFile *srec = (SrecFile*)model->getNode(index);
        QString str1 = model->getFullNodeName(index);

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str1);

        // set the list to edit
        fComp->charList.clear();
        foreach(Data *data, srec->getModifiedData())
        {
            fComp->charList.append(data->name);
        }
        fComp->on_quicklook_clicked();

        //set new FormCompare as activated
        ui->tabWidget->setCurrentWidget(fComp);

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
        Data *data = (Data*)this->model->getNode(index);

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
        else if (data->getSrecParent())
            parentName = data->getSrecParent()->name;
        else if (data->getCsvParent())
            parentName = data->getCsvParent()->name;
        else if (data->getCdfxParent())
            parentName = data->getCdfxParent()->name;
        win->setWindowTitle("HEXplorer :: " + QString(data->name) + " (" + parentName + ")");

        win->show();
        win->resize(600, 400);

    }
}

void MDImain::editCompare()
{
    QModelIndex index = ui->treeView_2->selectionModel()->selectedIndexes().at(0);

    //get the data name
    Node *node = (Node*)((A2lTreeModel*)ui->treeView_2->model())->getNode(index);
    qDebug() << node->name;
    qDebug() << myWidget;

    QString name = typeid(*myWidget).name();
    if (name.toLower().endsWith("formcompare"))
    {
        FormCompare *fComp = (FormCompare*)myWidget;

        //get the DataContainer
        Data *data1 = 0;
        Data *data2 = 0;
        SrecFile *_srec1 = fComp->getSrec1();
        SrecFile *_srec2 = fComp->getSrec2();
        HexFile *hex1 = fComp->getHex1();
        HexFile *hex2 = fComp->getHex2();
        Csv *csv1 = fComp->getCsv1();
        Csv *csv2 = fComp->getCsv2();
        CdfxFile *cdfx1 = fComp->getCdf1();
        CdfxFile *cdfx2 = fComp->getCdf2();

        if (_srec1)
            data1 = _srec1->getData(node->name);
        else if (hex1)
            data1 = hex1->getData(node->name);
        else if (csv1)
            data1 = csv1->getData(node->name);
        else if (cdfx1)
            data1 = cdfx1->getData(node->name);

        if (_srec2)
            data2 = _srec2->getData(node->name);
        else if (hex2)
            data2 = hex2->getData(node->name);
        else if (csv2)
            data2 = csv2->getData(node->name);
        else if (cdfx2)
            data2 = cdfx2->getData(node->name);

        if (data1 && data2)
        {
            QMainWindow *win = new QMainWindow(0);
            SpreadsheetView *view = new SpreadsheetView(win);

            CompareModel *model = new CompareModel();
            QList<Data*> *list1 = new QList<Data*>;
            QList<Data*> *list2 = new QList<Data*>;
            list1->append(data1);
            list2->append(data2);
            model->setList(list1, list2);
            view->setModel(model);
            view->horizontalHeader()->setDefaultSectionSize(50);
            view->verticalHeader()->setDefaultSectionSize(18);
            view->setColumnWidth(0, 200);
            win->setCentralWidget(view);

            QString parentName1 = "";
            if (data1->getHexParent())
            {
                parentName1 = data1->getHexParent()->name;
            }
            else if (data1->getSrecParent())
            {
                parentName1 = data1->getSrecParent()->name;
            }
            else if (data1->getCsvParent())
            {
                parentName1 = data1->getCsvParent()->name;
            }
            else if (data1->getCdfxParent())
            {
                parentName1 = data1->getCdfxParent()->name;
            }

            QString parentName2 = "";
            if (data2->getHexParent())
            {
                parentName2 = data2->getHexParent()->name;
            }
            else if (data2->getSrecParent())
            {
                parentName2 = data2->getSrecParent()->name;
            }
            else if (data2->getCsvParent())
            {
                parentName2 = data2->getCsvParent()->name;
            }
            else if (data2->getCdfxParent())
            {
                parentName2 = data2->getCdfxParent()->name;
            }
            win->setWindowTitle("HEXplorer :: " + parentName1 + " vs " + parentName2);

            win->show();
            win->resize(600, 400);
        }
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

    ui->Differentlabels_Dock->setVisible(true);

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

        if (name.endsWith("HexFile"))
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
        else if (name.endsWith("SrecFile"))
        {
            SrecFile *srec  = (SrecFile*)model->getNode(index);
            A2LFILE *a2l = srec->getParentWp()->a2lFile;

            //choose subset(s)
            QStringList subsetList;
            ChooseSubset *chooseSubset = new ChooseSubset(a2l, srec, subsetList, this);
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
                connect(srec, SIGNAL(lineParsed(int,int)), this, SLOT(setValueProgressBar(int,int)), Qt::DirectConnection);

                //export
                if (exportFormat == "csv")
                    srec->exportSubsetList2Csv(subsetList);
                else if (exportFormat == "cdf")
                    srec->exportSubsetList2Cdf(subsetList);


                // hide the statusbar
                statusBar()->hide();
                progBar->reset();
            }
        }

        else
        {

            QMessageBox::warning(this, "HEXplorer::export subsets from Hex file", "Please select first an hex file.",
                                             QMessageBox::Ok);
            return;


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

            QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                    QSettings settings(qApp->organizationName(), qApp->applicationName());
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
                       // Csv *csv = list.value(str);

                       // bool missingLabels = false;
                       // bool newLabels = false;
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

    //delete QScintilla pointer
    delete tabList->value(_name);

    //remove widget from tabList
    tabList->remove(_name);

    if (tabList->count() == 0)
    {
        ui->Differentlabels_Dock->hide();
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

void MDImain::tabWidget_currentChanged(int index)
{
    myWidget = this->ui->tabWidget->currentWidget();
    if (myWidget)
    {
        QString name = typeid(*myWidget).name();
        if (name.toLower().endsWith("formcompare"))
        {
            FormCompare *fComp = (FormCompare*)myWidget;
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

    QSettings settings(qApp->organizationName(), qApp->applicationName());
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
    QSettings settings(qApp->organizationName(), qApp->applicationName());
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
    //remove the old installer file used for previous Update
    QFile::remove(qApp->applicationDirPath() + "/update_HEXplorer.exe");

    QUrl url("http://hexplorer.googlecode.com/svn/trunk/src/update.xml");
    DialogHttpUpdate updater(url, true, this);
}

void MDImain::initCheckHttpUpdates()
{
   //remove the old installer file used for previous Update
   QFile::remove(qApp->applicationDirPath() + "/update_HEXplorer.exe");

   QUrl url("http://hexplorer.googlecode.com/svn/trunk/src/update.xml");
   DialogHttpUpdate updater(url, false, this);
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
        //check if hex is read
        HexFile* hex = dynamic_cast<HexFile*>(node);
        if (!hex->isRead())
        {
            readHexFile(hex);
        }

        //create a new FormCompare
        FormCompare *fComp = on_actionCompare_dataset_triggered();
        fComp->setDataset1(str);
        fComp->on_quicklook_clicked();

    }
    else if (name.toLower().endsWith("srecfile"))
    {
        //check if srec is read
        SrecFile* srec = dynamic_cast<SrecFile*>(node);
        if (!srec->isRead())
        {
            readSrecFile(srec);
        }

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
    #ifdef Q_OS_WIN32
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
        return "";

}

//-------------------- DataBase ---------------------//

void MDImain::exportA2lDb()
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
            //start a timer;
            QTime timer;
            timer.start();

            //Get the selected file name
            QString fullFileName = model->name(index);

            //create a pointer on the WorkProject
            WorkProject *wp = projectList->value(fullFileName);

            // get the list of CHARACTERISTIC nodes            
            Node *dim;
            Node *module = wp->a2lFile->getProject()->getNode("MODULE");
            if (module == NULL)
            {
                return;
            }
            else
            {
                QString _module;
                QList<MODULE*> listModule = wp->a2lFile->getProject()->listModule();
                if (listModule.count() == 0)
                {
                    writeOutput("action edit characteristics : no Module into A2l file !");
                    return;
                }
                else if (listModule.count() == 1)
                {
                    _module = QString(listModule.at(0)->name);
                }
                else
                {
                    // select a module
                    QString module;
                    DialogChooseModule *diag = new DialogChooseModule(&module);
                    QStringList listModuleName;
                    foreach (MODULE* module, listModule)
                    {
                        listModuleName.append(module->name);
                    }
                    diag->setList(listModuleName);
                    int ret = diag->exec();

                    if (ret == QDialog::Accepted)
                    {
                         _module = module;
                    }
                    else
                    {
                        writeOutput("action open new dataset : no module chosen !");
                        return;
                    }
                }

                dim = module->getNode(_module);
                if (dim == NULL)
                {
                    return;
                }
            }

            //open DB
            if (!QSqlDatabase::drivers().contains("QSQLITE"))
            {
                QMessageBox::critical(this, "Unable to load database", "SQL driver missing");
                return;
            }
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(":memory:");
            if (!db.open())
            {
                qWarning("Could not access database '%s'\n", qPrintable(":memory"));
                return;
            }

            //create tables into DB
            createDbTableCharacteristics(dim);
            createDbTableAxisPts(dim);
            createDbTableCompuMethod(dim);
            createDbTableCompuVtab(dim);
            createDbTableRecordLayout(dim);

            statusBar()->hide();

            //write output
            writeOutput("characteristics saved into DB in " + QString::number(timer.elapsed() / 1000) +  " sec.");

            //persist database
            int ret = QMessageBox::question(this, "HEXplorer :: save database on disk",
                                  "Save database on disk?",
                                  QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

            if (ret == QMessageBox::Yes)
            {
                QSettings settings(qApp->organizationName(), qApp->applicationName());
                QString currentDbPath = settings.value("currentDbPath").toString();
                QString fileName = QFileDialog::getSaveFileName(this,"save database file", currentDbPath,
                                                                "DB files (*.db);;all files (*.*)");
                persistDB(db, fileName, 1);
                currentDbPath = QFileInfo(fileName).absolutePath();
                settings.setValue("currentDbPath", currentDbPath);

            }
        }
    }
}

bool MDImain::persistDB(QSqlDatabase memdb, QString filename, bool save)
{

    bool state = false;

    QVariant v = memdb.driver()->handle();
    if( v.isValid() && qstrcmp(v.typeName(),"sqlite3*") == 0 )
    {
        // v.data() returns a pointer to the handle
        sqlite3 * handle = *static_cast<sqlite3 **>(v.data());

        if( handle != 0 ) // check that it is not NULL
        {
            sqlite3 * pInMemory = handle;
            QByteArray array = filename.toLocal8Bit();
            const char * zFilename = array.data();
            int rc;                   /* Function return code */
            sqlite3 *pFile;           /* Database connection opened on zFilename */
            sqlite3_backup *pBackup;  /* Backup object used to copy data */
            sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
            sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

            /* Open the database file identified by zFilename. Exit early if this fails
            ** for any reason. */
            rc = sqlite3_open( zFilename, &pFile );

            if( rc == SQLITE_OK )
            {
              /* If this is a 'load' operation (isSave==0), then data is copied
              ** from the database file just opened to database pInMemory.
              ** Otherwise, if this is a 'save' operation (isSave==1), then data
              ** is copied from pInMemory to pFile.  Set the variables pFrom and
              ** pTo accordingly. */

              pFrom = ( save ? pInMemory : pFile);
              pTo   = ( save ? pFile     : pInMemory);

              /* Set up the backup procedure to copy from the "main" database of
              ** connection pFile to the main database of connection pInMemory.
              ** If something goes wrong, pBackup will be set to NULL and an error
              ** code and  message left in connection pTo.
              **
              ** If the backup object is successfully created, call backup_step()
              ** to copy data from pFile to pInMemory. Then call backup_finish()
              ** to release resources associated with the pBackup object.  If an
              ** error occurred, then  an error code and message will be left in
              ** connection pTo. If no error occurred, then the error code belonging
              ** to pTo is set to SQLITE_OK.
              */
              pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
              if( pBackup )
              {
                (void)sqlite3_backup_step(pBackup, -1);
                (void)sqlite3_backup_finish(pBackup);
              }
              rc = sqlite3_errcode(pTo);
            }

            /* Close the database connection opened on database file zFilename
            ** and return the result of this function. */
            (void)sqlite3_close(pFile);

            if( rc == SQLITE_OK ) state = true;

        }

    }

    return state;

}

void MDImain::on_actionLoad_DB_triggered()
{
    //select database file to open
    QSettings settings(qApp->organizationName(), qApp->applicationName());
    QString currentDbPath = settings.value("currentDbPath").toString();
    QString file = QFileDialog::getOpenFileName(this,
                                      tr("select .db files"), currentDbPath,
                                      tr(".db files (*.db)"));
    if (file.isEmpty())
        return;

    if (projectList->contains(file))
    {
        QMessageBox::information(this, "HEXplorer", file + " already open !");
        return;
    }

    //check if SQL driver is available
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        QMessageBox::critical(this, "Unable to load database", "SQL driver missing");
        return;
    }

    //create Sql connection
    //QSqlDatabase::database("in_mem_db", false).close();
    //QSqlDatabase::removeDatabase("in_mem_db");
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", QFileInfo(file).fileName());
    //db.setDatabaseName(":memory:");
    db.setDatabaseName(file);
    if (!db.open())
    {
        qWarning("Could not access database '%s'\n", qPrintable(":memory"));
        return;
    }

    settings.setValue("currentDbPath", QFileInfo(file).absolutePath());

    //load the database in memory
    //persistDB(db, file, 0);

    // create a new Wp
    WorkProject *wp = new WorkProject(file, this->model, this);

    // parse the a2l file
    wp->attach(this);

    //create an ASAP2 file Node to start parsing
    DBFILE *nodeDB = new DBFILE(0,QFileInfo(file).fileName(), file);
    nodeDB->name = new char[(QFileInfo(file).fileName()).toLocal8Bit().count() + 1];
    strcpy(nodeDB->name, QFileInfo(file).fileName().toLocal8Bit().data());
    wp->dbFile = nodeDB;

    //update the ui->treeView
    model->addNode2RootNode(wp->dbFile);
    ui->treeView->setModel(model);
    ui->treeView->setColumnHidden(1, true);

    //set completer
    completer->setModel(model);

    //insert the new created project into the projectList
    projectList->insert(file, wp);

    //update output console
    ui->listWidget->scrollToBottom();

    //display in connectWidget
    ui->connectionWidget->refresh();
}

void MDImain::on_connectionWidget_tableActivated(const QString &table)
{
    //open current connection
    QSqlDatabase db = ui->connectionWidget->currentDatabase();

    //create a model to edit the database
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable(table);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    while (model->canFetchMore())
        model->fetchMore();

    //create a new spreadSheet
    SpreadsheetView *view = new SpreadsheetView();
    view->setModel(model);
    view->setAlternatingRowColors(true);
    view->hideColumn(0); // don't show the ID

    //add a new tab with the spreadsheet
    QIcon icon;
    icon.addFile(":/icones/milky_exportBaril.png");
    ui->tabWidget->addTab(view, icon, db.connectionName() + "::" + table);

    //set new FormCompare as activated
    ui->tabWidget->setCurrentWidget(view);
}

void MDImain::createDbTableCharacteristics(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q_Char;
    q_Char.exec(QLatin1String("create table CHARACTERISTICS(id integer primary key, "
                         "Name varchar, "
                         "LongIdentifier varchar, "
                         "Unit varchar, "
                         "Fonction varchar, "
                         "Adress varchar, "
                         "Type varchar, "
                         "Conversion varchar, "
                         "Deposit varchar, "
                         "MaxDiff varchar, "
                         "LowerLimit varchar, "
                         "UpperLimit varchar)"));

    //populates table characteristics
    q_Char.prepare(QLatin1String("insert into CHARACTERISTICS("
                            "Name, LongIdentifier, Unit, Fonction, "
                            "Adress, Type, Conversion, "
                            "Deposit, MaxDiff, LowerLimit, "
                            "UpperLimit) values(?,?,?,?,?,?,?,?,?,?,?)"));

    //create Table AXIS_DESCR_X into db.
    QSqlQuery q_AxisX;
    q_AxisX.exec(QLatin1String("create table AXIS_DESCR_X(id integer primary key, "
                         "Name varchar, "
                         "Attribute varchar, "
                         "InputQuantity varchar, "
                         "Conversion varchar, "
                         "MaxAxisPoints varchar, "
                         "LowerLimit varchar, "
                         "UpperLimit varchar)"));

    //populates table axis_pts
    q_AxisX.prepare(QLatin1String("insert into AXIS_DESCR_X("
                            "Name, Attribute, InputQuantity, Conversion, "
                            "MaxAxisPoints, LowerLimit, "
                            "UpperLimit) values(?,?,?,?,?,?,?)"));

    //create Table AXIS_DESCR_Y into db.
    QSqlQuery q_AxisY;
    q_AxisY.exec(QLatin1String("create table AXIS_DESCR_Y(id integer primary key, "
                         "Name varchar, "
                         "Attribute varchar, "
                         "InputQuantity varchar, "
                         "Conversion varchar, "
                         "MaxAxisPoints varchar, "
                         "LowerLimit varchar, "
                         "UpperLimit varchar)"));

    //populates table axis_pts
    q_AxisY.prepare(QLatin1String("insert into AXIS_DESCR_Y("
                            "Name, Attribute, InputQuantity, Conversion, "
                            "MaxAxisPoints, LowerLimit, "
                            "UpperLimit) values(?,?,?,?,?,?,?)"));

    QList<Node*> list;
    Node *charact = dim->getNode("CHARACTERISTIC");
    if (charact)
    {
        list = charact->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);
    statusBar()->show();

    for (int i = 0; i < list.count(); i++)
    {
        CHARACTERISTIC* myChar = (CHARACTERISTIC*)list.at(i);
        QString compu_method = myChar->getPar("Conversion");
        Node * node = myChar->getParentNode()->getParentNode();
        COMPU_METHOD *cmp = (COMPU_METHOD*)node->getNode("COMPU_METHOD/" + compu_method);

        q_Char.addBindValue(QLatin1String(myChar->name));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("LongIdentifier").c_str()));
        q_Char.addBindValue(QLatin1String(cmp->fixPar("Unit").c_str()));
        q_Char.addBindValue(myChar->getSubsetName());
        q_Char.addBindValue(QLatin1String(myChar->fixPar("Adress").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("Type").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("Conversion").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("Deposit").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("MaxDiff").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("LowerLimit").c_str()));
        q_Char.addBindValue(QLatin1String(myChar->fixPar("UpperLimit").c_str()));
        q_Char.exec();

        //AXIS_DESCR_X
        Node* nodeA = myChar->getNode("AXIS_DESCR");
        if (nodeA)
        {
            for (int i = 0; i < nodeA->childCount(); i++)
            {
                AXIS_DESCR* axis = (AXIS_DESCR*)nodeA->child(i);
                if (axis && i == 0)
                {
                    q_AxisX.addBindValue(QLatin1String(myChar->name));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("Attribute").c_str()));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("InputQuantity").c_str()));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("Conversion").c_str()));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("MaxAxisPoints").c_str()));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("LowerLimit").c_str()));
                    q_AxisX.addBindValue(QLatin1String(axis->fixPar("UpperLimit").c_str()));
                    q_AxisX.exec();
                }

                if (axis && i == 1)
                {
                    q_AxisY.addBindValue(QLatin1String(myChar->name));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("Attribute").c_str()));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("InputQuantity").c_str()));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("Conversion").c_str()));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("MaxAxisPoints").c_str()));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("LowerLimit").c_str()));
                    q_AxisY.addBindValue(QLatin1String(axis->fixPar("UpperLimit").c_str()));
                    q_AxisY.exec();
                }
            }
        }

        //AXIS_DESCR_Y


        progBar->setValue(i+1);
    }
}

void MDImain::createDbTableAxisPts(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q;
    q.exec(QLatin1String("create table AXIS_PTS(id integer primary key, "
                         "Name varchar, "
                         "LongIdentifier varchar, "
                         "Adress varchar, "
                         "InputQuantity varchar, "
                         "Deposit varchar, "
                         "Maxdiff varchar, "
                         "Conversion varchar, "
                         "MaxAxisPoints varchar, "
                         "LowerLimit varchar, "
                         "UpperLimit varchar)"));

    //populates table axis_pts
    q.prepare(QLatin1String("insert into AXIS_PTS("
                            "Name, LongIdentifier, Adress, InputQuantity, "
                            "Deposit, Maxdiff, Conversion, "
                            "MaxAxisPoints, LowerLimit, "
                            "UpperLimit) values(?,?,?,?,?,?,?,?,?,?)"));

    QList<Node*> list;
    Node *axisPts = dim->getNode("AXIS_PTS");
    if (axisPts)
    {
        list = axisPts->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);

    for (int i = 0; i < list.count(); i++)
    {
        AXIS_PTS* myNode = (AXIS_PTS*)list.at(i);
        q.addBindValue(QLatin1String(myNode->name));
        q.addBindValue(QLatin1String(myNode->fixPar("LongIdentifier").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Adress").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("InputQuantity").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Deposit").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("MaxDiff").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Conversion").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("MaxAxisPoints").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("LowerLimit").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("UpperLimit").c_str()));
        q.exec();
        progBar->setValue(i+1);
    }
}

void MDImain::createDbTableCompuMethod(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q;
    q.exec(QLatin1String("create table COMPU_METHOD(id integer primary key, "
                         "Name varchar, "
                         "LongIdentifier varchar, "
                         "ConversionType varchar, "
                         "Format varchar, "
                         "Unit varchar)"));

    //populates table axis_pts
    q.prepare(QLatin1String("insert into COMPU_METHOD("
                            "Name, LongIdentifier, ConversionType, "
                            "Format, Unit) values(?,?,?,?,?)"));

    QList<Node*> list;
    Node *compu = dim->getNode("COMPU_METHOD");
    if (compu)
    {
        list = compu->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);

    for (int i = 0; i < list.count(); i++)
    {
        COMPU_METHOD* myNode = (COMPU_METHOD*)list.at(i);
        q.addBindValue(QLatin1String(myNode->name));
        q.addBindValue(QLatin1String(myNode->fixPar("LongIdentifier").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("ConversionType").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Format").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Unit").c_str()));
        q.exec();
        progBar->setValue(i+1);
    }
}

void MDImain::createDbTableCompuVtab(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q;
    q.exec(QLatin1String("create table COMPU_VTAB(id integer primary key, "
                         "Name varchar, "
                         "LongIdentifier varchar, "
                         "ConversionType varchar, "
                         "NumberValuePairs varchar)"));

    //populates table axis_pts
    q.prepare(QLatin1String("insert into COMPU_VTAB("
                            "Name, LongIdentifier, ConversionType, "
                            "NumberValuePairs) values(?,?,?,?)"));

    QList<Node*> list;
    Node *compu = dim->getNode("COMPU_VTAB");
    if (compu)
    {
        list = compu->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);

    for (int i = 0; i < list.count(); i++)
    {
        COMPU_VTAB* myNode = (COMPU_VTAB*)list.at(i);
        q.addBindValue(QLatin1String(myNode->name));
        q.addBindValue(QLatin1String(myNode->fixPar("LongIdentifier").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("ConversionType").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("NumberValuePairs").c_str()));
        q.exec();
        progBar->setValue(i+1);
    }
}

void MDImain::createDbTableRecordLayout(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q;
    q.exec(QLatin1String("create table RECORD_LAYOUT(id integer primary key, "
                         "Name varchar)"));

    //populates table axis_pts
    q.prepare(QLatin1String("insert into RECORD_LAYOUT("
                            "Name) values(?)"));

    QList<Node*> list;
    Node *compu = dim->getNode("RECORD_LAYOUT");
    if (compu)
    {
        list = compu->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);

    for (int i = 0; i < list.count(); i++)
    {
        RECORD_LAYOUT* myNode = (RECORD_LAYOUT*)list.at(i);
        q.addBindValue(QLatin1String(myNode->name));
        //q.addBindValue(QLatin1String(myNode->fixPar("LongIdentifier").c_str()));
        q.exec();
        progBar->setValue(i+1);
    }
}

void MDImain::createDbTableAxisDescr(Node *dim)
{
    //create Table Charatericts into db.
    QSqlQuery q;
    q.exec(QLatin1String("create table AXIS_DESCR(id integer primary key, "
                         "Name varchar, "
                         "Attribute varchar, "
                         "InputQuantity varchar, "
                         "Conversion varchar, "
                         "MaxAxisPoints varchar, "
                         "LowerLimit varchar, "
                         "UpperLimit varchar)"));

    //populates table axis_pts
    q.prepare(QLatin1String("insert into AXIS_DESCR("
                            "Name, Attribute, InputQuantity, Conversion, "
                            "MaxAxisPoints, LowerLimit, "
                            "UpperLimit) values(?,?,?,?,?,?,?)"));

    QList<Node*> list;
    Node *axisPts = dim->getNode("AXIS_DESCR");
    if (axisPts)
    {
        list = axisPts->childNodes;
    }
    else return;

    progBar->reset();
    progBar->setMaximum(list.count());
    progBar->setValue(0);

    for (int i = 0; i < list.count(); i++)
    {
        AXIS_DESCR* myNode = (AXIS_DESCR*)list.at(i);
        q.addBindValue(QLatin1String(myNode->name));
        q.addBindValue(QLatin1String(myNode->fixPar("Attribute").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("InputQuantity").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("Conversion").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("MaxAxisPoints").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("LowerLimit").c_str()));
        q.addBindValue(QLatin1String(myNode->fixPar("UpperLimit").c_str()));
        q.exec();
        progBar->setValue(i+1);
    }
}
