// HEXplorer is an Asap and HEX file editor
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

#ifndef DIALOGPROGRESSDOWNLOAD_H
#define DIALOGPROGRESSDOWNLOAD_H

#include "ui_dialogprogressdownload.h"

class DialogProgressDownload : public QDialog, private Ui::DialogProgressDownload
{
    Q_OBJECT

public:
    explicit DialogProgressDownload(QWidget *parent = 0);

    void clear();
    void update();
    void setStatus(qint64 bytesReceived, qint64 bytesTotal);
    void setMessage(const QString &m);

private:
    QString message;
    qint64 value;
    qint64 maximum;
};

#endif // DIALOGPROGRESSDOWNLOAD_H
