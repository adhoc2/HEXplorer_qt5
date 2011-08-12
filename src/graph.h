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
    void plot3D();
    void showData(bool);
    void invertXY(bool);
};

#endif // GRAPH_H
