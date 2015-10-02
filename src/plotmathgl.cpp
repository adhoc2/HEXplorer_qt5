#include "plotmathgl.h"
#include <QScrollArea>
#include <QSpinBox>
#include <QToolBar>
#include <QMenuBar>


//#define MGL_MAX_LINES   (INT_MAX-1000)
//#if !defined(WIN32) && !defined(__APPLE__)
//#include <X11/Xlib.h>
//#endif

#define TR  QObject::tr

PlotMathGL::PlotMathGL(QWidget *parent, Data *data) : QMainWindow(parent)
{
    //ui->setupUi(this);

    qmgl = new QMathGL(this);

    //menus
    scroll = new QScrollArea(this);    //set menus and actions
    setMenus_Actions();

    //Graph
    resize(860,650);
    setWindowTitle("Display Label");

    foo = new Foo(data);
    qmgl->setViewYZ(true);
    qmgl->setDraw(foo);
//    HMGL gr = qmgl->getGraph();
//    mgl_rotate(gr ,50, 50, 50);
//    qmgl->setGraph(gr);
//    qmgl->refresh();
    qmgl->update();
    scroll->setWidget(qmgl);
    this->setCentralWidget(scroll);
}

PlotMathGL::~PlotMathGL()
{

}

void PlotMathGL::setMenus_Actions()
{
    //--------------------------
    //Menu1 : export/print/close
    //--------------------------
    fileMenu = this->menuBar()->addMenu(TR("&File"));
    exportMenu = new QMenu(TR("&Export as 2D ..."), this);
    actionExportJpg = new QAction(QPixmap(), TR("JPG"), this);
    actionExportJpg->setToolTip(TR("Export file as JPEG \t(ALT+J)"));
    actionExportJpg->setShortcut(Qt::ALT+Qt::Key_J);
    this->connect(actionExportJpg, SIGNAL(triggered()), qmgl, SLOT(exportJPG()));
    exportMenu->addAction(actionExportJpg);
    exportMenu->addAction(TR("PNG"), qmgl, SLOT(exportPNG()),Qt::ALT+Qt::Key_P);
    fileMenu->addMenu(exportMenu);

    fileMenu->addSeparator();
    actionPrint = new QAction(QPixmap(), TR("Print &graphics"), this);
    actionPrint->setToolTip(TR("Open printer dialog and print graphics\t(CTRl+P)"));
    actionPrint->setShortcut(Qt::CTRL+Qt::Key_P);
    this->connect(actionPrint, SIGNAL(triggered()), qmgl, SLOT(print()));
    fileMenu->addAction(actionPrint);
    fileMenu->addSeparator();
    fileMenu->addAction(TR("&Close"), this, SLOT(close()), Qt::CTRL+Qt::Key_W);

    fileMenu = this->menuBar()->addMenu(TR("&Graphics"));

    //-------------------------
    //Toolbar Top
    //-------------------------
    toolBarTop = new QToolBar(TR("Graphics"),this);
    this->addToolBar(Qt::TopToolBarArea, toolBarTop);

    actionRotate = new QAction(QPixmap(":/icones/rotate.xpm"), TR("&Rotate by  mouse"), this);
    actionRotate->setCheckable(true);
    this->connect(actionRotate, SIGNAL(toggled(bool)), qmgl, SLOT(setRotate(bool)));
    this->connect(qmgl, SIGNAL(rotateChanged(bool)), actionRotate, SLOT(setChecked(bool)));
    actionRotate->setToolTip(TR("Switch on/off mouse handling of the graphics\n(rotation, shifting, zooming and perspective)."));
    toolBarTop->addAction(actionRotate);

    actionZoomMouse = new QAction(QPixmap(":/icones/zoom_in.xpm"), TR("&Zoom by mouse"), this);
    actionZoomMouse->setCheckable(true);
    this->connect(actionZoomMouse, SIGNAL(toggled(bool)), qmgl, SLOT(setZoom(bool)));
    this->connect(qmgl, SIGNAL(zoomChanged(bool)), actionZoomMouse, SLOT(setChecked(bool)));
    actionZoomMouse->setToolTip(TR("Switch on/off mouse zoom of selected region."));
    toolBarTop->addAction(actionZoomMouse);

    fileMenu->addSeparator();
    actionRestore = new QAction(QPixmap(":/icones/zoom_out.xpm"), TR("Re&store"), this);
    this->connect(actionRestore, SIGNAL(triggered()), qmgl, SLOT(restore()));
    actionRestore->setToolTip(TR("Restore default graphics rotation, zoom and perspective (Alt+Space)."));
    actionRestore->setShortcut(Qt::ALT+Qt::Key_Space);
    fileMenu->addAction(actionRestore);
    toolBarTop->addAction(actionRestore);

    toolBarTop->addSeparator();
    actionUpdate = new QAction(QPixmap(":/icones/ok.xpm"), TR("Re&draw"), this);
    this->connect(actionUpdate, SIGNAL(triggered()), qmgl, SLOT(update()));
    actionUpdate->setToolTip(TR("Execute script and redraw graphics (F5)."));
    actionUpdate->setShortcut(Qt::Key_F5);
    fileMenu->addAction(actionUpdate);
    toolBarTop->addAction(actionUpdate);

    actionAdjustSize = new QAction(TR("&Adjust size"), this);
    this->connect(actionAdjustSize, SIGNAL(triggered()), qmgl, SLOT(adjust()));
    actionAdjustSize->setToolTip(TR("Change canvas size to fill whole region (F6)."));
    actionAdjustSize->setShortcut(Qt::Key_F6);
    fileMenu->addAction(actionAdjustSize);
    toolBarTop->addAction(actionAdjustSize);

    actionCopy = new QAction(QPixmap(":/icones/copy.xpm"), TR("&Copy plot"), this);
    this->connect(actionCopy, SIGNAL(triggered()), qmgl, SLOT(copy()));
    actionCopy->setToolTip(TR("Copy graphics to clipboard (CTRl+Shift+G)."));
    actionCopy->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_G);
    fileMenu->addAction(actionCopy);
    toolBarTop->addAction(actionCopy);

    toolBarTop->addSeparator();
    exportMenu = new QMenu(TR("Primitives ..."),this);

    actionAddLine = new QAction(QPixmap(":/icones/line.xpm"), TR("Add line"), this);
    this->connect(actionAddLine, SIGNAL(triggered()), qmgl, SLOT(addLine()));
    this->connect(qmgl, SIGNAL(usePrimChanged(bool)), actionAddLine, SLOT(setVisible(bool)));
    actionAddLine->setToolTip(TR("Add line which properties can be changed later by mouse."));
    toolBarTop->addAction(actionAddLine);
    exportMenu->addAction(actionAddLine);

    actionAddMark = new QAction(QPixmap(":/icones/mark_a.xpm"), TR("Add mark"), this);
    this->connect(actionAddMark, SIGNAL(triggered()), qmgl, SLOT(addMark()));
    this->connect(qmgl, SIGNAL(usePrimChanged(bool)), actionAddMark, SLOT(setVisible(bool)));
    actionAddMark->setToolTip(TR("Add marker which properties can be changed later by mouse."));
    toolBarTop->addAction(actionAddMark);
    exportMenu->addAction(actionAddMark);

    actionAddText = new QAction(QPixmap(":/icones/text.xpm"), TR("Add text"), this);
    this->connect(actionAddText, SIGNAL(triggered()), qmgl, SLOT(addText()));
    this->connect(qmgl, SIGNAL(usePrimChanged(bool)), actionAddText, SLOT(setVisible(bool)));
    actionAddText->setToolTip(TR("Add text which properties can be changed later by mouse."));
    toolBarTop->addAction(actionAddText);
    exportMenu->addAction(actionAddText);
    fileMenu->addMenu(exportMenu);

    toolBarTop->addSeparator();
    spinBox1 = new QSpinBox(this);
    spinBox1->setWrapping(true);
    toolBarTop->addWidget(spinBox1);
    spinBox1->setRange(-180, 180);
    spinBox1->setSingleStep(10);
    this->connect(spinBox1, SIGNAL(valueChanged(int)), qmgl, SLOT(setTet(int)));
    this->connect(qmgl, SIGNAL(tetChanged(int)), spinBox1, SLOT(setValue(int)));
    spinBox1->setToolTip(TR("Set value of \\theta angle."));

    toolBarTop->addSeparator();
    spinBox2 = new QSpinBox(this);
    spinBox2->setWrapping(true);
    toolBarTop->addWidget(spinBox2);
    spinBox2->setRange(-180, 180);
    spinBox2->setSingleStep(10);
    this->connect(spinBox2, SIGNAL(valueChanged(int)), qmgl, SLOT(setPhi(int)));
    this->connect(qmgl, SIGNAL(phiChanged(int)), spinBox2, SLOT(setValue(int)));
    spinBox2->setToolTip(TR("Set value of \\phi angle."));


    //-------------------------
    //toolbar graph (left)
    //-------------------------
    exportMenu = fileMenu->addMenu(TR("Zoom/move"));
    toolBarGraph = new QToolBar(TR("Zoom graphics"),this);
    this->addToolBar(Qt::LeftToolBarArea, toolBarGraph);

    actionMoveLeft = new QAction(QPixmap(":/icones/left_1.xpm"), TR("Move &left"),  this);
    this->connect(actionMoveLeft, SIGNAL(triggered()), qmgl, SLOT(shiftLeft()));
    actionMoveLeft->setToolTip(TR("Move graphics left by 1/3 of its width."));
    toolBarGraph->addAction(actionMoveLeft);
    exportMenu->addAction(actionMoveLeft);

    actionMoveUp = new QAction(QPixmap(":/icones/up_1.xpm"), TR("Move &up"), this);
    this->connect(actionMoveUp, SIGNAL(triggered()), qmgl, SLOT(shiftUp()));
    actionMoveUp->setToolTip(TR("Move graphics up by 1/3 of its height."));
    toolBarGraph->addAction(actionMoveUp);
    exportMenu->addAction(actionMoveUp);

    actionZoomIn = new QAction(QPixmap(":/icones/zoom_1.xpm"), TR("Zoom &in"), this);
    this->connect(actionZoomIn, SIGNAL(triggered()), qmgl, SLOT(zoomIn()));
    actionZoomIn->setToolTip(TR("Zoom in graphics."));
    toolBarGraph->addAction(actionZoomIn);
    exportMenu->addAction(actionZoomIn);

    actionZoomOut = new QAction(QPixmap(":/icones/norm_1.xpm"), TR("Zoom &out"), this);
    this->connect(actionZoomOut, SIGNAL(triggered()), qmgl, SLOT(zoomOut()));
    actionZoomOut->setToolTip(TR("Zoom out graphics."));
    toolBarGraph->addAction(actionZoomOut);
    exportMenu->addAction(actionZoomOut);

    actionMoveDown = new QAction(QPixmap(":/icones/down_1.xpm"), TR("Move &down"), this);
    this->connect(actionMoveDown, SIGNAL(triggered()), qmgl, SLOT(shiftDown()));
    actionMoveDown->setToolTip(TR("Move graphics up down 1/3 of its height."));
    toolBarGraph->addAction(actionMoveDown);
    exportMenu->addAction(actionMoveDown);

    actionMoveRight = new QAction(QPixmap(":/icones/right_1.xpm"), TR("Move &right"), this);
    this->connect(actionMoveRight, SIGNAL(triggered()), qmgl, SLOT(shiftRight()));
    actionMoveRight->setToolTip(TR("Move graphics right by 1/3 of its width."));
    toolBarGraph->addAction(actionMoveRight);
    exportMenu->addAction(actionMoveRight);

}
