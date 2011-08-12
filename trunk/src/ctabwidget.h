#ifndef CTABWIDGET_H
#define CTABWIDGET_H

#include <QtGui>

class CTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    CTabWidget(QWidget *parent = 0);

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

signals:
  void hexDropped(QString );


};

#endif // CTABWIDGET_H
