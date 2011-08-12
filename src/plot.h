#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <math.h>
#include "data.h"
#include "graphmodel.h"
#include "sptablemodel.h"

class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(GraphModel *model, QString type = "XZ");
    ~Plot();

    GraphModel * getModel();
    void invertXY(bool inverted);

private:
    int nPointsX;
    int nPointsY;
    GraphModel *model;

    void plotXZ();
    void plotYZ();

public slots:
    void printPlot();
};

#endif // PLOT_H
