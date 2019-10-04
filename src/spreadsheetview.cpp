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
#include <QTableWidgetSelectionRange>
#ifdef Q_WS_WIN32
    #include <QAxObject>
#endif
#include <bitset>
#include <typeinfo>

#include "spreadsheetview.h"
#include "sptablemodel.h"
#include "sptablemodelHex.h"
#include "comparemodel.h"
#include "graphmodel.h"
#include "obdMergeModel.h"
#include "comboboxdelegate.h"
#include "dialogeditastext.h"
#include "dialogbitdisplay.h"
#include "csv.h"
#include "cdfxfile.h"
#include "formcompare.h"
#include "measmodel.h"
#include "charmodel.h"
#include "dialogdatadimension.h"
#include "labelproperties.h"
#include "obdMergeModel.h"
#include "obdsortfilterproxymodel.h"
//#include "excel.h"


using namespace std;

SpreadsheetView::SpreadsheetView(QWidget *parent)
{
    createActions();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuEvent(QPoint)));
    setContextMenuPolicy(Qt::CustomContextMenu);
    setItemDelegate(new ComboBoxDelegate(this));

    QHeaderView *header = horizontalHeader();
    connect(header, SIGNAL(sectionResized(int,int,int)), this, SLOT(myResize(int,int,int)));

    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(updateActions(QModelIndex)));
}

SpreadsheetView::~SpreadsheetView()
{
}

