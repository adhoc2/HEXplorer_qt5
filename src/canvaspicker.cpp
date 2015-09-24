#include <qapplication.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_layout.h>
#include "canvaspicker.h"
#include "plot.h"
#include "graphmodel.h"
#include "graph.h"
#include "graphverify.h"
#include <qglobal.h>
#include <qtimer.h>
#include "scrollzoomer.h"
#include <qdebug.h>

const unsigned int c_rangeMax = 1000;

class Zoomer: public ScrollZoomer
{
public:
    Zoomer( QWidget *canvas ):
        ScrollZoomer( canvas )
    {
#if 0
        setRubberBandPen( QPen( Qt::red, 2, Qt::DotLine ) );
#else
        setRubberBandPen( QPen( Qt::red ) );
#endif
    }

    virtual QwtText trackerTextF( const QPointF &pos ) const
    {
        QColor bg( Qt::white );

        QwtText text = QwtPlotZoomer::trackerTextF( pos );
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }

    virtual void rescale()
    {
        QwtScaleWidget *scaleWidget = plot()->axisWidget( yAxis() );
        QwtScaleDraw *sd = scaleWidget->scaleDraw();

        double minExtent = 0.0;
        if ( zoomRectIndex() > 0 )
        {
            // When scrolling in vertical direction
            // the plot is jumping in horizontal direction
            // because of the different widths of the labels
            // So we better use a fixed extent.

            minExtent = sd->spacing() + sd->maxTickLength() + 1;
            minExtent += sd->labelSize(
                scaleWidget->font(), c_rangeMax ).width();
        }

        sd->setMinimumExtent( minExtent );

        ScrollZoomer::rescale();


    }
};

CanvasPicker::CanvasPicker(Graph *parent, Plot *plot, bool inverted ): QObject( plot ),
    d_selectedCurve( NULL ),
    d_selectedPoint( -1 )
{
    parentGraph = parent;
    isInverted = inverted;

    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    canvas->installEventFilter( this );

    model = plot->getModel();
    if (model->getLabel()->yCount() > 0)
        isMap = true;
    else
        isMap = false;

    // We want the focus, but no focus rect. The
    // selected point will be highlighted instead.

    canvas->setFocusPolicy( Qt::StrongFocus );
#ifndef QT_NO_CURSOR
    canvas->setCursor( Qt::PointingHandCursor );
#endif
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();

    const char *text =
        "All points can be moved using the left mouse button "
        "or with these keys:\n\n"
        "- Up:\t\tSelect next curve\n"
        "- Down:\t\tSelect previous curve\n"
        "- Left, ´-´:\tSelect next point\n"
        "- Right, ´+´:\tSelect previous point\n"
        "- 7, 8, 9, 4, 6, 1, 2, 3:\tMove selected point";
    canvas->setWhatsThis( text );

    // zooming
    d_zoomer = new Zoomer(canvas);
    d_zoomer->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
    d_zoomer->setTrackerPen(QPen(Qt::white));
    d_zoomer->setRubberBand(QwtPicker::RectRubberBand);
    enableZoomMode(false);

    // to update the graph when data changed
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(reDraw(QModelIndex,QModelIndex)));
}

CanvasPicker::CanvasPicker(GraphVerify *parent, Plot *plot, bool inverted):  QObject(plot), d_selectedCurve(NULL),  d_selectedPoint(-1)
{

}

QwtPlot *CanvasPicker::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *CanvasPicker::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool CanvasPicker::event( QEvent *ev )
{
    if ( ev->type() == QEvent::User )
    {
        showCursor( true );
        return true;
    }
    return QObject::event( ev );
}

