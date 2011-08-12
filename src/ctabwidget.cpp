#include "ctabwidget.h"

CTabWidget::CTabWidget(QWidget *parent) : QTabWidget(parent)
{
    setAcceptDrops(true);
}

void CTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}


void CTabWidget::dropEvent(QDropEvent *event)
{
    QString text;
    if (event->mimeData()->hasText())
    {
       text = event->mimeData()->text();
    }

    event->acceptProposedAction();

    emit hexDropped(text);
}
