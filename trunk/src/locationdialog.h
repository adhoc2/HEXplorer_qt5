/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <QDialog>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QTableWidget;
QT_END_NAMESPACE

class LocationDialog : public QDialog
{
    Q_OBJECT

public:
    LocationDialog(QWidget *parent = 0);

    QSettings::Format format() const;
    QSettings::Scope scope() const;
    QString organization() const;
    QString application() const;

private slots:
    void updateLocationsTable();

private:
    QLabel *formatLabel;
    QLabel *scopeLabel;
    QLabel *organizationLabel;
    QLabel *applicationLabel;
    QComboBox *formatComboBox;
    QComboBox *scopeComboBox;
    QComboBox *organizationComboBox;
    QComboBox *applicationComboBox;
    QGroupBox *locationsGroupBox;
    QTableWidget *locationsTable;
    QDialogButtonBox *buttonBox;
};

#endif