bool CanvasPicker::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
        case QEvent::FocusIn:
        {
            showCursor( true );
            break;
        }
        case QEvent::FocusOut:
        {
            showCursor( false );
            break;
        }
        case QEvent::Paint:
        {
            QApplication::postEvent( this, new QEvent( QEvent::User ) );
            break;
        }
        case QEvent::MouseButtonPress:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            select( mouseEvent->pos() );
            return true;
        }
        case QEvent::MouseMove:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            move( mouseEvent->pos() );
            return true;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );

            const int delta = 5;
            switch( keyEvent->key() )
            {
                case Qt::Key_Up:
                {
                    shiftCurveCursor( true );
                    return true;
                }
                case Qt::Key_Down:
                {
                    shiftCurveCursor( false );
                    return true;
                }
                case Qt::Key_Right:
                case Qt::Key_Plus:
                {
                    if ( d_selectedCurve )
                        shiftPointCursor( true );
                    else
                        shiftCurveCursor( true );
                    return true;
                }
                case Qt::Key_Left:
                case Qt::Key_Minus:
                {
                    if ( d_selectedCurve )
                        shiftPointCursor( false );
                    else
                        shiftCurveCursor( true );
                    return true;
                }

                // The following keys represent a direction, they are
                // organized on the keyboard.

                case Qt::Key_1:
                {
                    moveBy( -delta, delta );
                    break;
                }
                case Qt::Key_2:
                {
                    moveBy( 0, delta );
                    break;
                }
                case Qt::Key_3:
                {
                    moveBy( delta, delta );
                    break;
                }
                case Qt::Key_4:
                {
                    moveBy( -delta, 0 );
                    break;
                }
                case Qt::Key_6:
                {
                    moveBy( delta, 0 );
                    break;
                }
                case Qt::Key_7:
                {
                    moveBy( -delta, -delta );
                    break;
                }
                case Qt::Key_8:
                {
                    moveBy( 0, -delta );
                    break;
                }
                case Qt::Key_9:
                {
                    moveBy( delta, -delta );
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
    }

    return QObject::eventFilter( object, event );
}

// Select the point at a position. If there is no point
// deselect the selected point

void CanvasPicker::select( const QPoint &pos )
{
    QwtPlotCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;

    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = static_cast<QwtPlotCurve *>( *it );

            double d;
            int idx = c->closestPoint( pos, &d );
            if ( d < dist )
            {
                curve = c;
                index = idx;
                dist = d;
            }
        }
    }

    showCursor( false );
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    if ( curve && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = curve;
        d_selectedPoint = index;
        showCursor( true );

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
void CanvasPicker::moveBy( int dx, int dy )
{
//    if ( dx == 0 && dy == 0 )
//        return;

//    if ( !d_selectedCurve )
//        return;

//    const QPointF sample =
//        d_selectedCurve->sample( d_selectedPoint );

//    const double x = plot()->transform(
//        d_selectedCurve->xAxis(), sample.x() );
//    const double y = plot()->transform(
//        d_selectedCurve->yAxis(), sample.y() );

//    move( QPoint( qRound( x + dx ), qRound( y + dy ) ) );

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
void CanvasPicker::move( const QPoint &pos )
{
//    if ( !d_selectedCurve )
//        return;

//    QVector<double> xData( d_selectedCurve->dataSize() );
//    QVector<double> yData( d_selectedCurve->dataSize() );

//    for ( int i = 0;
//        i < static_cast<int>( d_selectedCurve->dataSize() ); i++ )
//    {
//        if ( i == d_selectedPoint )
//        {
//            xData[i] = plot()->invTransform(
//                d_selectedCurve->xAxis(), pos.x() );
//            yData[i] = plot()->invTransform(
//                d_selectedCurve->yAxis(), pos.y() );
//        }
//        else
//        {
//            const QPointF sample = d_selectedCurve->sample( i );
//            xData[i] = sample.x();
//            yData[i] = sample.y();
//        }
//    }
//    d_selectedCurve->setSamples( xData, yData );

//    /*
//       Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
//       updated before we paint the cursor on it.
//     */
//    QwtPlotCanvas *plotCanvas =
//        qobject_cast<QwtPlotCanvas *>( plot()->canvas() );

//    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
//    plot()->replot();
//    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );

//    showCursor( true );

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
void CanvasPicker::showCursor( bool showIt )
{
    if ( !d_selectedCurve )
        return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>( d_selectedCurve->symbol() );

    const QBrush brush = symbol->brush();
    if ( showIt )
    {
        //symbol->setBrush(symbol->brush().color().dark(180));
        symbol->setBrush(QColor(Qt::red));
    }

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries( d_selectedCurve, d_selectedPoint, d_selectedPoint );

    if ( showIt )
        symbol->setBrush( brush ); // reset brush
}

// Select the next/previous curve
void CanvasPicker::shiftCurveCursor( bool up )
{
    QwtPlotItemIterator it;

    const QwtPlotItemList &itemList = plot()->itemList();

    QwtPlotItemList curveList;
    for ( it = itemList.begin(); it != itemList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
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

    showCursor( false );
    //d_selectedPoint = 0;
    d_selectedCurve = static_cast<QwtPlotCurve *>( *it );
    showCursor( true );

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
void CanvasPicker::shiftPointCursor( bool up )
{
    if ( !d_selectedCurve )
        return;

    int index = d_selectedPoint + ( up ? 1 : -1 );
    index = ( index + d_selectedCurve->dataSize() ) % d_selectedCurve->dataSize();

    if ( index != d_selectedPoint )
    {
        showCursor( false );
        d_selectedPoint = index;
        showCursor( true );

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
