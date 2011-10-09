// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
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

#include <qobject.h>
#include  <QModelIndex>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotZoomer;
class GraphModel;
class Plot;
class Graph;
class GraphVerify;


class CanvasPicker: public QObject
{
    Q_OBJECT

public:
    CanvasPicker(Graph *parent, Plot *plot, bool inverted = false);
    CanvasPicker(GraphVerify *parent, Plot *plot, bool inverted = false);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual bool event(QEvent *);

private:
    bool isMap;
    bool isInverted;

    Graph *parentGraph;

    void selectPoint(const QPoint &);
    void move(const QPoint &);
    void moveBy(int dx, int dy);
    void release();
    void showCursor(bool enable);
    void shiftPointCursor(bool up);
    void shiftCurveCursor(bool up);

    QwtPlot *plot() { return (QwtPlot *)parent(); }
    const QwtPlot *plot() const { return (QwtPlot *)parent(); }
    GraphModel *model;

    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;

    QwtPlotZoomer *d_zoomer;
    void enableZoomMode(bool on);

    QwtPlotCurve * selectCurve(int yPos);

public slots:
    void reDraw(QModelIndex topLeft,QModelIndex bottomRight);
};
