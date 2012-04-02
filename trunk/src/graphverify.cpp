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

#include "graphverify.h"
#include "plot3d.h"
#include "plot.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QPrintDialog>
#include <QPrinter>
#include <qwt_counter.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qstatusbar.h>
#include "Nodes/project.h"
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <qwt_picker_machine.h>
#include <QInputDialog>
#include "qdebug.h"

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas): QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,
            Qt::RightButton);
    }
};

GraphVerify::GraphVerify(HexFile *hex, QWidget *parent):  QMainWindow(parent)
{
    setupUi(this);
    hexFile = hex;
    offset = 10;

    //get datas
    QString projectName = hex->getA2lFileProject()->getPar("name");;
    if (projectName.toLower() == "c340" || projectName.toLower() == "c342")
    {
        trqSpd = hex->getData("Eng_trqSpd_CUR");
        apsMap = hex->getData("Eng_trqAPSCor_MAP");
        qLim = hexFile->getData("EngPrt_qLim_CUR");
        fmtc = hexFile->getData("FMTC_trq2qBas_MAP");
        friction = hexFile->getData("EngM_trqFrc_MAP");
        trqMax_C = hexFile->getData("FrmMng_trqMax_C");
        trqRef_C = hexFile->getData("FrmMng_trqRef_C");
        monTrqMax_C = hexFile->getData("OvRMon_trqMax_C");

    }
    else if(projectName.toLower() == "p_662")
    {
        trqSpd = hex->getData("RngMod_trqSpd_CUR");
        apsMap = hex->getData("RngMod_trqAPSCor_MAP");
        qLim = hexFile->getData("EngPrt_qLim_CUR");
        fmtc = hexFile->getData("PhyMod_trq2qBasEOM0_MAP");
        if (!fmtc)
            fmtc = hexFile->getData("PhyMod_trq2qBas_MAP");
        friction = hexFile->getData("RngMod_trqFrc_MAP");
        trqMax_C = hexFile->getData("Frm_trqMax_C");
        trqRef_C = hexFile->getData("Frm_trqRef_C");
        monTrqMax_C = hexFile->getData("MoFFrm_trqMax_C");
    }

    // model
    QList<Data*> *list = new QList<Data*>;
    if (trqSpd)
        list->append(trqSpd);
    if (apsMap)
        list->append(apsMap);
    if (trqMax_C)
        list->append(trqMax_C);
    if (trqRef_C)
        list->append(trqRef_C);
    if (monTrqMax_C)
        list->append(monTrqMax_C);
    model = new SpTableModel();
    model->setList(list);

    // tableview Widget
    table = new SpreadsheetView();
    table->setModel(model);
    table->horizontalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setDefaultSectionSize(18);

    // plot Widget
    d_plot = new QwtPlot(this);

    // layout
    verticalLayout->addWidget(d_plot);
    verticalLayout->addWidget(table);
    table->setVisible(false);
    createButtons();

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(reDraw(QModelIndex,QModelIndex)));

    // calculate the inner/outter torque
    calculate();

//    // define zoomer
//    d_zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, d_plot->canvas());
//    d_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
//    d_zoomer[0]->setRubberBandPen(QColor(Qt::green));
//    d_zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
//    d_zoomer[0]->setTrackerPen(QColor(Qt::white));
//    activateZoom(false);

    // define panner
    d_panner = new QwtPlotPanner(d_plot->canvas());
    d_panner->setMouseButton(Qt::MidButton);

    //define picker
    d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        d_plot->canvas());
    d_picker->setStateMachine(new QwtPickerDragPointMachine());
    d_picker->setRubberBandPen(QColor(Qt::green));
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setTrackerPen(QColor(Qt::black));

    activateZoom(false);

}

GraphVerify::~GraphVerify()
{
    delete table;
    delete hexFile;
    delete model;
    delete d_plot;
    delete trqSpd;
    delete apsMap;
    delete qLim;
    delete fmtc;
    delete friction;
    delete[] d_zoomer;
    delete d_picker;
    delete d_panner;
}

void GraphVerify::createButtons()
{
    QToolButton *btnShowData = new QToolButton(toolBar);
    btnShowData->setText("Show data");
    btnShowData->setCheckable(true);
    btnShowData->setChecked(false);
    btnShowData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *btnVerify = new QToolButton(toolBar);
    btnVerify->setText("calculate");
    btnVerify->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *zoomButton = new QToolButton(toolBar);
    zoomButton->setCheckable(true);
    zoomButton->setChecked(false);
    zoomButton->setText("zoom");
    zoomButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    zoomButton->setVisible(false);

    toolBar->addWidget(btnVerify);
    toolBar->addWidget(btnShowData);
    toolBar->addWidget(zoomButton);

    connect(btnVerify, SIGNAL(clicked()), this, SLOT(setOffset()) );
    connect(btnShowData, SIGNAL(toggled(bool)), this, SLOT(showData(bool)) );
    connect(zoomButton, SIGNAL(toggled(bool)), this, SLOT(activateZoom(bool)));
}

