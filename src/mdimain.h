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

#ifndef MDIMAIN_H
#define MDIMAIN_H

#include <QtGui/QMainWindow>
#include <QModelIndex>
#include <QProgressBar>
class Node;

class QsciLexerXML;
class QsciLexer;
class A2l;
class Hex;
class QFile;
class QTextStream;
class QTextEdit;
class ChTextEdit;
class TreeModelCompleter;
class A2lTreeModel;
class CompleterModel;
class WorkProject;
class FormCompare;


namespace Ui
{
    class MDImain;
}

class MDImain : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MDImain)

public:
     MDImain(QWidget *parent = 0);
     ~MDImain();

     void writeOutput(QString str);
     void writeOutput(QStringList list);
     void modelUpdate();
     void updateRecentFileActions();
     void expandNode(Node *node);
     void reAppendProject(WorkProject *wp);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent *e);

private:
    //bool boolCiao;
    QString getUserName();
    QString strippedName(const QString &fullFileName);
    enum { MaxRecentFiles = 5};
    QAction *recentFileActs[MaxRecentFiles];
    QAction *separatorAct;
    void createMenus();
    void readSettings();
    bool registerVersion();
    QProgressBar *progBar;
    QString curFile;
    void setCurrentFile(const QString &fileName);
    bool FileExists(std::string strFilename);
    void updateView();
    A2lTreeModel *model;
    CompleterModel *completerModel;
    TreeModelCompleter *completer;
    Ui::MDImain *ui;
    int compareTabs;
    QMap<QString, WorkProject*> *projectList;
    QMap<QString, QWidget*> *tabList;
    QAction *importSubsets;
    QAction *exportSubsets;
    QAction *openJScript;
    QAction *editMeasChannels;
    QAction *editCharacteristics;
    QAction *addCsvFile;
    QAction *addCdfxFile;
    QAction *addHexFile;
    QAction *addSrecFile;
    QAction *deleteFile;
    QAction *editFile;
    QAction *showParam;
    QAction *childCount;
    QAction *deleteProject;
    QAction *resetAllChangedData;
    QAction *sortBySubset;
    QAction *resetData;
    QAction *saveFile;
    QAction *saveAsFile;
    QAction *compareHexFile;
    QAction *compareA2lFile;
    QAction *quicklook;
    QAction *clearOutput;
    QAction *plotData;
    QAction *readValuesFromCsv;
    QAction *readValuesFromCdfx;
    QAction *verify;
    QAction *checkFmtc;
    QAction *editChanged;
    QAction *editLabel;
    QAction *exportListData;
    QMenu *toolsMenu;
    QMenu *recentProMenu;


    void initToolBars();
    void createActions();
    void openProject(QString &fileName);
    bool checkChangedHexFiles();
    bool checkChangedSrecFiles();
    bool checkChangedCsvFiles();
    bool checkChangedCdfxFiles();

private slots:
     void compare_A2lFile();
     void import_Subsets();
     void export_Subsets();
     void newFormScript();
     void setValueProgressBar(int n, int max);
     void checkDroppedFile(QString str);
     void editMeasuringChannels();
     void editChar();
     void on_actionOpen_Saphir_file_triggered();
     void on_actionSettings_triggered();
     void on_actionCheck_for_updates_triggered();
     void initCheckHttpUpdates();
     void openRecentFile();
     void read_ValuesFromCsv();
     void read_ValuesFromCdfx();
     void on_actionSave_session_triggered();
     void fplotData();
     void on_listWidget_customContextMenuRequested();
     void on_tabWidget_currentChanged(QWidget* );
     void quicklookFile();
     void compare_HexFile();
     bool save_File();
     bool save_HexFile(QModelIndex index);
     bool save_SrecFile(QModelIndex index);
     bool save_CsvFile(QModelIndex index);
     bool save_CdfxFile(QModelIndex index);
     void saveAs_File();
     void saveAs_HexFile(QModelIndex index);
     void saveAs_SrecFile(QModelIndex index);
     void saveAs_CsvFile(QModelIndex index);
     void saveAs_CdfxFile(QModelIndex index);
     void clear_Output();
     void reset_Data();
     void sort_BySubset();
     void clone_HexFile();
     void reset_AllChangedData();
     FormCompare *on_actionCompare_dataset_triggered();
     void countChild();
     void on_actionCreate_database_triggered();
     void on_actionAbout_triggered();
     void resizeColumn0();
     void showContextMenu(QPoint);
     void showContextMenu_2(QPoint);
     void on_treeView_clicked(QModelIndex index);
     void showFixPar();
     void nodeSelected();
     void addHexFile2Project();
     void addSrecFile2Project();
     void addCsvFile2Project();
     void addCdfxFile2Project();
     void deleteFilesFromProject();
     void deleteFileFromProject(QModelIndex index);
     void removeWorkProjects();
     void removeWorkProject(QModelIndex index);
     void export_ListData();
     void removeTab ( int index );
     void on_actionNewA2lProject_triggered();
     void on_actionExit_triggered();
     bool editTextFile(QFile &file, QsciLexer *lexer = 0);
     bool editTextFile(QString &text);
     void editTextFile();
     void verifyMaxTorque();
     void checkFmtcMonotony();
     void editChangedLabels();
     void edit();
     void exception(int, const QString &, const QString &, const QString &);
     void doubleClicked(QModelIndex);

 public slots:
    void resetAllTableView();

signals:
    void check();
    //void adieu();

};

#endif // MDIMAIN_H
