#ifndef PLOTMATHGL_H
#define PLOTMATHGL_H

#include <QMainWindow>
#include <mgl2/qmathgl.h>
#include <mgl2/data.h>
#include <mgl2/mgl.h>
#include <mgl2/qmathgl.h>
#include <mgl2/qt.h>
#include <mgl2/wnd.h>
#include <mgl2/canvas_wnd.h>
#include "foo.h"
#include "data.h"


class PlotMathGL : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotMathGL(QWidget *parent, Data *data);
    ~PlotMathGL();

private:
    void setMenus_Actions();

    QMathGL *qmgl;
    Foo *foo;

    QAction *actionCopy, *actionPrint, *actionExportJpg;
    QAction *actionRotate, *actionZoomMouse, *actionRestore;
    QAction *actionUpdate, *actionAdjustSize, *actionAddLine;
    QAction *actionAddMark, *actionAddText, *actionMoveLeft;
    QAction *actionMoveRight, *actionMoveUp, *actionMoveDown;
    QAction *actionZoomIn, *actionZoomOut;
    QMenu *fileMenu, *exportMenu;
    QToolBar *toolBarTop, *toolBarGraph;
    QSpinBox *spinBox1;
    QSpinBox *spinBox2;
    QScrollArea *scroll;


};

#endif // PLOTMATHGL_H
