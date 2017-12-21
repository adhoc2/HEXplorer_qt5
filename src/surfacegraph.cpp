/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "surfacegraph.h"

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include "Nodes/axis_descr.h"
#include "Nodes/compu_method.h"

using namespace QtDataVisualization;

SurfaceGraphModifier::SurfaceGraphModifier(Q3DSurface *surface, Data *data)
    : m_graph(surface)
{
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis); 

    //fill the data Proxy with Data.
    m_dataProxy = new QSurfaceDataProxy();
    m_data = data;
    fillDataProxy();

    //Create a new series and set data to it.
    m_dataSeries = new QSurface3DSeries(m_dataProxy);
    m_graph->addSeries(m_dataSeries);

    //set DrawMode for DataSeries
    m_dataSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_dataSeries->setFlatShadingEnabled(true);

}

SurfaceGraphModifier::~SurfaceGraphModifier()
{
    delete m_graph;
}

void SurfaceGraphModifier::fillDataProxy()
{
    int dimX = m_data->getX().count();
    int dimY = m_data->getY().count();
    m_rangeMaxX = m_data->getX(0).toDouble();
    m_rangeMaxY = m_data->getZ(0).toDouble();
    m_rangeMaxZ = m_data->getY(0).toDouble();
    m_rangeMinX = m_data->getX(0).toDouble();
    m_rangeMinY = m_data->getZ(0).toDouble();
    m_rangeMinZ = m_data->getY(0).toDouble();

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    double dblX = 1;
    double dblY = 1;
    double dblZ = 1;

    for (int j = 0; j < dimY; j++)
    {
        dblY = m_data->getY(j).toDouble();
        if (dblY > m_rangeMaxZ)
            m_rangeMaxZ = dblY;
        if (dblY < m_rangeMinZ)
            m_rangeMinZ = dblY;

        QSurfaceDataRow *newRow = new QSurfaceDataRow();

        for (int i = 0; i < dimX; i++)
        {
            dblX = m_data->getX(i).toDouble();
            if (dblX > m_rangeMaxX)
                m_rangeMaxX = dblX;
            if (dblX < m_rangeMinX)
                m_rangeMinX = dblX;

            dblZ = m_data->getZ(i * dimY + j).toDouble();
            if (dblZ > m_rangeMaxY)
                m_rangeMaxY = dblZ;
            if (dblZ < m_rangeMinY)
                m_rangeMinY = dblZ;
            qDebug() << dblX << dblZ << dblY;

            *newRow << QVector3D(dblX, dblZ, dblY);
        }
        *dataArray << newRow;
    }

    m_dataProxy->resetArray(dataArray);

    //calculate scaling factors
    double rangeX = abs(m_rangeMaxX - m_rangeMinX);
    double rangeZ = abs(m_rangeMaxZ - m_rangeMinZ);
    double facX = 1;
    double facY = 1;
    double facZ = 1;

    if (rangeX > rangeZ)
    {
        if (rangeZ > 0)
            facZ = rangeX / rangeZ;
    }
    else if (rangeZ > rangeX )
    {
        if (rangeX > 0)
            facX = rangeZ / rangeX;
    }

    qDebug() << "scaleX:" << facX;
    qDebug() << "scaleY:" << facY;
    qDebug() << "scaleZ:" << facZ;
}

