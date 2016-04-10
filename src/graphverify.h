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

#ifndef GRAPHVERIFY_H
#define GRAPHVERIFY_H

#include "ui_graphverify.h"
#include <QMainWindow>
#include "data.h"
#include "plot.h"
#include <QTableView>
#include "sptablemodel.h"
#include "spreadsheetview.h"
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class Data;
class QwtPlot;

class GraphVerify : public QMainWindow, private Ui::GraphVerify
{
    Q_OBJECT

public:
    explicit GraphVerify(HexFile *hex, QWidget *parent = 0);
    ~GraphVerify();

    void selectIndexTable(QModelIndex index);

private:
    void createButtons();
    QwtPlotCurve * selectCurve(QString str);
    void calculateInnerOutterTorque();
    void plotInnerOutterTorque();

    //QwtArray<double> innerT;
    QVector<double> innerT;
    //QwtArray<double> outterT;
    QVector<double> outterT;
    double q2trq(double Ni, double Qi);
    double offset;
    double tCoolant;

    SpreadsheetView *table;
    HexFile *hexFile;
    SpTableModel *model;
    QwtPlot *d_plot;
    Data *trqSpd;
    Data *apsMap;
    Data *qLim;
    Data *fmtc;
    Data *friction;
    Data *trqMax_C;
    Data *trqRef_C;
    Data *monTrqMax_C;
    QwtPlotZoomer *d_zoomer[2];
    QwtPlotPicker *d_picker;
    QwtPlotPanner *d_panner;

private slots:
    void activateZoom(bool);
    void setDatasetValues();
    void setOffset();
    void showData(bool);
    void reDrawResult();
    void reDrawOutterTorque();
    void on_lineEdit_returnPressed();
    void on_lineEdit_2_returnPressed();
};

#endif // GRAPHVERIFY_H
