#ifndef MYQLINEEDIT_H
#define MYQLINEEDIT_H

#include <QtGui>


class MyQLineEdit : public QLineEdit
{    
    Q_OBJECT

    public:
        MyQLineEdit(QWidget *parent = 0);


    private:
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);

    signals:
      void textDropped(QString );

};

#endif // MYQLINEEDIT_H
