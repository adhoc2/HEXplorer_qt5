#ifndef Plot3D_H
#define Plot3D_H

#include <QMainWindow>
#include <qwt3d_surfaceplot.h>
#include <qwt3d_types.h>
#include "data.h"

class Plot3D : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plot3D(QWidget *parent = 0, Data *data = 0);
    ~Plot3D();

private:
    Data *data;
    Qwt3D::Triple **triple;
    Qwt3D::SurfacePlot *surface;
    void setData();

public slots:
    void printPlot();

};

#endif // Plot3D_H
