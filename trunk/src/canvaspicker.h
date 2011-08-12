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