void GraphVerify::showData(bool on)
{
    if (this->isVisible())
    {
        if (on)
        {
            table->setVisible(true);
            this->resize(this->size().width(), this->size().height() + 300);
        }
        else
        {
            table->setVisible(false);
            this->resize(this->size().width(), this->size().height() - 300);
        }
    }
}

void GraphVerify::selectIndexTable(QModelIndex index)
{
    table->selectionModel()->clear();
    table->selectionModel()->select(index, QItemSelectionModel::Select);
}

double GraphVerify::q2trq(double Ni, double Qi)
{
    double Ti = 0;

    //search index of Ni in FMTC.x
    int n1 = -1;
    int n2 = -1;
    int i = 0;
    while (fmtc->getX(i).toDouble() < Ni && i < fmtc->xCount() - 1)
    {
        i++;
    }
    if (fmtc->getX(i).toDouble() == Ni || i == 0)
        n1 = n2 = i;
    else
    {
        if (fmtc->getX(i).toDouble() > Ni)
        {
            n1 = i - 1;
            n2 = i;
        }
        else
            n1 = n2 = fmtc->xCount() - 1;

    }

    //create Zn1 and Zn2
    double *Zn1 = new double[fmtc->yCount()];
    double *Zn2 = new double[fmtc->yCount()];
    if (fmtc->isSortedByRow)
    {
        for (int i = 0; i < fmtc->yCount(); i++)
        {
            Zn1[i] = fmtc->getZ(n1 + i * fmtc->xCount()).toDouble();
            Zn2[i] = fmtc->getZ(n2 + i * fmtc->xCount()).toDouble();
        }
    }
    else
    {
        for (int i = 0; i < fmtc->yCount(); i++)
        {
            Zn1[i] = fmtc->getZ(i  + n1 * fmtc->yCount()).toDouble();
            Zn2[i] = fmtc->getZ(i  + n2 * fmtc->yCount()).toDouble();
        }
    }

    //interpolate
    double *Zn = new double[fmtc->yCount()];
    double N1 = fmtc->getX(n1).toDouble();
    double N2 = fmtc->getX(n2).toDouble();
    for (int i = 0; i < fmtc->yCount();i++)
    {
        if (N1 == N2)
        {
           Zn[i] = Zn1[i];
        }
        else
        {
            Zn[i] = (Ni - N1) * (Zn2[i] - Zn1[i])/ (N2 - N1) + Zn1[i];
        }
        //qDebug((QString::number(Zn[i])).toLocal8Bit().data());
    }

    //search index of Qi in Zn
    int q1 = -1;
    int q2 = -1;
    i = 0;
    while (Zn[i] < Qi && i < fmtc->yCount() - 1)
    {
        i++;
    }
    if (Zn[i] == Qi || i == 0)
        q1 = q2 = i;
    else
    {
        if (Zn[i] > Qi)
        {
            q1 = i - 1;
            q2 = i;
        }
        else
            q1 = q2 = fmtc->yCount() - 1;

    }

    //qDebug(("Q = " + QString::number(q1) + " : " + QString::number(q2)).toStdString().c_str());

    //interpolate Q
    double T1 = fmtc->getY(q1).toDouble();
    double T2 = fmtc->getY(q2).toDouble();
    if (T1 == T2)
    {
        Ti = T1;
    }
    else
    {
        Ti = (Qi - Zn[q1]) * (T2 - T1)/ (Zn[q2] - Zn[q1]) + T1;
    }

    delete[] Zn1;
    delete[] Zn2;
    delete[] Zn;

    //qDebug(("T = " + QString::number(Ti) + " Nm ").toStdString().c_str());
    return Ti;
}

