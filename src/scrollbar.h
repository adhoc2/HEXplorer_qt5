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

#ifndef _SCROLLBAR_H
#define _SCROLLBAR_H 1

#include <qscrollbar.h>

class ScrollBar: public QScrollBar
{
    Q_OBJECT

public:
    ScrollBar(QWidget *parent = NULL);
    ScrollBar(Qt::Orientation, QWidget *parent = NULL);
    ScrollBar(double minBase, double maxBase,       
        Qt::Orientation o, QWidget *parent = NULL);

    void setInverted(bool);
    bool isInverted() const;

    double minBaseValue() const;
    double maxBaseValue() const;

    double minSliderValue() const;
    double maxSliderValue() const;

    int extent() const;

signals:
    void sliderMoved(Qt::Orientation, double, double);
    void valueChanged(Qt::Orientation, double, double);

public slots:
    virtual void setBase(double min, double max);
    virtual void moveSlider(double min, double max);

protected:
    void sliderRange(int value, double &min, double &max) const;
    int mapToTick(double) const;
    double mapFromTick(int) const;

private slots:
    void catchValueChanged(int value);
    void catchSliderMoved(int value);

private:
    void init();

    bool d_inverted;
    double d_minBase;
    double d_maxBase;
    int d_baseTicks;
};

#endif
