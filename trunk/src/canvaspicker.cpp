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

#include <qapplication.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_directpainter.h>
#include "canvaspicker.h"
#include "plot.h"
#include "scrollzoomer.h"
#include "graphmodel.h"
#include "graph.h"
#include "graphverify.h"


const unsigned int c_rangeMax = 1000;

class Zoomer: public ScrollZoomer
{
public:
    Zoomer(QwtPlotCanvas *canvas):
        ScrollZoomer(canvas)
    {
    }

    virtual void rescale()
    {
        QwtScaleWidget *scaleWidget = plot()->axisWidget(yAxis());
        QwtScaleDraw *sd = scaleWidget->scaleDraw();

        int minExtent = 0;
        if ( zoomRectIndex() > 0 )
        {
            // When scrolling in vertical direction
            // the plot is jumping in horizontal direction
            // because of the different widths of the labels
            // So we better use a fixed extent.

            minExtent = sd->spacing() + sd->maxTickLength() + 1;
            minExtent += sd->labelSize(
                scaleWidget->font(), c_rangeMax).width();
        }

        sd->setMinimumExtent(minExtent);

        ScrollZoomer::rescale();
    }
};

CanvasPicker::CanvasPicker(Graph *parent, Plot *plot, bool inverted):  QObject(plot), d_selectedCurve(NULL),  d_selectedPoint(-1)
{
    parentGraph = parent;
    isInverted = inverted;
    QwtPlotCanvas *canvas = plot->canvas();
    canvas->installEventFilter(this);

    model = plot->getModel();
    if (model->getLabel()->yCount() > 0)
        isMap = true;
    else
        isMap = false;

    // We want the focus, but no focus rect. The
    // selected point will be highlighted instead.
    canvas->setFocusPolicy(Qt::StrongFocus);
#ifndef QT_NO_CURSOR
    canvas->setCursor(Qt::PointingHandCursor);
#endif
    canvas->setFocusIndicator(QwtPlotCanvas::ItemFocusIndicator);
    canvas->setFocus();
    //shiftCurveCursor(true);

    // zooming
    d_zoomer = new Zoomer(canvas);
    //d_zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    d_zoomer->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
    d_zoomer->setTrackerPen(QPen(Qt::white));
    d_zoomer->setRubberBand(QwtPicker::RectRubberBand);
//    d_zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    enableZoomMode(false);

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(reDraw(QModelIndex,QModelIndex)));
}

CanvasPicker::CanvasPicker(GraphVerify *parent, Plot *plot, bool inverted):  QObject(plot), d_selectedCurve(NULL),  d_selectedPoint(-1)
{

}

bool CanvasPicker::event(QEvent *e)
{
    if ( e->type() == QEvent::User )
    {
        showCursor(true);
        return true;
    }
    return QObject::event(e);
}

bool CanvasPicker::eventFilter(QObject *object, QEvent *e)
{
    if ( object != (QObject *)plot()->canvas() )
        return false;

    switch(e->type())
    {
        case QEvent::FocusIn:
            showCursor(true);
        case QEvent::FocusOut:
            showCursor(false);
        case QEvent::Paint:
        {   
            QApplication::postEvent(this, new QEvent(QEvent::User));
            break;
        }
        case QEvent::MouseButtonPress:
        {
            selectPoint(((QMouseEvent *)e)->pos());
            return true; 
        }
        case QEvent::MouseMove:
        {
            move(((QMouseEvent *)e)->pos());
            return true; 
        }
        case QEvent::KeyPress:
        {
            switch(((const QKeyEvent *)e)->key())
            {
                case Qt::Key_Up:
                    shiftCurveCursor(true);
                    return true;
                    
                case Qt::Key_Down:
                    shiftCurveCursor(false);
                    return true;

                case Qt::Key_Right:
                    if ( d_selectedCurve )
                        shiftPointCursor(true);
                    else
                        shiftCurveCursor(true);
                    return true;

                case Qt::Key_Left:
                    if ( d_selectedCurve )
                        shiftPointCursor(false);
                    else
                        shiftCurveCursor(true);
                    return true;

                case Qt::Key_Plus:
                        moveBy(0, -5);
                        return true;

                case Qt::Key_Minus:
                        moveBy(0, 5);
                        return true;

                default:
                    break;
            }
        }

    default:
            break;
    }
    return QObject::eventFilter(object, e);
}

