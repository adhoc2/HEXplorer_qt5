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
