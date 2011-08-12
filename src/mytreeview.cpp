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

