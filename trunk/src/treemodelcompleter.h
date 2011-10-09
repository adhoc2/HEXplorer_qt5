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

#ifndef TREEMODELCOMPLETER_H
#define TREEMODELCOMPLETER_H

#include <QCompleter>

class TreeModelCompleter : public QCompleter
{
    Q_OBJECT
    Q_PROPERTY(QString separator READ separator WRITE setSeparator)

    public:
        TreeModelCompleter(QObject *parent = 0);
        TreeModelCompleter(QAbstractItemModel *model, QObject *parent = 0);

        QString separator() const;

    public slots:
        void setSeparator(const QString &separator);

    protected:
        QStringList splitPath(const QString &path) const;
        QString pathFromIndex(const QModelIndex &index) const;

    private:
        QString sep;
};

#endif // TREEMODELCOMPLETER_H