// Select the point at a position. If there is no point
// deselect the selected point
void CanvasPicker::selectPoint(const QPoint &pos)
{
    QwtPlotCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;

    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it )
    {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = (QwtPlotCurve*)(*it);

            double d;
            int idx = c->closestPoint(pos, &d);
            if ( d < dist )
            {
                curve = c;
                index = idx;
                dist = d;
            } 
        }
    }

    showCursor(false);
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    if ( curve && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = curve;
        d_selectedPoint = index;
        showCursor(true);

        // show item in table
        int row = d_selectedCurve->title().text().toInt();
        QModelIndex index;
        if (!isInverted)
        {
           index = model->index(row + 1, d_selectedPoint + isMap);
        }
        else
        {
            index = model->index(d_selectedPoint + isMap, row + 1);
        }      
        if (parentGraph)
            parentGraph->selectIndexTable(index);

    }
}

// Move the selected point
void CanvasPicker::moveBy(int dx, int dy)
{
//    if ( dx == 0 && dy == 0 )
//        return;

//    if ( !d_selectedCurve )
//        return;

//    const int x = plot()->transform(d_selectedCurve->xAxis(),
//       d_selectedCurve->x(d_selectedPoint)) + dx;
//    const int y = plot()->transform(d_selectedCurve->yAxis(),
//        d_selectedCurve->y(d_selectedPoint)) + dy;

//    move(QPoint(x, y));

    if ( dx == 0 && dy == 0 )
            return;

    if ( !d_selectedCurve )
        return;

    const QPointF sample =
        d_selectedCurve->sample(d_selectedPoint);

    const double x = plot()->transform(
        d_selectedCurve->xAxis(), sample.x());
    const double y = plot()->transform(
        d_selectedCurve->yAxis(), sample.y());

    move( QPoint(qRound(x + dx), qRound(y + dy)) );
}

// Move the selected point
void CanvasPicker::move(const QPoint &pos)
{
    if ( !d_selectedCurve )
        return;

    //set Data via Model
    int row = 0;
    int col = 0;
    if (!isInverted)
    {
        row = d_selectedCurve->title().text().toInt();
        col = d_selectedPoint;
    }
    else
    {
        col = d_selectedCurve->title().text().toInt();
        row = d_selectedPoint;
    }

    model->setData(row, col, plot()->invTransform(d_selectedCurve->yAxis(), pos.y()), Qt::EditRole);

}

// Hightlight the selected point
void CanvasPicker::showCursor(bool showIt)
{

    if ( !d_selectedCurve )
        return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>( d_selectedCurve->symbol() );

    const QBrush brush = symbol->brush();
    if ( showIt )
    {
        //symbol->setBrush(symbol->brush().color().dark(150));
        symbol->setBrush(QColor(Qt::red));
    }

    const bool doReplot = plot()->autoReplot();

    plot()->setAutoReplot(false);

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries(d_selectedCurve, d_selectedPoint, d_selectedPoint);

    if ( showIt )
        symbol->setBrush(brush); // reset brush

    plot()->setAutoReplot(doReplot);
}

// Select the next/previous curve 
void CanvasPicker::shiftCurveCursor(bool up)
{
    QwtPlotItemIterator it;

    const QwtPlotItemList &itemList = plot()->itemList();

    QwtPlotItemList curveList;
    for ( it = itemList.begin(); it != itemList.end(); ++it )
    {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve )
            curveList += *it;
    }
    if ( curveList.isEmpty() )
        return;

    it = curveList.begin();

    if ( d_selectedCurve )
    {
        for ( it = curveList.begin(); it != curveList.end(); ++it )
        {
            if ( d_selectedCurve == *it )
                break;
        }
        if ( it == curveList.end() ) // not found
            it = curveList.begin();

        if ( up )
        {
            ++it;
            if ( it == curveList.end() )
                it = curveList.begin();
        }
        else
        {
            if ( it == curveList.begin() )
                it = curveList.end();
            --it;
        }
    }
        
    showCursor(false);
    //d_selectedPoint = 0;
    d_selectedCurve = (QwtPlotCurve *)*it;
    showCursor(true);

    // show item in table
    int row = d_selectedCurve->title().text().toInt();
    QModelIndex index;
    if (!isInverted)
    {
       index = model->index(row + 1, d_selectedPoint + isMap);
    }
    else
    {
        index = model->index(d_selectedPoint + isMap, row + 1);
    }
    if (parentGraph)
        parentGraph->selectIndexTable(index);
}