void GraphVerify::setData()
{
    // calculate speedCurve
    QModelIndexList listIndex;
    QModelIndex index = model->getFirstZindex(trqSpd);
    for (int i = 0; i < trqSpd->xCount(); i++)
    {
        listIndex.append(model->getIndex(index.row(), index.column() + i));
    }
    for (int x = 0; x < trqSpd->xCount(); x++)
    {
        //interp outterT
        double Ni = trqSpd->getX(x).toDouble();
        int n1 = -1;
        int n2 = -1;
        int i = 0;
        while (qLim->getX(i).toDouble() < Ni && i < qLim->xCount() - 1)
        {
            i++;
        }
        if (qLim->getX(i).toDouble() == Ni || i == 0)
            n1 = n2 = i;
        else
        {
            if (qLim->getX(i).toDouble() > Ni)
            {
                n1 = i - 1;
                n2 = i;
            }
            else
                n1 = n2 = qLim->xCount() - 1;
        }

        //calculate Ti (+ offset)
        double Ti = 0;
        double T1 = outterT[n1];
        double T2 = outterT[n2];
        double N1 = qLim->getX(n1).toDouble();
        double N2 = qLim->getX(n2).toDouble();
        if (N2 == N1)
        {
            Ti = T1 + offset;
        }
        else
        {
            Ti = (Ni - N1) * (T2 - T1)/ (N2 - N1) + T1 + offset;
        }

        //set trqSpd
        QModelIndex index2 = model->getIndex(index.row(), index.column() + x);
        model->setData(index2, listIndex ,QString::number(Ti), Qt::EditRole);
    }

    // calculate APScor
    listIndex.clear();
    index = model->getFirstZindex(apsMap);
    for (int i = 0; i < apsMap->yCount(); i++)
    {
        for (int j = 0; j < apsMap->xCount(); j++)
        {
            listIndex.append(model->getIndex(index.row() + i, index.column() +  j));
        }
    }
    for (int x = 0; x < apsMap->xCount(); x++)
    {
        //interp outterT
        double Ni = apsMap->getX(x).toDouble();
        int n1 = -1;
        int n2 = -1;
        int i = 0;
        while (qLim->getX(i).toDouble() < Ni && i < qLim->xCount() - 1)
        {
            i++;
        }
        if (qLim->getX(i).toDouble() == Ni || i == 0)
            n1 = n2 = i;
        else
        {
            if (qLim->getX(i).toDouble() > Ni)
            {
                n1 = i - 1;
                n2 = i;
            }
            else
                n1 = n2 = qLim->xCount() - 1;
        }

        double Ti = 0;
        double T1 = innerT[n1];
        double T2 = innerT[n2];
        double N1 = qLim->getX(n1).toDouble();
        double N2 = qLim->getX(n2).toDouble();
        if (N2 == N1)
        {
            Ti = T1 + offset;
        }
        else
        {
            Ti = (Ni - N1) * (T2 - T1)/ (N2 - N1) + T1 + offset;
        }

        //set apsMap
        for (int j = 0; j < apsMap->yCount(); j++)
        {
            QModelIndex index2 = model->getIndex(index.row() + j, index.column() + x);
            model->setData(index2, listIndex, QString::number(Ti), Qt::EditRole);
        }
    }

    // set trqMax_c
    if (trqMax_C)
    {
        QModelIndex index = model->getFirstZindex(trqMax_C);
        QModelIndexList list;
        list.append(index);
        model->setData(index, list, QString::number(apsMap->getMaxZ()), Qt::EditRole);
    }

    // set trqRef_c
    if (trqRef_C)
    {
        QModelIndex index = model->getFirstZindex(trqRef_C);
        QModelIndexList list;
        list.append(index);
        model->setData(index, list, QString::number(apsMap->getMaxZ()), Qt::EditRole);
    }

    // set monTrqMax_c
    if (monTrqMax_C)
    {
        QModelIndex index = model->getFirstZindex(monTrqMax_C);
        QModelIndexList list;
        list.append(index);
        model->setData(index, list, QString::number(apsMap->getMaxZ()), Qt::EditRole);
    }
}