void SpreadsheetView::createActions()
{

    export2Excel = new QAction(tr("&export to Excel"), this);
    #ifdef Q_WS_WIN32
        connect(export2Excel, SIGNAL(triggered()), this, SLOT(exportToExcel()));
    #endif
    export2Excel->setIcon(QIcon(":/icones/excel"));
    this->addAction(export2Excel);

    changeSize = new QAction(tr("&add/remove breakpoints"), this);
    connect(changeSize, SIGNAL(triggered()), this, SLOT(changeLabelSize()));
    this->addAction(changeSize);

    selectAllLabel = new QAction(tr("&Interpolate X axis"), this);
    selectAllLabel->setShortcut(tr("Ctrl+A"));
    connect(selectAllLabel, SIGNAL(triggered()), this, SLOT(selectAll_label()));
    this->addAction(selectAllLabel);

    axisXInterpolate = new QAction(tr("&Interpolate X axis"), this);
    connect(axisXInterpolate, SIGNAL(triggered()), this, SLOT(interpAxisX()));
    this->addAction(axisXInterpolate);

    axisYInterpolate = new QAction(tr("&Interpolate Y axis"), this);
    connect(axisYInterpolate, SIGNAL(triggered()), this, SLOT(interpAxisY()));
    this->addAction(axisYInterpolate);

    interpolateX = new QAction(tr("&Interpolate X"), this);
    interpolateX->setStatusTip(tr("interpolate the values in the X direction"));
    connect(interpolateX, SIGNAL(triggered()), this, SLOT(interpX()));
    this->addAction(interpolateX);

    interpolateY = new QAction(tr("&Interpolate Y"), this);
    interpolateY->setStatusTip(tr("interpolate the values in the Y direction"));
    connect(interpolateY, SIGNAL(triggered()), this, SLOT(interpY()));
    this->addAction(interpolateY);

    undoModif = new QAction(tr("&Undo"), this);
    undoModif->setShortcut(tr("Ctrl+Z"));
    undoModif->setIcon(QIcon(":/icones/milky_backward.png"));
    undoModif->setShortcutContext(Qt::WidgetShortcut);
    undoModif->setStatusTip(tr("undo the last change into the current selection's contents"));
    connect(undoModif, SIGNAL(triggered()), this, SLOT(undoM()));
    this->addAction(undoModif);

    resetModif = new QAction(tr("&Reset"), this);
    resetModif->setShortcut(tr("Ctrl+U"));
    resetModif->setIcon(QIcon(":/icones/milky_resetAll.png"));
    resetModif->setShortcutContext(Qt::WidgetShortcut);
    resetModif->setStatusTip(tr("reset the current selection's contents witht he original values"));
    connect(resetModif, SIGNAL(triggered()), this, SLOT(resetM()));
    this->addAction(resetModif);

    factorMulti = new QAction(tr("&Multiply by"), this);
    factorMulti->setShortcut(tr("*"));
    factorMulti->setShortcutContext(Qt::WidgetShortcut);
    factorMulti->setStatusTip(tr("Multiply the current selection's contents by the entered factor"));
    connect(factorMulti, SIGNAL(triggered()), this, SLOT(factorM()));
    this->addAction(factorMulti);

    factorDiv = new QAction(tr("&Divide by"), this);
    factorDiv->setShortcut(tr("/"));
    factorDiv->setShortcutContext(Qt::WidgetShortcut);
    factorDiv->setStatusTip(tr("divide the current selection's contents by the entered factor"));
    connect(factorDiv, SIGNAL(triggered()), this, SLOT(factorD()));
    this->addAction(factorDiv);

    offsetPlus = new QAction(tr("&Offset (+)"), this);
    offsetPlus->setShortcut(tr("+"));
    offsetPlus->setShortcutContext(Qt::WidgetShortcut);
    offsetPlus->setStatusTip(tr("add to the entered offset to the current selection's contents"));
    connect(offsetPlus, SIGNAL(triggered()), this, SLOT(offsetP()));
    this->addAction(offsetPlus);

    offsetMinus = new QAction(tr("&Offset (-)"), this);
    offsetMinus->setShortcut(tr("-"));
    offsetMinus->setShortcutContext(Qt::WidgetShortcut);
    offsetMinus->setStatusTip(tr("substract to the entered offset to the current selection's contents"));
    connect(offsetMinus, SIGNAL(triggered()), this, SLOT(offsetM()));
    this->addAction(offsetMinus);

    fillAllWith = new QAction(tr("&Fill all with"), this);
    fillAllWith->setShortcut(tr("="));
    fillAllWith->setShortcutContext(Qt::WidgetShortcut);
    fillAllWith->setStatusTip(tr("Fill the current selection's contents with the same entered value"));
    connect(fillAllWith, SIGNAL(triggered()), this, SLOT(fillAll()));
    this->addAction(fillAllWith);

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":/icones/copy.png"));
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    copyAction->setStatusTip(tr("Copy the current selection's contents "
                                "to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    this->addAction(copyAction);

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setIcon(QIcon(":/icones/paste.png"));
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    pasteAction->setStatusTip(tr("Paste the current selection's contents "));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    this->addAction(pasteAction);

    plotAction = new QAction(tr("&Graph"), this);
    plotAction->setShortcut(Qt::Key_F4);
    plotAction->setIcon(QIcon(":/icones/AXIS.bmp"));
    plotAction->setStatusTip(tr("plot the currebt selected label"));
    connect(plotAction, SIGNAL(triggered()), this, SLOT(plot()));
    this->addAction(plotAction);

    editText = new QAction(tr("&Text"), this);
    editText->setShortcut(Qt::CTRL+Qt::Key_T);
    editText->setIcon(QIcon(":/icones/Text-64.png"));
    editText->setShortcutContext(Qt::WidgetShortcut);
    connect(editText, SIGNAL(triggered()), this, SLOT(editAsText()));
    this->addAction(editText);

    editBit = new QAction(tr("&Bit"), this);
    editBit->setShortcut(Qt::CTRL+Qt::Key_B);
    editBit->setIcon(QIcon(":/icones/binary.png"));
    editBit->setShortcutContext(Qt::WidgetShortcut);
    connect(editBit, SIGNAL(triggered()), this, SLOT(editAsBit()));
    this->addAction(editBit);

    editHex = new QAction(tr("&Hex"), this);
    editHex->setShortcut(Qt::CTRL+Qt::Key_H);
    editHex->setIcon(QIcon(":/icones/binary.png"));
    editHex->setShortcutContext(Qt::WidgetShortcut);
    connect(editHex, SIGNAL(triggered()), this, SLOT(editAsHex()));
    this->addAction(editHex);

    editProperties = new QAction(tr("&Edit properties"), this);
    editProperties->setShortcut(Qt::CTRL+Qt::Key_I);
    //editProperties->setIcon(QIcon(":/icones/binary.png"));
    editProperties->setShortcutContext(Qt::WidgetShortcut);
    connect(editProperties, SIGNAL(triggered()), this, SLOT(editProp()));
    this->addAction(editProperties);

    findObdView = new QAction(tr("Find"), this);
    findObdView->setIcon(QIcon(":/icones/milky_find.png"));
    findObdView->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(findObdView, SIGNAL(triggered()), this, SLOT(findInObdView()));
    findObdView->setDisabled(false);

    filterColumns = new QAction(tr("Filter column"), this);
    filterColumns->setIcon(QIcon(":/icones/milky_loopPlus.png"));
    //filterColumns->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(filterColumns, SIGNAL(triggered()), this, SLOT(filterColumn()));
    filterColumns->setDisabled(false);

    resetAllFilters = new QAction(tr("reset all filters"), this);
    resetAllFilters->setIcon(QIcon(":/icones/milky_pinceau.png"));
    //resetAllFilters->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(resetAllFilters, SIGNAL(triggered()), this, SLOT(resetAll_Filters()));
    resetAllFilters->setDisabled(false);


}

void SpreadsheetView::contextMenuEvent ( QPoint p )
{
    QMenu *menu = new QMenu(this);
    QModelIndex index = indexAt(p);
    if (index.isValid())
    {
        //get the selected data
        Data *data = 0;
        QString name = typeid(*model()).name();
        if (name.toLower().endsWith("sptablemodel"))
        {
            SpTableModel *spModel = (SpTableModel*)model();
            data = spModel->getLabel(index, Qt::EditRole);
        }
        else if (name.toLower().endsWith("sptablemodelhex"))
        {
            SpTableModelHex *spModel = (SpTableModelHex*)model();
            data = spModel->getLabel(index, Qt::EditRole);
        }
        else if (name.toLower().endsWith("comparemodel"))
        {
            CompareModel *spModel = (CompareModel*)model();
            data = spModel->getLabel(index, Qt::EditRole);
        }             
            //return;

        // create the menus
        if (name.toLower().endsWith("comparemodel") ||
                 name.toLower().endsWith("sptablemodel"))
        {
            //menu->addAction(export2Excel);
            menu->addAction(copyAction);
            menu->addAction(pasteAction);
            menu->addAction(undoModif);
            menu->addAction(resetModif);
            menu->addSeparator();
            menu->addAction(fillAllWith);
            menu->addAction(offsetPlus);
            menu->addAction(offsetMinus);
            menu->addAction(factorMulti);
            menu->addAction(factorDiv);
            menu->addAction(interpolateX);
            menu->addAction(interpolateY);
            menu->addSeparator();
            menu->addAction(editProperties);

            //get type of data
            if (data)
            {
                changeSize->setEnabled(true);
                if (data->getAxisDescrX())
                {
                    QString typeAxisX = data->getAxisDescrX()->getPar("Attribute");
                    if (typeAxisX != "STD_AXIS")
                    {
                        changeSize->setEnabled(false);
                    }

                    if (data->getAxisDescrY())
                    {
                        QString typeAxisY = data->getAxisDescrY()->getPar("Attribute");
                        if (typeAxisY != "STD_AXIS")
                        {
                            changeSize->setEnabled(false);
                        }
                        else
                        {
                            changeSize->setEnabled(true);
                        }
                    }
                }
            }

            menu->addAction(changeSize);
            menu->addSeparator();

            QMenu  *toolsMenu1 = new QMenu();
            toolsMenu1->setTitle("display as ...");
            toolsMenu1->addAction(plotAction);
            toolsMenu1->addAction(editText);
            toolsMenu1->addAction(editBit);
            toolsMenu1->addAction(editHex);

            QMenu  *toolsMenu2 = new QMenu();
            toolsMenu2->setTitle("axis ...");
            toolsMenu2->addAction(axisXInterpolate);
            toolsMenu2->addAction(axisYInterpolate);

            menu->addMenu(toolsMenu1);
            menu->addMenu(toolsMenu2);
        }
        else if (name.toLower().endsWith("sptablemodelhex"))
        {
            menu->addAction(copyAction);
            menu->addAction(pasteAction);
            menu->addAction(undoModif);
            menu->addAction(resetModif);
            menu->addSeparator();
            menu->addAction(fillAllWith);
            menu->addSeparator();
            menu->addAction(changeSize);
        }
        else if (name.toLower().endsWith("measmodel") || name.toLower().endsWith("charmodel")
                 || name.toLower().endsWith("fandrmodel"))
        {
            menu->addAction(copyAction);
        }
        else if (name.toLower().endsWith("obdmergemodel")  || name.toLower().endsWith("obdsortfilterproxymodel") )
        {
            menu->addAction(fillAllWith);
            menu->addSeparator();
            menu->addAction(undoModif);
            menu->addAction(resetModif);
            menu->addSeparator();
            menu->addAction(findObdView);
            menu->addAction(copyAction);
            menu->addAction(pasteAction);
            menu->addSeparator();

            QMenu *menuFilter = new QMenu("Filter");
            menu->addMenu(menuFilter);
            obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
            QStringList uniqueList = proxyModel->getUniqueValues(index.column());
            foreach (QString value, uniqueList)
            {
                QAction *action = new QAction(value);
                menuFilter->addAction(action);
                connect(action, &QAction::triggered, this, [=]() { this->filterColumn(action->text()); });
            }

            menu->addAction(resetAllFilters);            
        }
        else
            return;
    }
    else
      menu->addAction("No item selected");

    menu->exec(QCursor::pos());
}

void SpreadsheetView::findInObdView()
{
    bool ok;
    QString searchText = QInputDialog::getText(this, tr("find in OBD view"),
                                         tr(""), QLineEdit::Normal,
                                         "your text", &ok);
    if (ok && !searchText.isEmpty())
    {
        // get the selected items
        QItemSelection select = selectionModel()->selection();
        if (select.isEmpty())
        {
            return;
        }
        QItemSelectionRange range = select.at(0);
        if (range.isEmpty())
        {
            return;
        }
        else
        {
            int col = range.topLeft().column();
            ObdMergeModel *obdMergeModel = (ObdMergeModel*)this->model();
            int row = obdMergeModel->getRow(col, searchText);

        }

    }
}

void SpreadsheetView::resetAll_Filters()
{
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("obdsortfilterproxymodel"))
    {
        obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
        proxyModel->setFilterRegExp("");
    }
}

