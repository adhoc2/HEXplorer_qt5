//HEXplorer is an Asap and HEX file editor
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

#include "graph.h"
#include "plot3d.h"
#include "plot.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QPrintDialog>
#include <QPrinter>
#include <qwt_counter.h>
#include <qwt_plot_zoomer.h>
#include <qstatusbar.h>
#include "canvaspicker.h"
#include <qmessagebox.h>
#include <QHeaderView>
#include "qdebug.h"

#include "surfacegraph.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QScreen>

Graph::Graph(QWidget *parent, Data *dat) :  QMainWindow(parent)
{

    setupUi(this);
    setWindowTitle("HEXplorer :: plot");

    // model
    model = new GraphModel();
    model->setVal(dat);

    // tableview Widget
    table = new SpreadsheetView();
    table->setModel(model);
    table->horizontalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setDefaultSectionSize(18);

    // create plots
    data = dat;
    plotXZ = new Plot(model, QString("XZ"));
    verticalLayout->addWidget(plotXZ);
    canvasXZ =  new CanvasPicker(this, plotXZ);
    if (dat->yCount() > 0)
    {
        plotYZ = new Plot(model, QString("YZ"));
        verticalLayout->addWidget(plotYZ);
        plotYZ->hide();
        canvasYZ =  new CanvasPicker(this, plotYZ);
    }

    // tableView
    verticalLayout->addWidget(table);
    table->setVisible(false);

    createButtons();

}

Graph::~Graph()
{

}

void Graph::createButtons()
{
    QToolButton *btnPrint = new QToolButton(toolBar);
    btnPrint->setText("Print");
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *invertAxis = new QToolButton(toolBar);
    invertAxis->setText("X<->Y");
    invertAxis->setCheckable(true);
    invertAxis->setChecked(false);
    invertAxis->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *btn3D = new QToolButton(toolBar);
    btn3D->setText("3D");
    btn3D->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *btn3D_3 = new QToolButton(toolBar);
    btn3D_3->setText("Q3DSurface");
    btn3D_3->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn3D_3->setEnabled(true);

    QToolButton *btnShowData = new QToolButton(toolBar);
    btnShowData->setText("Show data");
    btnShowData->setCheckable(true);
    btnShowData->setChecked(false);
    btnShowData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBar->addWidget(btnShowData);
    toolBar->addWidget(invertAxis);
    toolBar->addWidget(btn3D);
    toolBar->addWidget(btn3D_3);
    toolBar->addWidget(btnPrint);


    connect(btnPrint, SIGNAL(clicked()), plotXZ, SLOT(printPlot()) );
    connect(btn3D, SIGNAL(clicked()), this, SLOT(plot3D_qwt()));
    connect(btn3D_3, SIGNAL(clicked()), this, SLOT(plot3D_q3dsurface()));
    connect(btnShowData, SIGNAL(toggled(bool)), this, SLOT(showData(bool)) );
    connect(invertAxis, SIGNAL(toggled(bool)), this, SLOT(invertXY(bool)));
}

void Graph::showInfo(QString text)
{
    if ( text == QString::null )
    {
    }

#ifndef QT_NO_STATUSBAR
     statusBar()->showMessage(text);
#endif
}

void Graph::plot3D_qwt()
{
    if ( !QGLFormat::hasOpenGL() )
    {
          QMessageBox::warning(0, "HEXplorer :: plot 3D",
                                          "3D plot not possible.\nThis system has no OpenGL features.",
                                          QMessageBox::Ok, QMessageBox::Cancel);
          return;
    }

    if (data->getY().count() > 0)
    {
        Plot3D *plot = new Plot3D(0, data);
        plot->resize(600, 400);
        plot->show();

    }
}