void SurfaceGraphModifier::setPlotProperties()
{
    m_graph->setTitle(m_data->getName() + " [" + m_data->getUnit() + "]");

    //AxisX setting
    m_graph->axisX()->setLabelFormat("%.3f");
    m_graph->axisX()->setRange(m_rangeMinX, m_rangeMaxX);
    m_graph->axisX()->setLabelAutoRotation(30);
    AXIS_DESCR *axisX = m_data->getAxisDescrX();
    COMPU_METHOD *cpmX = m_data->getCompuMethodAxisX();
    QString name = "";
    if (axisX)
        name.append(axisX->fixPar("InputQuantity").c_str());
    QString unit = "";
    if (cpmX)
        unit.append(cpmX->fixPar("Unit").c_str());
    QString titleX =  name + " - " + unit;
    m_graph->axisX()->setTitle("AxisX :" + titleX);
    m_graph->axisX()->setTitleVisible(true);

    //AxisY setting (dimZ)
    m_graph->axisY()->setLabelAutoRotation(90);
    //m_graph->axisY()->setRange(m_rangeMinY, m_rangeMaxY);
    m_graph->axisY()->setAutoAdjustRange(true);
    m_graph->axisY()->setTitleVisible(true);
    m_graph->axisY()->setTitle(m_data->getName() + " [" + m_data->getUnit() + "]");

    //AxisZ setting (dimY)
    m_graph->axisZ()->setLabelFormat("%.3f");
    m_graph->axisZ()->setRange(m_rangeMinZ, m_rangeMaxZ);
    m_graph->axisZ()->setLabelAutoRotation(30);
    m_graph->axisZ()->setSegmentCount(5);
    AXIS_DESCR *axisY = m_data->getAxisDescrY();
    COMPU_METHOD *cpmY = m_data->getCompuMethodAxisY();
    name = "";
    if (axisY)
        name.append(axisY->fixPar("InputQuantity").c_str());
    unit = "";
    if (cpmY)
        unit.append(cpmY->fixPar("Unit").c_str());
    QString titleY =  name + " - " + unit;
    m_graph->axisZ()->setTitle("AxisY : " + titleY);
    m_graph->axisZ()->setTitleVisible(true);

    //Sliders setting
    int dimX = m_data->getX().count();
    int dimY = m_data->getY().count();
    m_stepX = (m_rangeMaxX - m_rangeMinX) / float(dimX - 1);
    qDebug() << m_stepX;
    m_stepZ = (m_rangeMaxZ - m_rangeMinZ) / float(dimY - 1);
    qDebug() << m_stepZ;

    m_axisMinSliderX->setMaximum(dimX - 2);
    m_axisMinSliderX->setValue(0);
    m_axisMaxSliderX->setMaximum(dimX - 1);
    m_axisMaxSliderX->setValue(dimX - 1);
    m_axisMinSliderZ->setMaximum(dimY - 2);
    m_axisMinSliderZ->setValue(0);
    m_axisMaxSliderZ->setMaximum(dimY - 1);
    m_axisMaxSliderZ->setValue(dimY - 1);
}

void SurfaceGraphModifier::adjustXMin(int min)
{
    float minX = m_stepX * float(min) + m_rangeMinX;

    int max = m_axisMaxSliderX->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderX->setValue(max);
    }
    float maxX = m_stepX * max + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraphModifier::adjustXMax(int max)
{
    float maxX = m_stepX * float(max) + m_rangeMinX;

    int min = m_axisMinSliderX->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderX->setValue(min);
    }
    float minX = m_stepX * min + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraphModifier::adjustZMin(int min)
{
    float minZ = m_stepZ * float(min) + m_rangeMinZ;

    int max = m_axisMaxSliderZ->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderZ->setValue(max);
    }
    float maxZ = m_stepZ * max + m_rangeMinZ;

    setAxisZRange(minZ, maxZ);
}

void SurfaceGraphModifier::adjustZMax(int max)
{
    float maxX = m_stepZ * float(max) + m_rangeMinZ;

    int min = m_axisMinSliderZ->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderZ->setValue(min);
    }
    float minX = m_stepZ * min + m_rangeMinZ;

    setAxisZRange(minX, maxX);
}

void SurfaceGraphModifier::setAxisXRange(float min, float max)
{
    m_graph->axisX()->setRange(min, max);
}

void SurfaceGraphModifier::setAxisZRange(float min, float max)
{
    m_graph->axisZ()->setRange(min, max);
}

void SurfaceGraphModifier::changeTheme(int theme)
{
    m_graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void SurfaceGraphModifier::setBlackToYellowGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void SurfaceGraphModifier::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

