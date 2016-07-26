#include "foo.h"
#include "Nodes/axis_descr.h"
#include "Nodes/compu_method.h"
#include <QDebug>

Foo::Foo(Data *dat)
{
    data = dat;
}

Foo::~Foo()
{

}

int Foo::Draw(mglGraph *gr)
{
    int dimX = data->getX().count();
    mglData x(dimX);
    int dimY = data->getY().count();
    mglData y(dimY);
    mglData z(dimX,dimY);

    //create datas for mathGL
    double maxX = data->getX().at(0).toDouble();
    double maxY = data->getY().at(0).toDouble();
    double maxZ = data->getZ().at(0).toDouble();
    double minX = data->getX().at(0).toDouble();
    double minY = data->getY().at(0).toDouble();
    double minZ = data->getZ().at(0).toDouble();

    //added for flap map to be displayed
    if (maxZ == minZ)
    {
        minZ -= 10;
        maxZ += 10;
    }

    double dblX, dblY, dblZ;
    for (int i = 0; i < dimX; i++)
    {
        dblX = data->getX(i).toDouble();
        if (dblX > maxX)
            maxX = dblX;
        if (dblX < minX)
            minX = dblX;
        x.a[i] = dblX;
    }

    for (int j = 0; j < dimY; j++)
    {
        dblY = data->getY(j).toDouble();
        if (dblY > maxY)
            maxY = dblY;
        if (dblY < minY)
            minY = dblY;
        y.a[j] = dblY;
    }

    for(long i=0;i<dimX;i++)
    {
        for(long j=0;j<dimY;j++)
        {
            dblZ = data->getZ(i * dimY + j).toDouble();
            if (dblZ > maxZ)
                maxZ = dblZ;
            if (dblZ < minZ)
                minZ = dblZ;
             z.a[i+dimX*j] = dblZ;
        }
    }

     QString title = data->getName() + " [" + data->getUnit() + "]";
     title.replace("_", "_-");
     gr->SetFontSize(2);
     gr->Title(title.toLocal8Bit(), "");

     gr->SetRange('x', minX, maxX);
     gr->SetRange('y',minY ,maxY);
     gr->SetRange('z',minZ,maxZ);
     gr->SetRange('c',minZ,maxZ);

     gr->Light(true);
     gr->Rotate(70,30);
     gr->Axis();
     gr->Box();

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
     gr->Label('x',QString(titleX).toLatin1(),0);

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
     gr->Label('y', QString(titleY).toLatin1(),0);

     //gr->Label('z',"AxisY description \\i{cm}",0);

     gr->Colorbar();
     gr->Mesh(x,y,z);
     gr->Surf(x,y,z);

     return 0;
}


