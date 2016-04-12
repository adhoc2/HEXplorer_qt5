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

#include "plot3d.h"
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include "Nodes/axis_descr.h"
#include "Nodes/compu_method.h"
#include "qdebug.h"

Plot3D::Plot3D(QWidget *parent, Data *dat) : QMainWindow(parent)
{
    // pointers
    data = dat;

    // create 3D surface
    setData();

    // design widget
    QToolBar *toolBar = new QToolBar(this);

    QToolButton *btnSpectrogram = new QToolButton(toolBar);
    QToolButton *btnContour = new QToolButton(toolBar);
    QToolButton *btnPrint = new QToolButton(toolBar);

    btnPrint->setText("Print");
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPrint);

    addToolBar(toolBar);

    connect(btnPrint, SIGNAL(clicked()), this, SLOT(printPlot()));

    btnSpectrogram->setChecked(true);
    btnContour->setChecked(false);
}

Plot3D::~Plot3D()
{
    for (int i = 0; i < data->getX().count(); i++)
        delete[] triple[i];
    delete triple;
    delete surface;
}

void Plot3D::setData()
{
    int dimX = data->getX().count();
    int dimY = data->getY().count();

    //create TripleVector
    Qwt3D::TripleVector pos;
    double maxX = data->getX().at(0).toDouble();
    double maxY = data->getY().at(0).toDouble();
    double maxZ = data->getZ().at(0).toDouble();
    double minX = data->getX().at(0).toDouble();
    double minY = data->getY().at(0).toDouble();
    double minZ = data->getZ().at(0).toDouble();
    double dblX, dblY, dblZ;
    for (int i = 0; i < dimX; i++)
    {
        dblX = data->getX(i).toDouble();
        if (dblX > maxX)
            maxX = dblX;
        if (dblX < minX)
            minX = dblX;

        for (int j = 0; j < dimY; j++)
        {
            if (i == 0)
            {
                dblY = data->getY(j).toDouble();
                if (dblY > maxY)
                    maxY = dblY;
                if (dblY < minY)
                    minY = dblY;
            }

            dblZ = data->getZ(i * dimY + j).toDouble();
            if (dblZ > maxZ)
                maxZ = dblZ;
            if (dblZ < minZ)
                minZ = dblZ;

            pos.push_back(Qwt3D::Triple(data->getX(i).toDouble(),
                                        data->getY(j).toDouble(),
                                        data->getZ(j,i).toDouble()));
        }
    }

    //create CellVector
    Qwt3D::CellVector poly;
    Qwt3D::Cell cell(4);

    for (int i = 0; i < dimX - 1; i++)
    {
        for (int j = 0; j < dimY - 1; j++)
        {
            cell[0] = i * dimY + j;
            cell[1] = i * dimY + j + 1;
            cell[2] = (i + 1)*dimY + j + 1;
            cell[3] = (i + 1)*dimY + j;
            poly.push_back(cell);
        }
    }


    //calculate scaling factors
    double rangeX = abs(maxX - minX);
    double rangeY = abs(maxY - minY);
    double rangeZ = abs(maxZ - minZ);
    double facX = 1;
    double facY = 1;
    double facZ = 1;

    if (rangeX > rangeY && rangeX > rangeZ)
    {
        if (rangeY > 0)
            facY = rangeX / rangeY;
        if (rangeZ > 0)
            facZ = rangeX / rangeZ;
    }
    else if (rangeY > rangeX && rangeY > rangeZ)
    {
        if (rangeX > 0)
            facX = rangeY / rangeX;
        if (rangeZ > 0)
            facZ = rangeY / rangeZ;
    }
    else if (rangeZ > rangeX && rangeZ > rangeY)
    {
        if (rangeX > 0)
            facX = rangeZ / rangeX;
        if (rangeY > 0)
            facY = rangeZ / rangeY;
    }

    //create surface
    surface = new Qwt3D::MeshPlot();
    setCentralWidget(surface);
    surface->createDataset(pos, poly);

    // Set axisX label
    AXIS_DESCR *axisX = data->getAxisDescrX();
    COMPU_METHOD *cpmX = data->getCompuMethodAxisX();
    QString name = "";
    if (axisX)
        name.append(axisX->fixPar("InputQuantity").c_str());
    QString unit = "";
    if (cpmX)
        unit.append(cpmX->fixPar("Unit").c_str());
    QString titleX =  name + " - " + unit;
    surface->coordinates()->axes[Qwt3D::X1].setLabelString(titleX);
    surface->coordinates()->axes[Qwt3D::X2].setLabelString(titleX);
    surface->coordinates()->axes[Qwt3D::X3].setLabelString(titleX);
    surface->coordinates()->axes[Qwt3D::X4].setLabelString(titleX);

    // Set axisY label
    AXIS_DESCR *axisY = data->getAxisDescrY();
    COMPU_METHOD *cpmY = data->getCompuMethodAxisY();
    name = "";
    if (axisY)
        name.append(axisY->fixPar("InputQuantity").c_str());
    unit = "";
    if (cpmY)
        unit.append(cpmY->fixPar("Unit").c_str());
    QString titleY =  name + " - " + unit;
    surface->coordinates()->axes[Qwt3D::Y1].setLabelString(titleY);
    surface->coordinates()->axes[Qwt3D::Y2].setLabelString(titleY);
    surface->coordinates()->axes[Qwt3D::Y3].setLabelString(titleY);
    surface->coordinates()->axes[Qwt3D::Y4].setLabelString(titleY);

    // Set axisZ label
    surface->coordinates()->axes[Qwt3D::Z1].setLabelString("z-axis");
    surface->coordinates()->axes[Qwt3D::Z2].setLabelString("z-axis");
    surface->coordinates()->axes[Qwt3D::Z3].setLabelString("z-axis");
    surface->coordinates()->axes[Qwt3D::Z4].setLabelString("z-axis");

    for (unsigned i=0; i!=surface->coordinates()->axes.size(); ++i)
  {
    surface->coordinates()->axes[i].setMajors(7);
    surface->coordinates()->axes[i].setMinors(5);
    surface->coordinates()->axes[i].setLabelColor(Qwt3D::RGBA(0,0,0.4));
    //surface->coordinates()->axes[i].setTicLength(10 , 5);
  }

    surface->setScale(facX, facY, facZ);
    surface->setTitle(data->getName() + " [" + data->getUnit() + "]");
    surface->setTitleFont("Courier",12, QFont::Bold);
    surface->setRotation(15,0,20);
    surface->setZoom(0.5);
    surface->setPlotStyle(Qwt3D::FILLEDMESH);
    surface->setMeshColor(Qwt3D::RGBA(0.2f,0.7f,0.3f));
    surface->setMeshLineWidth(1);
    surface->setCoordinateStyle(Qwt3D::BOX);
    surface->coordinates()->setGridLinesColor(Qwt3D::RGBA(0,0,0.5));
    surface->coordinates()->setLineWidth(1);
    surface->coordinates()->setNumberColor(Qwt3D::RGBA(0,0.5,0));
    surface->coordinates()->setNumberFont("Courier",10);
    surface->coordinates()->setLabelFont("Courier",10, QFont::Bold);
    surface->coordinates()->setTicLength(10 , 5);
    surface->coordinates()->adjustLabels(20);
    surface->coordinates()->adjustNumbers(10);

    surface->makeCurrent();
    surface->updateData();
    surface->updateGL();
    surface->show();

}

void Plot3D::printPlot()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {

    }
}
