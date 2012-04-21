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

#include "plot.h"
#include <QPrintDialog>
#include <QPrinter>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include "Nodes/axis_descr.h"
#include "Nodes/compu_method.h"
#include "csv.h"
#include "cdfxfile.h"


Plot::Plot(GraphModel *mod, QString type) : QwtPlot()
{
    setAutoReplot(false);

    model = mod;
    Data *data = model->getLabel();

    //set Title
    if (data->getHexParent())
    {
        setTitle(data->getName() + " " + data->getUnit() + " (" + QString(data->getHexParent()->name) + ")");
    }
    else if (data->getSrecParent())
    {
        setTitle(data->getName() + " " + data->getUnit() + " (" + QString(data->getSrecParent()->name) + ")");
    }
    else if (data->getCsvParent())
    {
        setTitle(data->getName() + " " + data->getUnit() + " (" + QString(data->getCsvParent()->name) + ")");
    }
    else
    {
        setTitle(data->getName() + " " + data->getUnit() + " (" + QString(data->getCdfxParent()->name) + ")");
    }

    //draw plot
    if (type == "XZ")
        plotXZ();
    else if (type == "YZ")
        plotYZ();

    setAutoReplot(true);

}

Plot::~Plot()
{
}

void Plot::printPlot()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {
        //print(printer);
    }
}

GraphModel * Plot::getModel()
{
    return model;
}

void Plot::invertXY(bool inverted)
{
    // remove old curves
    detachItems(QwtPlotItem::Rtti_PlotCurve, false);

    // add new curves
    if (inverted)
        plotYZ();
    else
        plotXZ();

    // replot
    replot();
}

void Plot::plotXZ()
{
    //get the label to plot
    Data *data = model->getLabel();

    // Set axisX titles
    AXIS_DESCR *axisX = data->getAxisDescrX();
    COMPU_METHOD *cpmX = data->getCompuMethodAxisX();
    QString name = "";
    if (axisX)
    {
        name.append(axisX->fixPar("InputQuantity").c_str());
    }
    QString unit = "";
    if (cpmX)
    {
        unit.append(cpmX->fixPar("Unit").c_str());
    }
    QString description = "";
    if (cpmX)
    {
        description.append(cpmX->fixPar("LongIdentifier").c_str());
    }
    QString titleX =  name + " - " + unit + "\n" + description;
    setAxisTitle(xBottom, titleX);

    // set Grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    // set background
    setCanvasBackground(Qt::white);
    //insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    // Create  data
    nPointsX = data->xCount();
    nPointsY = data->yCount();
    QVector<double> xData(nPointsX);
    QVector<double> yData(nPointsY);
    int i = 0;
    foreach (QString str, data->getX())
    {
        xData[i] = str.toDouble();
        i++;
    }
    i = 0;
    foreach (QString str, data->getY())
    {
        yData[i] = str.toDouble();
        i++;
    }  


    plotLayout()->setAlignCanvasToScales(true);

    // Insert new curves
    if (nPointsY != 0)
    {
        if (data->isSortedByRow)
        {
            for (int i = 0; i < nPointsY; i++)
            {
                QVector<double> zData(nPointsX);
                for (int j = 0; j < nPointsX; j++)
                {
                    zData[j] = data->getZ(i*nPointsX + j).toDouble();
                }
                QwtPlotCurve *curve = new QwtPlotCurve(QString::number(i));
                curve->setSamples(xData, zData);
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
                sym->setSize(QSize(8, 8));
                sym->setPen(QColor(Qt::gray));
                curve->setSymbol(sym);
                curve->setPen(QColor(Qt::blue));
                curve->attach(this);
            }
        }
        else
        {
            for (int i = 0; i < nPointsY; i++)
            {
                QVector<double> zData(nPointsX);
                for (int j = 0; j < nPointsX; j++)
                {
                    zData[j] = data->getZ(j*nPointsY + i).toDouble();
                }
                QwtPlotCurve *curve = new QwtPlotCurve(QString::number(i));
                curve->setSamples(xData, zData);
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
                sym->setSize(QSize(8, 8));
                sym->setPen(QColor(Qt::gray));
                curve->setSymbol(sym);
                curve->setPen(QColor(Qt::blue));
                curve->attach(this);
            }

        }
    }
    else
    {
        QVector<double> zData(nPointsX);
        for (int j = 0; j < nPointsX; j++)
        {
            zData[j] = data->getZ(j).toDouble();
        }

        QwtPlotCurve *curve = new QwtPlotCurve(QString::number(0));
        curve->setSamples(xData, zData);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);
        QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
        sym->setSize(QSize(8, 8));
        sym->setPen(QColor(Qt::gray));
        curve->setSymbol(sym);
        curve->setPen(QColor(Qt::blue));
        curve->attach(this);
    }
}

