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

#ifndef GRAPH_H
#define GRAPH_H

#include "ui_graph.h"
#include <QMainWindow>
#include "data.h"
#include "plot.h"
#include <QTableView>
#include "graphmodel.h"
#include "spreadsheetview.h"
class CanvasPicker;

class Data;

class Graph : public QMainWindow, private Ui::Graph
{
    Q_OBJECT

public:
    explicit Graph(QWidget *parent = 0, Data *data = 0);
    ~Graph();

    void selectIndexTable(QModelIndex index);

private:
    GraphModel *model;
    Data *data;
    Plot *plotXZ;
    Plot *plotYZ;
    void showInfo(QString text = QString::null);
    void createButtons();
    SpreadsheetView *table;
    CanvasPicker *canvasXZ;
    CanvasPicker *canvasYZ;


signals:

private slots:
    void plot3D_qwt();
    void plot3D_q3dsurface();
    void showData(bool);
    void invertXY(bool);
};

#endif // GRAPH_H