void SpreadsheetView::filterColumn(QString value)
{
    QModelIndexList indexList = this->selectedIndexes();
    QModelIndex index = indexList.at(0);
    if (index.isValid())
    {
        QString name = typeid(*model()).name();
        if (name.toLower().endsWith("obdsortfilterproxymodel"))
        {
            obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
            QRegExp regExp("^" + value + "$");
            proxyModel->setFilterRegExp(regExp);
            proxyModel->setFilterKeyColumn(index.column());
        }
    }

}

void SpreadsheetView::copy()
{
    // get the selected items
    QItemSelection select = selectionModel()->selection();
    if (select.isEmpty())
    {
        return;
    }
    QItemSelectionRange range = select.at(0);
    if (range.isEmpty())
    {
        return;
    }
    else
    {
        //set mouse cursor
         QApplication::setOverrideCursor(Qt::WaitCursor);

        // indentify topLeft and btmRight
        QModelIndex topLeft = range.topLeft();
        QModelIndex btmRight = range.bottomRight();

        int rowCount = btmRight.row() - topLeft.row() + 1;
        int colCount = btmRight.column() - topLeft.column() + 1;

        // get the tableView as text
        std::string str;
        for (int i = 0; i < rowCount; ++i)
        {
            //new line into file
//            if (i > 0)
//                str.append("\n");

            //get the row from tableView
            QStringList listVal;
            for (int j = 0; j < colCount; ++j)
            {
                QModelIndex index = this->model()->index(topLeft.row() + i, topLeft.column() + j);
                QString val = (this->model()->data(index, Qt::DisplayRole)).toString();
                listVal.append(val);
            }
            while (!listVal.isEmpty() && listVal.last().isEmpty())
                listVal.removeLast();

            //copy the row to the file
            for (int i = 0; i < listVal.count(); i++)
            {
                if (i > 0)
                    str.append("\t");
                str.append(listVal.at(i).toStdString());
            }

            str.append("\n");
        }

        //set Clipboard
        QApplication::clipboard()->setText(str.c_str());

        //reset mouse cursor
         QApplication::restoreOverrideCursor();
    }
}

void SpreadsheetView::paste()
{
    // get the number of rows and columns
    QString str = QApplication::clipboard()->text();

    // get the selected items
    QItemSelection select = selectionModel()->selection();
    if (select.isEmpty())
    {
        return;
    }
    QItemSelectionRange range = select.at(0);
    if (range.isEmpty())
    {
        return;
    }
    QModelIndex topLeft = range.topLeft();
    QStringList rows = str.split('\n');
    int numRows = rows.count() - 1;
    int btmRightRow = topLeft.row() + numRows - 1;
    if (btmRightRow > model()->rowCount() - 1)
        btmRightRow = model()->rowCount() - 1;

    QList<QStringList> value;
    int numColumns = 0;
    foreach(QString list, rows)
    {
        QStringList columns = list.split('\t');
        while (!columns.isEmpty() && columns.last().isEmpty())
            columns.removeLast();
        value.append(columns);
        if (columns.count() > numColumns)
        {
            numColumns = columns.count();
        }
    }
    int btmRightCol = topLeft.column() + numColumns - 1;
    if (btmRightCol > model()->columnCount() - 1)
        btmRightCol = model()->columnCount()- 1;

    QModelIndex btmRight = model()->index(btmRightRow, btmRightCol);
    QModelIndexList indexList;
    indexList.append(topLeft);
    indexList.append(btmRight);

    // Paste value
    int maxRow = btmRightRow - topLeft.row() + 1;
    QString name = typeid(*model()).name();
    for (int i = 0; i < maxRow; ++i)
    {
        QStringList columns = value.at(i);
        if (!columns.isEmpty())
        {
            for (int j = 0; j < columns.count(); ++j)
            {
                if (!columns.at(j).isEmpty())
                {
                    QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);

                    if (name.toLower().endsWith("sptablemodel") || name.toLower().endsWith("sptablemodelhex"))
                    {
                        ((SpTableModel*)model())->setData(index, indexList, columns.at(j), Qt::EditRole);
                    }
                    else if (name.toLower().endsWith("comparemodel"))
                    {
                        ((CompareModel*)model())->setData(index, indexList, columns.at(j), Qt::EditRole);
                    }
                    else if (name.toLower().endsWith("graphmodel"))
                    {
                        ((GraphModel*)model())->setData(index, indexList, columns.at(j), Qt::EditRole);
                    }
                }
            }
        }
    }

    // Select items
    QItemSelection *selection = new QItemSelection(topLeft, btmRight);
    this->selectionModel()->select(*selection, QItemSelectionModel::Select);

}

