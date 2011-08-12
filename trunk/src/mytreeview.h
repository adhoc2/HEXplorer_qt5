#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QtGui>

class MyTreeView : public QTreeView
{

public:
    MyTreeView(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void startDrag();

    QPoint startPos;
};

#endif // MYTREEVIEW_H
