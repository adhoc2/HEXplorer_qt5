#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

    void addItem(QString str);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
