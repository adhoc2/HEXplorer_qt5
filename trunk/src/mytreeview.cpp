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

#include "mytreeview.h"
#include "a2ltreemodel.h"

MyTreeView::MyTreeView(QWidget *parent) : QTreeView(parent)
{

    setDragEnabled(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);

}

void MyTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        startPos = event->pos();
    QTreeView::mousePressEvent(event);
}

void MyTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            startDrag();
    }
    QTreeView::mouseMoveEvent(event);
}



void MyTreeView::startDrag()
{
    //get selected items
    QModelIndexList list = selectionModel()->selectedIndexes();


    //save the items name in mimeData with a separator
    QMimeData *mimeData = new QMimeData;
    QString str;
    foreach(QModelIndex index, list)
    {
        A2lTreeModel *mod = (A2lTreeModel*)this->model();
        str.append(mod->getFullNodeName(index) + ";");
    }
    mimeData->setText(str);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/icones/person.png"));
}

