#ifndef DIALOGCSV_H
#define DIALOGCSV_H

#include "ui_dialogcsv.h"
class A2lTreeModel;


class DialogCsv : public QDialog, private Ui::DialogCsv
{
    Q_OBJECT

public:
    explicit DialogCsv(QWidget *parent = 0, QStringList *list = 0);
    void setModel(A2lTreeModel *mod);

private:
    QStringList *listlabel;
    A2lTreeModel *model;

private slots:
    void on_add_all_clicked();
    void on_remove_all_clicked();
    void on_buttonBox_accepted();
};

#endif // DIALOGCSV_H
