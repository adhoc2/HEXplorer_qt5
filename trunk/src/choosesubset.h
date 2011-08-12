#ifndef CHOOSESUBSET_H
#define CHOOSESUBSET_H

#include <QDialog>
#include <QStringlist>
class A2LFILE;
class HexFile;

namespace Ui {
    class ChooseSubset;
}

class ChooseSubset : public QDialog {
    Q_OBJECT
public:
    ChooseSubset(A2LFILE *_a2l, HexFile *_hex, QStringList &list,QWidget *parent = 0);
    ~ChooseSubset();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ChooseSubset *ui;
    A2LFILE *a2l;
    HexFile *hex;
    QWidget *mainWidget;
    QStringList *subsetList;

private slots:
    void on_export_2_clicked();
    void on_import_2_clicked();
    void on_lineEdit_textChanged(QString );
    void on_leftButton_clicked();
    void on_add_all_clicked();
    void on_remove_all_clicked();
    void on_buttonBox_accepted();
    void on_rightButton_clicked();
};

#endif // CHOOSESUBSET_H