// Select the next/previous neighbour of the selected point
void CanvasPicker::shiftPointCursor(bool up)
{
    if ( !d_selectedCurve )
        return;

    int index = d_selectedPoint + (up ? 1 : -1);
    index = (index + d_selectedCurve->dataSize()) % d_selectedCurve->dataSize();

    if ( index != d_selectedPoint )
    {
        showCursor(false);
        d_selectedPoint = index;
        showCursor(true);

        // show item in table
        int row = d_selectedCurve->title().text().toInt();
        QModelIndex index;
        if (!isInverted)
        {
           index = model->index(row + 1, d_selectedPoint + isMap);
        }
        else
        {
            index = model->index(d_selectedPoint + isMap, row + 1);
        }
        if (parentGraph)
            parentGraph->selectIndexTable(index);
    }
}

// Enable Zoom
void CanvasPicker::enableZoomMode(bool on)
{
    d_zoomer->setEnabled(on);
    //d_zoomer->zoom(0);
}

// Update Graph after data in table changed
void CanvasPicker::reDraw(QModelIndex topLeft,QModelIndex bottomRight)
{
    int numRows = bottomRight.row() - topLeft.row() + 1;
    int numColumns = bottomRight.column() - topLeft.column() + 1;
    int nXPts = model->getLabel()->xCount();
    int nYPts = model->getLabel()->yCount();

    if ( nYPts == 0) //Curve
    {        
        QwtPlotCurve *d_selectedCurve = selectCurve(0);

        for (int j = 0; j < numRows; j++)
        {
            for (int i = 0; i < numColumns; i++)
            {
                int row = topLeft.row() + j;
                int col = topLeft.column() + i;

                if (row == 0) //axisX
                {
                    if (d_selectedCurve)
                    {
                        //read Data via Model
                        //QwtArray<double> xData(d_selectedCurve->dataSize());
                        //QwtArray<double> yData(d_selectedCurve->dataSize());
                        QVector<double> xData(d_selectedCurve->dataSize());
                        QVector<double> yData(d_selectedCurve->dataSize());

                        for ( int i = 0; i < d_selectedCurve->dataSize(); i++ )
                        {
                            if ( i == col )
                            {
                                xData[i] = model->data(row - 1, col, Qt::DisplayRole).toDouble();
                                yData[i] = d_selectedCurve->sample(i).y();
                            }
                            else
                            {
                                //xData[i] = d_selectedCurve->x(i);
                                xData[i] = d_selectedCurve->sample(i).x();
                                //yData[i] = d_selectedCurve->y(i);
                                yData[i] = d_selectedCurve->sample(i).y();
                            }
                        }
                        d_selectedCurve->setSamples(xData, yData);
                    }
                }
                else if (row ==  1) //axisZ
                {
                    if (d_selectedCurve)
                    {
                        //read Data via Model
                        //QwtArray<double> xData(d_selectedCurve->dataSize());
                        //QwtArray<double> yData(d_selectedCurve->dataSize());
                        QVector<double> xData(d_selectedCurve->dataSize());
                        QVector<double> yData(d_selectedCurve->dataSize());
                        for ( int i = 0; i < d_selectedCurve->dataSize(); i++ )
                        {
                            if ( i == col )
                            {
                                xData[i] = d_selectedCurve->sample(i).x();
                                yData[i] = model->data(row - 1, col, Qt::DisplayRole).toDouble();

                            }
                            else
                            {
                                xData[i] = d_selectedCurve->sample(i).x();
                                yData[i] = d_selectedCurve->sample(i).y();
                            }
                        }
                        d_selectedCurve->setSamples(xData, yData);
                    }
                }
            }
        }
    }
    else //MAP
    {
        if (!isInverted)
        {
            for (int j = 0; j < numRows; j++)
            {
                for (int i = 0; i < numColumns; i++)
                {
                    int row = topLeft.row() + j;
                    int col = topLeft.column() + i;

                    // X values changes
                    if (row == 0 && col > 0)
                    {
                       for (int n = 0; n < nYPts; n++)
                       {
                            QwtPlotCurve *d_selectedCurve = selectCurve(n);
                            if (d_selectedCurve)
                            {
                                //read Data via Model
                                //QwtArray<double> xData(d_selectedCurve->dataSize());
                                //QwtArray<double> yData(d_selectedCurve->dataSize());
                                QVector<double> xData(d_selectedCurve->dataSize());
                                QVector<double> yData(d_selectedCurve->dataSize());
                                for ( int k = 0; k < d_selectedCurve->dataSize(); k++ )
                                {
                                    if ( k == col - 1 )
                                    {
                                        xData[k] = model->data(row - 1 , col - 1, Qt::DisplayRole).toDouble();
                                        yData[k] = d_selectedCurve->sample(k).y();

                                    }
                                    else
                                    {
                                        xData[k] = d_selectedCurve->sample(k).x();
                                        yData[k] = d_selectedCurve->sample(k).y();
                                    }
                                }
                                d_selectedCurve->setSamples(xData, yData);
                            }
                       }
                    }
                    // z values changes
                    else if (row > 0 && col > 0)
                    {
                        QwtPlotCurve *d_selectedCurve = selectCurve(row - 1);
                        if (d_selectedCurve)
                        {
                            //read Data via Model
                            //QwtArray<double> xData(d_selectedCurve->dataSize());
                            //QwtArray<double> yData(d_selectedCurve->dataSize());
                            QVector<double> xData(d_selectedCurve->dataSize());
                            QVector<double> yData(d_selectedCurve->dataSize());
                            for ( int n = 0; n < d_selectedCurve->dataSize(); n++ )
                            {
                                if ( n == col - 1 )
                                {
                                    xData[n] = d_selectedCurve->sample(n).x();
                                    yData[n] = model->data(row - 1 , col - 1, Qt::DisplayRole).toDouble();

                                }
                                else
                                {
                                    xData[n] = d_selectedCurve->sample(n).x();
                                    yData[n] = d_selectedCurve->sample(n).y();
                                }
                            }
                            d_selectedCurve->setSamples(xData, yData);
                        }
                    }
                }
            }
        }
        else
        {
            for (int j = 0; j < numColumns; j++)
            {
                for (int i = 0; i < numRows; i++)
                {
                    int row = topLeft.row() + i;
                    int col = topLeft.column() + j;

                    //Y values changes
                    if (col == 0 && row > 0)
                    {
                       for (int n = 0; n < nXPts; n++)
                       {
                            QwtPlotCurve *d_selectedCurve = selectCurve(n);
                            if (d_selectedCurve)
                            {
                                //read Data via Model
                                //QwtArray<double> xData(d_selectedCurve->dataSize());
                                //QwtArray<double> yData(d_selectedCurve->dataSize());
                                QVector<double> xData(d_selectedCurve->dataSize());
                                QVector<double> yData(d_selectedCurve->dataSize());
                                for ( int k = 0; k < d_selectedCurve->dataSize(); k++ )
                                {
                                    if ( k == row - 1 )
                                    {
                                        xData[k] = model->data(row - 1 , col - 1, Qt::DisplayRole).toDouble();
                                        yData[k] = d_selectedCurve->sample(k).y();

                                    }
                                    else
                                    {
                                        xData[k] = d_selectedCurve->sample(k).x();
                                        yData[k] = d_selectedCurve->sample(k).y();
                                    }
                                }
                                d_selectedCurve->setSamples(xData, yData);
                            }
                       }
                    }
                    //Z values changes
                    else if (col > 0 && row > 0)
                    {
                        QwtPlotCurve *d_selectedCurve = selectCurve(col - 1);
                        if (d_selectedCurve)
                        {
                            //read Data via Model
                            //QwtArray<double> xData(d_selectedCurve->dataSize());
                            //QwtArray<double> yData(d_selectedCurve->dataSize());
                            QVector<double> xData(d_selectedCurve->dataSize());
                            QVector<double> yData(d_selectedCurve->dataSize());
                            for ( int n = 0; n < d_selectedCurve->dataSize(); n++ )
                            {
                                if ( n == row - 1 )
                                {
                                    xData[n] = d_selectedCurve->sample(n).x();
                                    yData[n] = model->data(row - 1 , col - 1, Qt::DisplayRole).toDouble();

                                }
                                else
                                {
                                    xData[n] = d_selectedCurve->sample(n).x();
                                    yData[n] = d_selectedCurve->sample(n).y();
                                }
                            }
                            d_selectedCurve->setSamples(xData, yData);
                        }
                    }
                }
            }
        }
    }

    // redraw graph
    plot()->replot();
    plot()->repaint();
    showCursor(true);
}

// Select the curve from Table position
QwtPlotCurve * CanvasPicker::selectCurve(int yPos)
{
    QwtPlotCurve *curve = NULL;

    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it )
    {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = (QwtPlotCurve*)(*it);
            if (c->title().text().toInt() == yPos)
            {
                curve = c;
            }
        }
    }

    return curve;
}
