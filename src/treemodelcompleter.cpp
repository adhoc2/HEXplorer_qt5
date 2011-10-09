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

#include "treemodelcompleter.h"
#include <QStringList>

TreeModelCompleter::TreeModelCompleter(QObject *parent)
    : QCompleter(parent)
{
}

TreeModelCompleter::TreeModelCompleter(QAbstractItemModel *model, QObject *parent)
    : QCompleter(model, parent)
{
}


void TreeModelCompleter::setSeparator(const QString &separator)
{
    sep = separator;
}


QString TreeModelCompleter::separator() const
{
    return sep;
}

QStringList TreeModelCompleter::splitPath(const QString &path) const
{
    if (sep.isNull()) {
        return QCompleter::splitPath(path);
    }

    return path.split(sep);
}

QString TreeModelCompleter::pathFromIndex(const QModelIndex &index) const
{
    if (sep.isNull())
    {
        return QCompleter::pathFromIndex(index);
    }

    // navigate up and accumulate data
    QStringList dataList;
    for (QModelIndex i = index; i.isValid(); i = i.parent())
    {
        dataList.prepend(model()->data(i, completionRole()).toString());
    }

    return dataList.join(sep);
}