void Graph::plot3D_q3dsurface()
{
    if (data->getY().count() > 0)
    {
        //create a new Q3DSurface
        Q3DSurface *graph = new Q3DSurface();
        QWidget *container = QWidget::createWindowContainer(graph);

        //check openGL
        if (!graph->hasContext())
        {
            QMessageBox msgBox;
            msgBox.setText("Couldn't initialize the OpenGL context.");
            msgBox.exec();
            return;
        }

        //define Q3DSurface layout widget
        QSize screenSize = graph->screen()->size();
        container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
        container->setMaximumSize(screenSize);
        //container->setMinimumSize(800, 600);
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        container->setFocusPolicy(Qt::StrongFocus);



        //define widget layout
        QWidget *widget = new QWidget;
        QHBoxLayout *hLayout = new QHBoxLayout(widget);
        QVBoxLayout *vLayout = new QVBoxLayout();
        hLayout->addWidget(container, 1);
        hLayout->addLayout(vLayout);
        vLayout->setAlignment(Qt::AlignTop);

        widget->setWindowTitle(QStringLiteral("HEXplorer :: plot3D"));

        QGroupBox *selectionGroupBox = new QGroupBox(QStringLiteral("Selection Mode"));

        QRadioButton *modeNoneRB = new QRadioButton(widget);
        modeNoneRB->setText(QStringLiteral("No selection"));
        modeNoneRB->setChecked(false);

        QRadioButton *modeItemRB = new QRadioButton(widget);
        modeItemRB->setText(QStringLiteral("Item"));
        modeItemRB->setChecked(false);

        QRadioButton *modeSliceRowRB = new QRadioButton(widget);
        modeSliceRowRB->setText(QStringLiteral("Row Slice"));
        modeSliceRowRB->setChecked(false);

        QRadioButton *modeSliceColumnRB = new QRadioButton(widget);
        modeSliceColumnRB->setText(QStringLiteral("Column Slice"));
        modeSliceColumnRB->setChecked(false);

        QVBoxLayout *selectionVBox = new QVBoxLayout;
        selectionVBox->addWidget(modeNoneRB);
        selectionVBox->addWidget(modeItemRB);
        selectionVBox->addWidget(modeSliceRowRB);
        selectionVBox->addWidget(modeSliceColumnRB);
        selectionGroupBox->setLayout(selectionVBox);

        QSlider *axisMinSliderX = new QSlider(Qt::Horizontal, widget);
        axisMinSliderX->setMinimum(0);
        axisMinSliderX->setTickInterval(1);
        axisMinSliderX->setEnabled(true);
        QSlider *axisMaxSliderX = new QSlider(Qt::Horizontal, widget);
        axisMaxSliderX->setMinimum(1);
        axisMaxSliderX->setTickInterval(1);
        axisMaxSliderX->setEnabled(true);
        QSlider *axisMinSliderZ = new QSlider(Qt::Horizontal, widget);
        axisMinSliderZ->setMinimum(0);
        axisMinSliderZ->setTickInterval(1);
        axisMinSliderZ->setEnabled(true);
        QSlider *axisMaxSliderZ = new QSlider(Qt::Horizontal, widget);
        axisMaxSliderZ->setMinimum(1);
        axisMaxSliderZ->setTickInterval(1);
        axisMaxSliderZ->setEnabled(true);

        QComboBox *themeList = new QComboBox(widget);
        themeList->addItem(QStringLiteral("Qt"));
        themeList->addItem(QStringLiteral("Primary Colors"));
        themeList->addItem(QStringLiteral("Digia"));
        themeList->addItem(QStringLiteral("Stone Moss"));
        themeList->addItem(QStringLiteral("Army Blue"));
        themeList->addItem(QStringLiteral("Retro"));
        themeList->addItem(QStringLiteral("Ebony"));
        themeList->addItem(QStringLiteral("Isabelle"));

        QGroupBox *colorGroupBox = new QGroupBox(QStringLiteral("Custom gradient"));

        QLinearGradient grBtoY(0, 0, 1, 100);
        grBtoY.setColorAt(1.0, Qt::black);
        grBtoY.setColorAt(0.67, Qt::blue);
        grBtoY.setColorAt(0.33, Qt::red);
        grBtoY.setColorAt(0.0, Qt::yellow);
        QPixmap pm(24, 100);
        QPainter pmp(&pm);
        pmp.setBrush(QBrush(grBtoY));
        pmp.setPen(Qt::NoPen);
        pmp.drawRect(0, 0, 24, 100);
        QPushButton *gradientBtoYPB = new QPushButton(widget);
        gradientBtoYPB->setIcon(QIcon(pm));
        gradientBtoYPB->setIconSize(QSize(24, 100));

        QLinearGradient grGtoR(0, 0, 1, 100);
        grGtoR.setColorAt(1.0, Qt::darkGreen);
        grGtoR.setColorAt(0.5, Qt::yellow);
        grGtoR.setColorAt(0.2, Qt::red);
        grGtoR.setColorAt(0.0, Qt::darkRed);
        pmp.setBrush(QBrush(grGtoR));
        pmp.drawRect(0, 0, 24, 100);
        QPushButton *gradientGtoRPB = new QPushButton(widget);
        gradientGtoRPB->setIcon(QIcon(pm));
        gradientGtoRPB->setIconSize(QSize(24, 100));

        QHBoxLayout *colorHBox = new QHBoxLayout;
        colorHBox->addWidget(gradientBtoYPB);
        colorHBox->addWidget(gradientGtoRPB);
        colorGroupBox->setLayout(colorHBox);

        vLayout->addWidget(selectionGroupBox);
        vLayout->addWidget(new QLabel(QStringLiteral("Column range")));
        vLayout->addWidget(axisMinSliderX);
        vLayout->addWidget(axisMaxSliderX);
        vLayout->addWidget(new QLabel(QStringLiteral("Row range")));
        vLayout->addWidget(axisMinSliderZ);
        vLayout->addWidget(axisMaxSliderZ);
        vLayout->addWidget(new QLabel(QStringLiteral("Theme")));
        vLayout->addWidget(themeList);
        vLayout->addWidget(colorGroupBox);

        widget->show();

        //create a SurfaceGraph and set properties
        SurfaceGraphModifier *modifier = new SurfaceGraphModifier(graph, data);

        QObject::connect(modeNoneRB, &QRadioButton::toggled,
                         modifier, &SurfaceGraphModifier::toggleModeNone);
        QObject::connect(modeItemRB,  &QRadioButton::toggled,
                         modifier, &SurfaceGraphModifier::toggleModeItem);
        QObject::connect(modeSliceRowRB,  &QRadioButton::toggled,
                         modifier, &SurfaceGraphModifier::toggleModeSliceRow);
        QObject::connect(modeSliceColumnRB,  &QRadioButton::toggled,
                         modifier, &SurfaceGraphModifier::toggleModeSliceColumn);
        QObject::connect(axisMinSliderX, &QSlider::valueChanged,
                         modifier, &SurfaceGraphModifier::adjustXMin);
        QObject::connect(axisMaxSliderX, &QSlider::valueChanged,
                         modifier, &SurfaceGraphModifier::adjustXMax);
        QObject::connect(axisMinSliderZ, &QSlider::valueChanged,
                         modifier, &SurfaceGraphModifier::adjustZMin);
        QObject::connect(axisMaxSliderZ, &QSlider::valueChanged,
                         modifier, &SurfaceGraphModifier::adjustZMax);
        QObject::connect(themeList, SIGNAL(currentIndexChanged(int)),
                         modifier, SLOT(changeTheme(int)));
        QObject::connect(gradientBtoYPB, &QPushButton::pressed,
                         modifier, &SurfaceGraphModifier::setBlackToYellowGradient);
        QObject::connect(gradientGtoRPB, &QPushButton::pressed,
                         modifier, &SurfaceGraphModifier::setGreenToRedGradient);

        modifier->setAxisMinSliderX(axisMinSliderX);
        modifier->setAxisMaxSliderX(axisMaxSliderX);
        modifier->setAxisMinSliderZ(axisMinSliderZ);
        modifier->setAxisMaxSliderZ(axisMaxSliderZ);

        modifier->setGreenToRedGradient();

        //create the surface with datas
        modifier->setPlotProperties();

        modeItemRB->setChecked(true);
        themeList->setCurrentIndex(0);
    }

}

void Graph::showData(bool on)
{
    if (on)
    {
        table->setVisible(true);
        this->resize(600, 700);
    }
    else
    {
        table->setVisible(false);
        this->resize(600, 400);
    }
}

void Graph::selectIndexTable(QModelIndex index)
{
    table->selectionModel()->clear();
    table->selectionModel()->select(index, QItemSelectionModel::Select);
}

void Graph::invertXY(bool inverted)
{
    if (data->yCount() > 0)
    {
        if (inverted)
        {
            plotXZ->hide();
            plotYZ->show();
        }
        else
        {
            plotYZ->hide();
            plotXZ->show();
        }
    }
}