void Plot::plotYZ()
{
    //get the label to plot
    Data *data = model->getLabel();

    // Set axisX titles
    AXIS_DESCR *axisX = data->getAxisDescrY();
    COMPU_METHOD *cpmX = data->getCompuMethodAxisY();
    QString name = "";
    if (axisX)
    {
        name.append(axisX->fixPar("InputQuantity").c_str());
    }
    QString unit = "";
    if (cpmX)
    {
        unit.append(cpmX->fixPar("Unit").c_str());
    }
    QString description = "";
    if (cpmX)
    {
        description.append(cpmX->fixPar("LongIdentifier").c_str());
    }
    QString titleX =  name + " - " + unit + "\n" + description;
    setAxisTitle(xBottom, titleX);

    // set Grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    // set background
    setCanvasBackground(Qt::white);
    //insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    // Create  xData and yData
    nPointsX = data->yCount();
    nPointsY = data->xCount();
    QVector<double> xData(nPointsX);
    QVector<double> yData(nPointsY);
    int i = 0;
    foreach (QString str, data->getY())
    {
        xData[i] = str.toDouble();
        i++;
    }
    i = 0;
    foreach (QString str, data->getX())
    {
        yData[i] = str.toDouble();
        i++;
    }


    plotLayout()->setAlignCanvasToScales(true);

    // Insert new curves
    if (nPointsY != 0)
    {
        if (!data->isSortedByRow)
        {
            for (int i = 0; i < nPointsY; i++)
            {
                QVector<double> zData(nPointsX);
                for (int j = 0; j < nPointsX; j++)
                {
                    zData[j] = data->getZ(i*nPointsX + j).toDouble();
                }
                QwtPlotCurve *curve = new QwtPlotCurve(QString::number(i));
                curve->setSamples(xData, zData);
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
                sym->setSize(QSize(8, 8));
                sym->setPen(QColor(Qt::gray));
                curve->setSymbol(sym);
                curve->setPen(QColor(Qt::blue));
                curve->attach(this);
            }

        }
        else
        {
            for (int i = 0; i < nPointsY; i++)
            {
                //QwtArray<double> zData(nPointsX);
                QVector<double> zData(nPointsX);
                for (int j = 0; j < nPointsX; j++)
                {
                    zData[j] = data->getZ(j*nPointsY + i).toDouble();
                }
                QwtPlotCurve *curve = new QwtPlotCurve(QString::number(i));
                curve->setSamples(xData, zData);
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
                sym->setSize(QSize(8, 8));
                sym->setPen(QColor(Qt::gray));
                curve->setSymbol(sym);
                curve->setPen(QColor(Qt::blue));
                curve->attach(this);
            }

        }
    }
    else
    {
        //QwtArray<double> zData(nPointsX);
        QVector<double> zData(nPointsX);
        for (int j = 0; j < nPointsX; j++)
        {
            zData[j] = data->getZ(j).toDouble();
        }

        QwtPlotCurve *curve = new QwtPlotCurve(QString::number(0));
        curve->setSamples(xData, zData);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);
        QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
        sym->setSize(QSize(8, 8));
        sym->setPen(QColor(Qt::gray));
        curve->setSymbol(sym);
        curve->setPen(QColor(Qt::blue));
        curve->attach(this);
    }

}
