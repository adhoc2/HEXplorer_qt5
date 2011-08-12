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
    void calculate();

    //QwtArray<double> innerT;
    QVector<double> innerT;
    //QwtArray<double> outterT;
    QVector<double> outterT;
    double q2trq(double Ni, double Qi);
    double offset;

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
    void setData();
    void setOffset();
    void showData(bool);
    void reDraw(QModelIndex topLeft,QModelIndex bottomRight);
};

#endif // GRAPHVERIFY_H