void SpreadsheetView::factorM()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();

    bool ok;
    QString valueStr  = QInputDialog::getText(this, tr("HEXplorer::multiply"),
                                         tr("enter your factor:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !valueStr.isEmpty())
    {
        QString name = typeid(*model()).name();
        foreach (QModelIndex index, indexList)
        {
            double val = this->model()->data(index, Qt::DisplayRole).toDouble();
            bool b;
            double fac = valueStr.toDouble(&b);

            if (b)
            {
                if (name.toLower().endsWith("sptablemodel"))
                {
                    ((SpTableModel*)model())->setData(index, indexList,QString::number(val * fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("comparemodel"))
                {
                    ((CompareModel*)model())->setData(index, indexList,QString::number(val * fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("graphmodel"))
                {
                    ((GraphModel*)model())->setData(index, indexList,QString::number(val * fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("obdsortfilterproxymodel"))
                {
                    obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
                    ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
                    QModelIndex indexSource = proxyModel->mapToSource(index);
                    obdModel->setData(indexSource,QString::number(val * fac,'f'), Qt::EditRole);
                }
            }
        }
    }

    //reselect the indexes in view
    foreach(QModelIndex index, indexList)
    {
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void SpreadsheetView::factorD()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("obdmergemodel"))
    {
        return;
    }

    bool ok;
    QString valueStr  = QInputDialog::getText(this, tr("HEXplorer::divide"),
                                         tr("enter your factor:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !valueStr.isEmpty())
    {
        QString name = typeid(*model()).name();
        foreach (QModelIndex index, indexList)
        {
            double val = this->model()->data(index, Qt::DisplayRole).toDouble();
            bool b;
            double fac = valueStr.toDouble(&b);

            if (b && fac != 0)
            {
                if (name.toLower().endsWith("sptablemodel"))
                {
                    ((SpTableModel*)model())->setData(index, indexList,QString::number(val / fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("comparemodel"))
                {
                    ((CompareModel*)model())->setData(index, indexList,QString::number(val / fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("graphmodel"))
                {
                    ((GraphModel*)model())->setData(index, indexList,QString::number(val / fac,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("obdsortfilterproxymodel"))
                {
                    obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
                    ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
                    QModelIndex indexSource = proxyModel->mapToSource(index);
                    obdModel->setData(indexSource,QString::number(val / fac,'f'), Qt::EditRole);
                }
            }
        }

    }

    //reselect the indexes in view
    foreach(QModelIndex index, indexList)
    {
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void SpreadsheetView::offsetP()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("obdmergemodel"))
    {
        return;
    }

    bool ok;
    QString valueStr  = QInputDialog::getText(this, tr("HEXplorer::offset(+)"),
                                         tr("enter your offset:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !valueStr.isEmpty())
    {
        QString name = typeid(*model()).name();
        foreach (QModelIndex index, indexList)
        {
            double val = this->model()->data(index, Qt::DisplayRole).toDouble();
            bool b;
            double offset = valueStr.toDouble(&b);

            if (b)
            {
                if (name.toLower().endsWith("sptablemodel"))
                {
                    ((SpTableModel*)model())->setData(index, indexList,QString::number(val + offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("comparemodel"))
                {
                    ((CompareModel*)model())->setData(index, indexList,QString::number(val + offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("graphmodel"))
                {
                    ((GraphModel*)model())->setData(index, indexList,QString::number(val + offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("obdsortfilterproxymodel"))
                {
                    obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
                    ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
                    QModelIndex indexSource = proxyModel->mapToSource(index);
                    obdModel->setData(indexSource,QString::number(val + offset,'f'), Qt::EditRole);
                }
            }
        }
    }

    //reselect the indexes in view
    foreach(QModelIndex index, indexList)
    {
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void SpreadsheetView::offsetM()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("obdmergemodel"))
    {
        return;
    }

    bool ok;
    QString valueStr  = QInputDialog::getText(this, tr("HEXplorer::offset(-)"),
                                         tr("enter your offset:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !valueStr.isEmpty())
    {
        QString name = typeid(*model()).name();
        foreach (QModelIndex index, indexList)
        {
            double val = this->model()->data(index, Qt::DisplayRole).toDouble();
            bool b;
            double offset = valueStr.toDouble(&b);

            if (b)
            {
                if (name.toLower().endsWith("sptablemodel"))
                {
                    ((SpTableModel*)model())->setData(index, indexList,QString::number(val - offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("comparemodel"))
                {
                    ((CompareModel*)model())->setData(index, indexList,QString::number(val - offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("graphmodel"))
                {
                    ((GraphModel*)model())->setData(index, indexList,QString::number(val - offset,'f'), Qt::EditRole);
                }
                else if (name.toLower().endsWith("obdsortfilterproxymodel"))
                {
                    obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
                    ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
                    QModelIndex indexSource = proxyModel->mapToSource(index);
                    obdModel->setData(indexSource,QString::number(val - offset,'f'), Qt::EditRole);
                }
            }
        }
    }

    //reselect the indexes in view
    foreach(QModelIndex index, indexList)
    {
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void SpreadsheetView::fillAll()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("obdmergemodel"))
    {
        return;
    }

    bool ok;
    QString valueStr  = QInputDialog::getText(this, tr("HEXplorer::setValue"),
                                         tr("enter your value:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !valueStr.isEmpty())
    {
        foreach (QModelIndex index, indexList)
        {
            model()->setData(index, valueStr, Qt::EditRole);
        }
    }
}

void SpreadsheetView::resetM()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel") || name.toLower().endsWith("sptablemodelhex"))
    {
        SpTableModel *spModel = (SpTableModel*)model();
        spModel->resetData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        CompareModel *spModel = (CompareModel*)model();
        spModel->resetData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        GraphModel *spModel = (GraphModel*)model();
        spModel->resetData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("obdmergemodel"))
    {
        ObdMergeModel *obdModel = (ObdMergeModel*)model();
        obdModel->resetData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("obdsortfilterproxymodel"))
    {
        QModelIndexList indexListSource;
        obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
        foreach (QModelIndex index, indexList)
        {
            QModelIndex indexSource = proxyModel->mapToSource(index);
            indexListSource.append(indexSource);

        }
        ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
        obdModel->resetData(indexListSource,  Qt::EditRole);

    }
}

void SpreadsheetView::undoM()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel") || name.toLower().endsWith("sptablemodelhex"))
    {
        SpTableModel *spModel = (SpTableModel*)model();
        spModel->undoData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        CompareModel *cpModel = (CompareModel*)model();
        cpModel->undoData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        GraphModel *grModel = (GraphModel*)model();
        grModel->undoData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("obdmergemodel"))
    {
        ObdMergeModel *obdModel = (ObdMergeModel*)model();
        obdModel->undoData(indexList, Qt::EditRole);
    }
    else if (name.toLower().endsWith("obdsortfilterproxymodel"))
    {
        QModelIndexList indexListSource;
        obdSortFilterProxyModel *proxyModel = (obdSortFilterProxyModel*)model();
        foreach (QModelIndex index, indexList)
        {
            QModelIndex indexSource = proxyModel->mapToSource(index);
            indexListSource.append(indexSource);

        }
        ObdMergeModel *obdModel = (ObdMergeModel*)proxyModel->sourceModel();
        obdModel->undoData(indexListSource,  Qt::EditRole);

    }

    //reselect the indexes in view
    foreach(QModelIndex index, indexList)
    {
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void SpreadsheetView::plot()
{
    // Index
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
    {
        return;
    }
    QModelIndex topLeft = indexList.at(0);

    //get the label
    Data *data = 0;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        SpTableModel *spModel = (SpTableModel*)model();
        QModelIndex index = spModel->index(topLeft.row(), topLeft.column());
        data = spModel->getLabel(index, Qt::EditRole);
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        CompareModel *spModel = (CompareModel*)model();
        QModelIndex index = spModel->index(topLeft.row(), topLeft.column());
        data = spModel->getLabel(index, Qt::EditRole);
    }

    //plot the label
    if (data)
    {
        data->plot(this);
    }
}

void SpreadsheetView::interpAxisX()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QModelIndex topLeft = indexList.at(0);

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xfirst and Xlast
        int length = data->xCount();
        if (length <= 1)
        {
            return;
        }
        double Xfirst = data->getX(0).toDouble();
        double Xlast = data->getX(length - 1).toDouble();

        // interpolate
        double diff = (Xlast - Xfirst)/(length - 1);
        QModelIndex firstZindex = ((SpTableModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
             QModelIndex index = model()->index(firstZindex.row() - 1, firstZindex.column() + i);
            ((SpTableModel*)model())->setData(index, indexList,QString::number(Xfirst + i * diff, 'f'), Qt::EditRole);
        }
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xfirst and Xlast
        int length = data->xCount();
        if (length <= 1)
        {
            return;
        }
        double Xfirst = data->getX(0).toDouble();
        double Xlast = data->getX(length - 1).toDouble();

        // interpolate
        double diff = (Xlast - Xfirst)/(length - 1);
        QModelIndex firstZindex = ((CompareModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
            QModelIndex index = model()->index(firstZindex.row() - 1, firstZindex.column() + i);
            ((CompareModel*)model())->setData(index, indexList,QString::number(Xfirst + i * diff, 'f'), Qt::EditRole);
        }
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((GraphModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xfirst and Xlast
        int length = data->xCount();
        if (length <= 1)
        {
            return;
        }
        double Xfirst = data->getX(0).toDouble();
        double Xlast = data->getX(length - 1).toDouble();

        // interpolate
        double diff = (Xlast - Xfirst)/(length - 1);
        QModelIndex firstZindex = ((GraphModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
            QModelIndex index = model()->index(firstZindex.row() - 1, firstZindex.column() + i);
            ((GraphModel*)model())->setData(index, indexList,QString::number(Xfirst + i * diff, 'f'), Qt::EditRole);
        }
    }

}

void SpreadsheetView::interpAxisY()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

     QModelIndex topLeft = indexList.at(0);

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xfirst and Xlast
        int length = data->yCount();
        if (length <= 1)
        {
            return;
        }
        double Yfirst = data->getY(0).toDouble();
        double Ylast = data->getY(length - 1).toDouble();

        // interpolate
        double diff = (Ylast - Yfirst)/(length - 1);
        QModelIndex firstZindex = ((SpTableModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
            QModelIndex index = model()->index(firstZindex.row() + i, firstZindex.column() - 1);
            ((SpTableModel*)model())->setData(index, indexList,QString::number(Yfirst + i * diff, 'f'), Qt::EditRole);
        }
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row() , topLeft.column());
        Data *data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xfirst and Xlast
        int length = data->yCount();
        if (length <= 1)
        {
            return;
        }
        double Yfirst = data->getY(0).toDouble();
        double Ylast = data->getY(length - 1).toDouble();

        // interpolate
        double diff = (Ylast - Yfirst)/(length - 1);
        QModelIndex firstZindex = ((SpTableModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
             QModelIndex index = model()->index(firstZindex.row() + i, firstZindex.column() - 1);
            ((CompareModel*)model())->setData(index, indexList,QString::number(Yfirst + i * diff, 'f'), Qt::EditRole);
        }
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((GraphModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Yfirst and Ylast
        int length = data->yCount();
        if (length <= 1)
        {
            return;
        }
        double Yfirst = data->getY(0).toDouble();
        double Ylast = data->getY(length - 1).toDouble();

        // interpolate
        double diff = (Ylast - Yfirst)/(length - 1);
        QModelIndex firstZindex = ((SpTableModel*)model())->getFirstZindex(data);
        for (int i = 0; i < length - 1; ++i)
        {
             QModelIndex index = model()->index(firstZindex.row() + i, firstZindex.column() - 1);
            ((GraphModel*)model())->setData(index, indexList,QString::number(Yfirst + i * diff, 'f'), Qt::EditRole);
        }
    }
}

void SpreadsheetView::interpX()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    int numRows = btmRight.row() - topLeft.row() + 1;
    int numColumns = btmRight.column() - topLeft.column() + 1;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xa and Xb
        double Xa = data->getX(topLeft.column() - 1 - isMap).toDouble();
        double Xb = data->getX(topLeft.column() - 1 - isMap + numColumns - 1).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row() + i, topLeft.column());
                QModelIndex indexB = model()->index(topLeft.row() + i, topLeft.column() + numColumns - 1);
                double Xi = data->getX(topLeft.column() - 1 - isMap + j).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Xi - Xa) * (Zb - Za)/(Xb - Xa) + Za;
                ((SpTableModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xa and Xb
        double Xa = data->getX(topLeft.column() - 1 - isMap).toDouble();
        double Xb = data->getX(topLeft.column() - 1 - isMap + numColumns - 1).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row() + i, topLeft.column());
                QModelIndex indexB = model()->index(topLeft.row() + i, topLeft.column() + numColumns - 1);
                double Xi = data->getX(topLeft.column() - 1 - isMap + j).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Xi - Xa) * (Zb - Za)/(Xb - Xa) + Za;
                ((CompareModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        Data *data = ((GraphModel*)model())->getLabel();

        // determine type of data
        bool isMap = 0;
        if (data->yCount() > 0)
        {
            isMap = true;
        }
        else
        {
            isMap = false;
        }

        // get Xa and Xb
        double Xa = data->getX(topLeft.column() - isMap).toDouble();
        double Xb = data->getX(topLeft.column() - isMap + numColumns - 1).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row() + i, topLeft.column());
                QModelIndex indexB = model()->index(topLeft.row() + i, topLeft.column() + numColumns - 1);
                double Xi = data->getX(topLeft.column() - isMap + j).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Xi - Xa) * (Zb - Za)/(Xb - Xa) + Za;
                ((GraphModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }
    }
}

void SpreadsheetView::interpY()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QModelIndex topLeft = indexList.at(0);
    QModelIndex btmRight = indexList.at(indexList.count() - 1);
    int numRows = btmRight.row() - topLeft.row() + 1;
    int numColumns = btmRight.column() - topLeft.column() + 1;

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);
        if (data->yCount() == 0)
            return;

        // get Xa and Xb
        index = model()->index(topLeft.row(), 1);
        double Ya = model()->data(index).toDouble();
        index = model()->index(topLeft.row() + numRows - 1, 1);
        double Yb = model()->data(index).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row(), topLeft.column() + j);
                QModelIndex indexB = model()->index(topLeft.row() + numRows - 1, topLeft.column() + j);
                QModelIndex indexi = model()->index(topLeft.row() + i, 1);
                double Yi = model()->data(indexi).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Yi - Ya) * (Zb - Za)/(Yb - Ya) + Za;
                ((SpTableModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }

    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);
        if (data->yCount() == 0)
            return;

        // get Xa and Xb
        index = model()->index(topLeft.row(), 1);
        double Ya = model()->data(index).toDouble();
        index = model()->index(topLeft.row() + numRows - 1, 1);
        double Yb = model()->data(index).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row(), topLeft.column() + j);
                QModelIndex indexB = model()->index(topLeft.row() + numRows - 1, topLeft.column() + j);
                QModelIndex indexi = model()->index(topLeft.row() + i, 1);
                double Yi = model()->data(indexi).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Yi - Ya) * (Zb - Za)/(Yb - Ya) + Za;
                ((CompareModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }

    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        Data *data = ((GraphModel*)model())->getLabel();
        if (data->yCount() == 0)
            return;

        // get Xa and Xb
        double Ya = data->getY(topLeft.row() - 1).toDouble();
        double Yb = data->getY(topLeft.row() - 1 + numRows - 1).toDouble();

        // interpolate
        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                QModelIndex index = model()->index(topLeft.row() + i, topLeft.column() + j);
                QModelIndex indexA = model()->index(topLeft.row(), topLeft.column() + j);
                QModelIndex indexB = model()->index(topLeft.row() + numRows - 1, topLeft.column() + j);
                double Yi = data->getY(topLeft.row() + i - 1).toDouble();
                double Za = model()->data(indexA).toDouble();
                double Zb = model()->data(indexB).toDouble();
                double Zi = (Yi - Ya) * (Zb - Za)/(Yb - Ya) + Za;
                ((GraphModel*)model())->setData(index, indexList,QString::number(Zi, 'f'), Qt::EditRole);
            }
        }
    }
}

void SpreadsheetView::editAsText()
{
    // select the area and paste
    QModelIndexList indexList = selectedIndexes();
    if (indexList.isEmpty())
    {
        return;
    }
    QModelIndex topLeft = indexList.at(0);


    Data *data = NULL;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        data = ((GraphModel*)model())->getLabel();
    }

    if (data && (data->getType().toLower() == "val_blk" ||
        data->getType().toLower() == "ascii"))
    {
        DialogEditAsText *wid = new DialogEditAsText(data);
        wid->exec();
    }
}

void SpreadsheetView::editAsBit()
{
    QModelIndexList indexList = selectedIndexes();
    if (indexList.isEmpty())
    {
        return;
    }

    // get the data
    QModelIndex index = indexList.at(0);
    Data *data = NULL;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);
    }

    // convert value into bits
    if (data && !data->getCompuTabAxisZ())
    {
        //get the bits number through the deposit field of the characteristic node
        int _length = data->getZnbyte()*8;

        //open the bit edit window
        ulong value = (ulong)model()->data(index).toDouble();
        DialogBitDisplay *dsp = new DialogBitDisplay(value, _length);
        int ret = dsp->exec();

        // save the result into the Data model
        if (ret == QDialog::Accepted)
        {
            ((SpTableModel*)model())->setData(index, QString::number(value), Qt::EditRole);
        }
    }
}

void SpreadsheetView::editAsHex()
{
    QModelIndexList indexList = selectedIndexes();
    if (indexList.isEmpty())
    {
        return;
    }

    // get the data
    QModelIndex index = indexList.at(0);
    Data *data = NULL;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);
    }
    else
        return;

    // convert value into Hex
    QMainWindow *win = new QMainWindow(0);
    SpreadsheetView *view = new SpreadsheetView(win);
    SpTableModelHex *model = new SpTableModelHex();
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

void SpreadsheetView::editProp()
{
    // select the area and paste
    QModelIndexList indexList = selectedIndexes();
    if (indexList.isEmpty())
    {
        return;
    }
    QModelIndex topLeft = indexList.at(0);


    Data *data = NULL;
    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        // get the data
        data = ((GraphModel*)model())->getLabel();
    }

    if (data)
    {
        LabelProperties *prop = new LabelProperties(data, this);
        prop->exec();
    }
}

void SpreadsheetView::selectAll_label()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QModelIndex index = indexList.first();

    //deselect the selection
    foreach(QModelIndex index, indexList)
    {
       selectionModel()->select(index, QItemSelectionModel::Deselect);
    }

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel") || name.toLower().endsWith("sptablemodelhex"))
    {
        SpTableModel *spModel = (SpTableModel*)model();
        Data *data = spModel->getLabel(index, Qt::EditRole);
        QString toto =  data->getName();
        QModelIndex firstZIndex = spModel->getFirstZindex(data);

        if (index.row() == firstZIndex.row() - 1 && index.column() == firstZIndex.column() - 1)
        {
            // Select all items
            if (data->yCount() > 0)
            {
                QItemSelection *selection = new QItemSelection(index, spModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
            else
            {
                QItemSelection *selection = new QItemSelection(spModel->selectAllX(data).first(), spModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
        }
        else if (index.row() >= firstZIndex.row() && index.column() >= firstZIndex.column())
        {
            // Select only Z items
            QItemSelection *selection = new QItemSelection(spModel->selectAllZ(data).first(),
                                                           spModel->selectAllZ(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() == firstZIndex.row() - 1 && index.column() >= firstZIndex.column())
        {
            // Select only X items
            QItemSelection *selection = new QItemSelection(spModel->selectAllX(data).first(),
                                                           spModel->selectAllX(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() >= firstZIndex.row()  && index.column() == firstZIndex.column() - 1)
        {
            // Select only Yitems
            QItemSelection *selection = new QItemSelection(spModel->selectAllY(data).first(),
                                                           spModel->selectAllY(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);

        }
        else
        {
            return;
        }
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        CompareModel *cpModel = (CompareModel*)model();
        Data *data = cpModel->getLabel(index, Qt::EditRole);

        QModelIndex firstZIndex = cpModel->getFirstZindex(data);

        if (index.row() == firstZIndex.row() - 1 && index.column() == firstZIndex.column() - 1)
        {
            // Select items
            if (data->yCount() > 0)
            {
                QItemSelection *selection = new QItemSelection(index, cpModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
            else
            {
                QItemSelection *selection = new QItemSelection(cpModel->selectAllX(data).first(), cpModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
        }
        else if (index.row() >= firstZIndex.row() && index.column() >= firstZIndex.column())
        {
            // Select only Z items
            QItemSelection *selection = new QItemSelection(cpModel->selectAllZ(data).first(),
                                                           cpModel->selectAllZ(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() == firstZIndex.row() - 1 && index.column() >= firstZIndex.column())
        {
            // Select only X items
            QItemSelection *selection = new QItemSelection(cpModel->selectAllX(data).first(),
                                                           cpModel->selectAllX(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() >= firstZIndex.row()  && index.column() == firstZIndex.column() - 1)
        {
            // Select only Yitems
            QItemSelection *selection = new QItemSelection(cpModel->selectAllY(data).first(),
                                                           cpModel->selectAllY(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else
        {
            return;
        }
    }
    else if (name.toLower().endsWith("graphmodel"))
    {
        GraphModel *grModel = (GraphModel*)model();
        Data *data = grModel->getLabel(index, Qt::EditRole);

        QModelIndex firstZIndex = grModel->getFirstZindex(data);

        if (index.row() == firstZIndex.row() - 1 && index.column() == firstZIndex.column() - 1)
        {
            // Select items
            if (data->yCount() > 0)
            {
                QItemSelection *selection = new QItemSelection(index, grModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
            else
            {
                QItemSelection *selection = new QItemSelection(grModel->selectAllX(data).first(), grModel->selectAllZ(data).last());
                this->selectionModel()->select(*selection, QItemSelectionModel::Select);
            }
        }
        else if (index.row() >= firstZIndex.row() && index.column() >= firstZIndex.column())
        {
            // Select only Z items
            QItemSelection *selection = new QItemSelection(grModel->selectAllZ(data).first(),
                                                           grModel->selectAllZ(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() == firstZIndex.row() - 1 && index.column() >= firstZIndex.column())
        {
            // Select only X items
            QItemSelection *selection = new QItemSelection(grModel->selectAllX(data).first(),
                                                           grModel->selectAllX(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else if (index.row() >= firstZIndex.row()  && index.column() == firstZIndex.column() - 1)
        {
            // Select only Yitems
            QItemSelection *selection = new QItemSelection(grModel->selectAllY(data).first(),
                                                           grModel->selectAllY(data).last());
            this->selectionModel()->select(*selection, QItemSelectionModel::Select);
        }
        else
        {
            return;
        }
    }
}

void SpreadsheetView::changeLabelSize()
{
    QModelIndexList indexList = this->selectedIndexes();
    if (indexList.isEmpty())
        return;

    QModelIndex topLeft = indexList.at(0);

    QString name = typeid(*model()).name();
    if (name.toLower().endsWith("sptablemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((SpTableModel*)model())->getLabel(index, Qt::EditRole);

        int xDim = data->xCount();
        int yDim = data->yCount();
        DialogDataDimension *win = new DialogDataDimension(xDim, data->getMaxAxisX(),
                                                           yDim, data->getMaxAxisY());
        win->exec();

        if (xDim != data->xCount() || yDim != data->yCount())
        {
            data->extendSize(xDim, yDim);
            ((FormCompare*)parent())->on_quicklook_clicked();
        }
    }
    else if (name.toLower().endsWith("comparemodel"))
    {
        // get the data
        QModelIndex index = model()->index(topLeft.row(), topLeft.column());
        Data *data = ((CompareModel*)model())->getLabel(index, Qt::EditRole);

        int xDim = data->xCount();
        int yDim = data->yCount();

        DialogDataDimension *win = new DialogDataDimension(xDim, data->getMaxAxisX(),
                                                           yDim, data->getMaxAxisY());
        win->exec();

        if (xDim != data->xCount() || yDim != data->yCount())
        {
            data->extendSize(xDim, yDim);
            ((FormCompare*)parent())->on_compare_clicked();
        }
    }
}

#ifdef Q_WS_WIN32
void SpreadsheetView::exportToExcel()
{
    //Create connection, add workbook und sheet
//    QAxObject* excel = new QAxObject("Excel.Application", 0);
//    if (excel)
//    {
//        QAxObject* app = excel->querySubObject("Application()");
//        QAxObject* wbks = excel->querySubObject("Workbooks()");
//        QAxObject* wb = wbks->querySubObject("Add()");
//        QAxObject* ws = wb->querySubObject("Worksheets(int)", 1 );

//        QAxObject* rng = ws->querySubObject("Cells()");
//        QAxObject* item = rng->querySubObject("Item(1,1)");
//        item->setProperty("Value", QVariant(7));

//        int n;
//        QList< QVariant> table;
//        for (int i = 0; i < 5; i++)
//        {
//            QList< QVariant> row;
//            for (int j = 0; j<5; j++)
//            {
//                n = i * 5 + j;
//                row.append(QVariant(n));
//            }
//            table.append(QVariant(row));
//        }

//        //get a 5x5 range of cells : from A1 to E5
//        QAxObject *range = ws->querySubObject( "Range(const QString&, const QString&)", QString("A1"), QString("E5"));
//        QVariant vTable(table);
//        range->dynamicCall("SetValue(const QVariant&)", vTable);
//        range->setProperty("HorizontalAlignment", -4108); // Zentrieren
//        range->querySubObject("Font")->setProperty("Bold", true); // Fett
//        range->querySubObject("Interior")->setProperty("ColorIndex", 36); // Fett
//        ws->querySubObject("Columns(1)")->setProperty("ColumnWidth", 20); // Spaltenbreite setzen

//        //Show Excel
//        app->setProperty("Visible", true );

//    QAxObject* excell = new QAxObject( "Excel.Application", 0 );
//    QFile file1("help.html");
//    file1.open(QIODevice::WriteOnly | QIODevice::Text);
//    QTextStream out(&file1);
//    out << excell->generateDocumentation();
//    file1.close();

//    Excel *app = new Excel();
//    if (app->isOk())
//    {
//        //create a workbook
//        app->addWorkBook();

//        //create dummy data
//        int n;
//        QList< QVariant> table;
//        for (int i = 0; i < 5; i++)
//        {
//            QList< QVariant> row;
//            for (int j = 0; j<5; j++)
//            {
//                n = i * 5 + j;
//                row.append(QVariant(n));
//            }
//            table.append(QVariant(row));
//        }

//        for (int i = 0; i < 10000; i++)
//        {
//            //copy data to excel
//            int Ar = 1 + 6*i;
//            int Ac = 1;
//            int Br = 5 + 6*i;
//            int Bc = 5;
//            QString cell1 = getExcelCell(Ar,Ac);
//            QString cell2 = getExcelCell(Br,Bc);

//            app->setRangeValue(cell1, cell2, QVariant(table));

//            //set the font
//            app->setFontBold(cell1, cell2, true);

//            //set the backgroundcolor
//            app->setBackgroundColor(cell1, cell2, 36);
//        }

//        //set first column size


//        //show excel
//        app->showExcel(true);
//    }


}
#endif

QString SpreadsheetView::getExcelCell(int row, int col)
{
    char sCell[18];
    memset(sCell, 0, 18);
    if (col <= 26)
        sprintf(sCell, " %c", 'A' + col - 1);
    else
        sprintf(sCell, "%c%c", 'A' + col / 26 - 1, 'A' + col % 26 - 1);

    QString cell = sCell;
    cell = cell.trimmed() + QString::number(row);

    return cell;
}

void SpreadsheetView::myResize(int index, int a, int b)
{

   QHeaderView *header = horizontalHeader();
   QModelIndexList list = this->selectionModel()->selectedColumns();
   foreach(QModelIndex index, list)
   {
       int col = index.column();
       header->resizeSection(col, b);
   }

}

void SpreadsheetView::updateActions(QModelIndex index)
{
    if (index.isValid())
    {
        QString name = typeid(*model()).name();
        if (name.toLower().endsWith("sptablemodel"))
        {
            export2Excel->setEnabled(false);
            changeSize->setEnabled(true);
            selectAllLabel->setEnabled(true);
            axisXInterpolate->setEnabled(true);
            axisYInterpolate->setEnabled(true);
            interpolateX->setEnabled(true);
            interpolateY->setEnabled(true);
            undoModif->setEnabled(true);
            resetModif->setEnabled(true);
            factorMulti->setEnabled(true);
            factorDiv->setEnabled(true);
            offsetPlus->setEnabled(true);
            offsetMinus->setEnabled(true);
            fillAllWith->setEnabled(true);
            copyAction->setEnabled(true);
            pasteAction->setEnabled(true);
            plotAction->setEnabled(true);
            editText->setEnabled(true);
            editBit->setEnabled(true);
            editHex->setEnabled(true);

        }
        else if (name.toLower().endsWith("sptablemodelhex"))
        {
            export2Excel->setEnabled(false);
            changeSize->setEnabled(true);
            selectAllLabel->setEnabled(true);
            axisXInterpolate->setEnabled(false);
            axisYInterpolate->setEnabled(false);
            interpolateX->setEnabled(false);
            interpolateY->setEnabled(false);
            undoModif->setEnabled(true);
            resetModif->setEnabled(true);
            factorMulti->setEnabled(false);
            factorDiv->setEnabled(false);
            offsetPlus->setEnabled(false);
            offsetMinus->setEnabled(false);
            fillAllWith->setEnabled(true);
            copyAction->setEnabled(true);
            pasteAction->setEnabled(true);
            plotAction->setEnabled(false);
            editText->setEnabled(false);
            editBit->setEnabled(false);
            editHex->setEnabled(false);
        }
        else if (name.toLower().endsWith("comparemodel"))
        {
            export2Excel->setEnabled(false);
            changeSize->setEnabled(true);
            selectAllLabel->setEnabled(true);
            axisXInterpolate->setEnabled(true);
            axisYInterpolate->setEnabled(true);
            interpolateX->setEnabled(true);
            interpolateY->setEnabled(true);
            undoModif->setEnabled(true);
            resetModif->setEnabled(true);
            factorMulti->setEnabled(true);
            factorDiv->setEnabled(true);
            offsetPlus->setEnabled(true);
            offsetMinus->setEnabled(true);
            fillAllWith->setEnabled(true);
            copyAction->setEnabled(true);
            pasteAction->setEnabled(true);
            plotAction->setEnabled(true);
            editText->setEnabled(true);
            editBit->setEnabled(true);
            editHex->setEnabled(true);
        }
        else if (name.toLower().endsWith("measmodel") || name.toLower().endsWith("charmodel"))
        {
            export2Excel->setEnabled(false);
            changeSize->setEnabled(false);
            selectAllLabel->setEnabled(false);
            axisXInterpolate->setEnabled(false);
            axisYInterpolate->setEnabled(false);
            interpolateX->setEnabled(false);
            interpolateY->setEnabled(false);
            undoModif->setEnabled(false);
            resetModif->setEnabled(false);
            factorMulti->setEnabled(false);
            factorDiv->setEnabled(false);
            offsetPlus->setEnabled(false);
            offsetMinus->setEnabled(false);
            fillAllWith->setEnabled(false);
            copyAction->setEnabled(true);
            pasteAction->setEnabled(true);
            plotAction->setEnabled(false);
            editText->setEnabled(false);
            editBit->setEnabled(false);
            editHex->setEnabled(false);
        }
    }
}
