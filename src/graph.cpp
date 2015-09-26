//HEXplorer is an Asap and HEX file editor
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

#include "graph.h"
#include "plot3d.h"
#include "plot.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QPrintDialog>
#include <QPrinter>
#include <qwt_counter.h>
#include <qwt_plot_zoomer.h>
#include <qstatusbar.h>
#include "canvaspicker.h"
#include "qdebug.h"

Graph::Graph(QWidget *parent, Data *dat) :  QMainWindow(parent)
{
    setupUi(this);
    setWindowTitle("HEXplorer :: plot");

    // model
    model = new GraphModel();
    model->setVal(dat);

    // tableview Widget
    table = new SpreadsheetView();
    table->setModel(model);
    table->horizontalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setDefaultSectionSize(18);

    // create plots
    data = dat;
    plotXZ = new Plot(model, QString("XZ"));
    verticalLayout->addWidget(plotXZ);
    canvasXZ =  new CanvasPicker(this, plotXZ);
    if (dat->yCount() > 0)
    {
        plotYZ = new Plot(model, QString("YZ"));
        verticalLayout->addWidget(plotYZ);
        plotYZ->hide();
        canvasYZ =  new CanvasPicker(this, plotYZ);
    }

    // tableView
    verticalLayout->addWidget(table);
    table->setVisible(false);

    createButtons();

}

Graph::~Graph()
{

}

void Graph::createButtons()
{
    QToolButton *btnPrint = new QToolButton(toolBar);
    btnPrint->setText("Print");
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *invertAxis = new QToolButton(toolBar);
    invertAxis->setText("X<->Y");
    invertAxis->setCheckable(true);
    invertAxis->setChecked(false);
    invertAxis->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *btn3D = new QToolButton(toolBar);
    btn3D->setText("3D");
    btn3D->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *btnShowData = new QToolButton(toolBar);
    btnShowData->setText("Show data");
    btnShowData->setCheckable(true);
    btnShowData->setChecked(false);
    btnShowData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBar->addWidget(btnShowData);
    toolBar->addWidget(invertAxis);
    toolBar->addWidget(btn3D);
    toolBar->addWidget(btnPrint);

    connect(btnPrint, SIGNAL(clicked()), plotXZ, SLOT(printPlot()) );
    connect(btn3D, SIGNAL(clicked()), this, SLOT(plot3D()) );
    connect(btnShowData, SIGNAL(toggled(bool)), this, SLOT(showData(bool)) );
    connect(invertAxis, SIGNAL(toggled(bool)), this, SLOT(invertXY(bool)));
}

void Graph::showInfo(QString text)
{
    if ( text == QString::null )
    {
    }

#ifndef QT_NO_STATUSBAR
     statusBar()->showMessage(text);
#endif
}

void Graph::plot3D()
{
    if (data->getY().count() > 0)
    {
        Plot3D *plot = new Plot3D(0, data);
        plot->resize(600, 400);
        plot->show();
    }
}

void Graph::showData(bool on)
{
    if (on)
    {
        table->setVisible(true);
        this->resize(600, 700);
    }
    else
    {
        table->setVisible(false);
        this->resize(600, 400);
    }
}

void Graph::selectIndexTable(QModelIndex index)
{
    table->selectionModel()->clear();
    table->selectionModel()->select(index, QItemSelectionModel::Select);
}

void Graph::invertXY(bool inverted)
{
    if (data->yCount() > 0)
    {
        if (inverted)
        {
            plotXZ->hide();
            plotYZ->show();
        }
        else
        {
            plotYZ->hide();
            plotXZ->show();
        }
    }
}
