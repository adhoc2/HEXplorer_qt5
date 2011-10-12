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

#ifndef DIALOGUPDATE_H
#define DIALOGUPDATE_H

#include <QtCore>
#include <QtNetwork>
class MDImain;

// based on dtkUpdaterPrivate from Inria

class DialogUpdate : public QObject
{
    Q_OBJECT

public:
    explicit DialogUpdate(const QString &path, bool bl, QWidget *parent = 0);
    ~DialogUpdate();

private:
    MDImain *mdiMain;
    QString updateFilePath;
    QString updateDetails;
    void readConfigFile(QString path, bool bl);
    void downloadUpdate();
    void installUpdate();
    QString getUserName();
    QString newVersion;
};

#endif // DIALOGUPDATE_H


