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

    //create Triples
    triple = new Qwt3D::Triple * [data->getX().count()];
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

        triple[i] = new Qwt3D::Triple[dimY];
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

            triple[i][j] = Qwt3D::Triple(data->getX(i).toDouble(),
                                        data->getY(j).toDouble(),
                                        data->getZ(j,i).toDouble());
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
    surface = new Qwt3D::SurfacePlot;
    setCentralWidget(surface);
    surface->setTitle(data->getName() + " [" + data->getUnit() + "]");

    surface->loadFromData(triple, dimX, dimY);
    surface->setCoordinateStyle(Qwt3D::BOX);

    surface->setRotation(15,0,20);
    surface->setScale(facX, facY, facZ);
    surface->setZoom(0.6);
    surface->setPlotStyle(Qwt3D::FILLEDMESH);
    surface->setMeshColor(Qwt3D::RGBA(0.2f,0.7f,0.3f));

    QFont font;
    font.setItalic(true);
    for (unsigned i=0; i!=surface->coordinates()->axes.size(); ++i)
    {
      surface->coordinates()->axes[i].setMajors(7);
      surface->coordinates()->axes[i].setMinors(4);
      surface->coordinates()->axes[i].setLabelFont(font);
    }

    // Set axisX titles
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

    // Set axisY titles
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

    //surface->coordinates()->axes[Qwt3D::Z1].setLabelString("z-axis");

    surface->coordinates()->setTicLength(0 , 0);

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