void GraphVerify::calculate()
{
    //axisX
    QVector<double> xData(qLim->xCount());
    int i = 0;
    foreach (QString str, qLim->getX())
    {
        xData[i] = str.toDouble();
        i++;
    }

    //calculate innerT
    for (int i = 0; i < qLim->xCount(); i++)
    {
        double Ni = qLim->getX(i).toDouble();
        double Qi = qLim->getZ(i).toDouble();
        innerT.append(q2trq(Ni, Qi));

        //qDebug() << Ni << ";" << Qi << " : " << q2trq(Ni, Qi);
    }

    //calculate outterT
    for (int i = 0; i < qLim->xCount(); i++)
    {
        outterT.append(innerT[i] + friction->interp2D(xData[i], 90));
    }

    //PLOT : backgroungColor
    d_plot->setCanvasBackground(Qt::white);

    //PLOT : legend
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    d_plot->insertLegend(legend, QwtPlot::RightLegend);

    //PLOT : Axes
    d_plot->setAxisTitle(0, "Torque [Nm]");
    d_plot->setAxisTitle(2, "Engine Speed [rpm]");

    //PLOT : Grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(d_plot);

    //plot innerT curve
    QwtPlotCurve *curveInnerT = new QwtPlotCurve("inner Torque");
    curveInnerT->setSamples(xData, innerT);
    curveInnerT->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveInnerT->setPen(QColor(Qt::blue));
    curveInnerT->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    curveInnerT->attach(d_plot);

    //plot APScor map
    QVector<double> xAPScor(apsMap->xCount());
    QVector<double> yAPScor(apsMap->xCount());
    i = 0;
    foreach (QString str, apsMap->getX())
    {
        xAPScor[i] = str.toDouble();
        i++;
    }

    for (int i = 0; i < apsMap->xCount();i++ )
    {
        bool bl;
        yAPScor[i] = apsMap->getZ(0, i, &bl);
    }

    QwtPlotCurve *curveAPScor = new QwtPlotCurve("APScor map");
    curveAPScor->setSamples(xAPScor, yAPScor);
    curveAPScor->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveAPScor->setPen(QColor(Qt::red));
    curveAPScor->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    curveAPScor->attach(d_plot);

    //plot outterT curve
    QwtPlotCurve *curveOutterT = new QwtPlotCurve("outter Torque");
    curveOutterT->setSamples(xData, outterT);
    curveOutterT->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveOutterT->setPen(QPen(Qt::blue, 0, Qt::DashDotLine));
    curveOutterT->setLegendAttribute(QwtPlotCurve::LegendShowLine);;
    curveOutterT->attach(d_plot);

    //plot speed curve
    QVector<double> xTrqSpd(trqSpd->xCount());
    QVector<double> yTrqSpd(trqSpd->xCount());
    i = 0;
    foreach (QString str, trqSpd->getX())
    {
        xTrqSpd[i] = str.toDouble();
        i++;
    }
    i = 0;
    foreach (QString str, trqSpd->getZ())
    {
        yTrqSpd[i] = str.toDouble();
        i++;
    }

    QwtPlotCurve *curveTrqSpd = new QwtPlotCurve("Torque Speed");
    curveTrqSpd->setSamples(xTrqSpd, yTrqSpd);
    curveTrqSpd->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveTrqSpd->setPen(QPen(Qt::red, 0, Qt::DashDotLine));
    curveTrqSpd->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    curveTrqSpd->attach(d_plot);    
}

QwtPlotCurve * GraphVerify::selectCurve(QString str)
{
    QwtPlotCurve *curve = NULL;

    const QwtPlotItemList& itmList = d_plot->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it )
    {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = (QwtPlotCurve*)(*it);
            if (c->title().text() == str)
            {
                curve = c;
            }
        }
    }

    return curve;
}

void GraphVerify::reDraw(QModelIndex topLeft,QModelIndex bottomRight)
{
    // calculate speedCurve
    QwtPlotCurve *d_selectedCurve = selectCurve("Torque Speed");
    if (d_selectedCurve)
    {
        QVector<double> xData(d_selectedCurve->dataSize());
        QVector<double> yData(d_selectedCurve->dataSize());

        for (int x = 0; x < trqSpd->xCount(); x++)
        {
            xData[x] = trqSpd->getX(x).toDouble();
            yData[x] = trqSpd->getZ(x).toDouble();
        }
        d_selectedCurve->setSamples(xData, yData);
    }

    // calculate APScor
    d_selectedCurve = selectCurve("APScor map");
    if (d_selectedCurve)
    {
        QVector<double> xData(d_selectedCurve->dataSize());
        QVector<double> yData(d_selectedCurve->dataSize());

        for (int x = 0; x < apsMap->xCount(); x++)
        {
            bool bl;
            xData[x] = apsMap->getX(x).toDouble();
            yData[x] = apsMap->getZ(0, x, &bl);
        }
        d_selectedCurve->setSamples(xData, yData);
    }

    d_plot->replot();
}

void GraphVerify::setOffset()
{
    bool ok;
    QString valueStr = QInputDialog::getText(this, tr("HEXplorer :: verify"),
                                         tr("enter an offset:"), QLineEdit::Normal,
                                         "", &ok);

    bool bl;
    offset = valueStr.toDouble(&bl);

    if (ok && bl)
    {
        setData();
    }
}

void GraphVerify::activateZoom(bool on)
{
    d_panner->setEnabled(on);

//    d_zoomer[0]->setEnabled(on);
//    d_zoomer[0]->zoom(0);

//    d_zoomer[1]->setEnabled(on);
//    d_zoomer[1]->zoom(0);

    d_picker->setEnabled(!on);

}
