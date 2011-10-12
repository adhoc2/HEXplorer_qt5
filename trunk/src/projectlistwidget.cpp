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

#include <QtGui>
#include <typeinfo>
#include "projectlistwidget.h"

ProjectListWidget::ProjectListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);
}

void ProjectListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        startPos = event->pos();
    QListWidget::mousePressEvent(event);
}

void ProjectListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            startDrag();
    }
    QListWidget::mouseMoveEvent(event);
}

void ProjectListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QString name = typeid(*event->source()).name();
    if (name.toLower().endsWith("projectlistwidget"))
    {
        ProjectListWidget *source = qobject_cast<ProjectListWidget *>(event->source());
        if (source && source != this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
    else if (name.toLower().endsWith("mytreeview"))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void ProjectListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QString name = typeid(*event->source()).name();
    if (name.toLower().endsWith("projectlistwidget"))
    {
        ProjectListWidget *source =  qobject_cast<ProjectListWidget *>(event->source());
        if (source && source != this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
    else if (name.toLower().endsWith("mytreeview"))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }

}

void ProjectListWidget::dropEvent(QDropEvent *event)
{
    QString name = typeid(*event->source()).name();
    if (name.toLower().endsWith("projectlistwidget"))
    {
        ProjectListWidget *source =  qobject_cast<ProjectListWidget *>(event->source());
        if (source && source != this)
        {
                QStringList list = event->mimeData()->text().split(QLatin1String("/"));
                foreach(QString str, list)
                {
                    if (!str.isEmpty())
                        addItem(str);
                }
                event->setDropAction(Qt::MoveAction);
                event->accept();
        }
    }
   else if (name.toLower().endsWith("mytreeview"))
   {
        QStringList list = event->mimeData()->text().split(QLatin1String(";"));
        foreach(QString str, list)
        {
            if (!str.isEmpty())
                addItem(str);
        }
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void ProjectListWidget::startDrag()
{
    //get selected items
    QList<QListWidgetItem*> list = selectedItems();    

    //save the items name in mimeData with a separator
    QMimeData *mimeData = new QMimeData;
    QString str;
    foreach(QListWidgetItem* item, list)
    {        
        str.append(item->text() + "/");
    }
    mimeData->setText(str);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/icones/person.png"));
    if (drag->start(Qt::MoveAction) == Qt::MoveAction)
    {
        foreach(QListWidgetItem* item, list)
        {
            delete item;
        }
    }
}
