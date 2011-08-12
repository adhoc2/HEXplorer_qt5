#include "myqlineedit.h"

MyQLineEdit::MyQLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setAcceptDrops(true);
}

void MyQLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}


void MyQLineEdit::dropEvent(QDropEvent *event)
{
    QString str = text();

    if (event->mimeData()->hasText())
    {
        QString text = event->mimeData()->text();
        setText(text);
    }

    event->acceptProposedAction();

    emit textDropped(str);
}

