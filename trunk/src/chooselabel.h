#ifndef CHOOSELABEL_H
#define CHOOSELABEL_H

#include <QTimer>
#include <QDialog>
#include "a2ltreemodel.h"
#include <QListWidgetItem>
#include "treemodelcompleter.h"
class QTreeView;
class A2LFILE;
class HexFile;
class Csv;
class ProjectListWidget;
class CdfxFile;

namespace Ui {
    class ChooseLabel;
}

class ChooseLabel : public QDialog
{
    Q_OBJECT

    public:
        ChooseLabel(A2LFILE *_a2l, HexFile *_hex, QWidget *parent = 0);
        ChooseLabel(A2LFILE *_a2l, Csv *_csv, QWidget *parent = 0);
        ChooseLabel(A2LFILE *_a2l, CdfxFile *_cdfx, QWidget *parent = 0);
        ~ChooseLabel();

    protected:
        void changeEvent(QEvent *e);

    private:
        QTimer timer;
        QAction *deleteRight;
        QAction *leftSelect;
        QAction *rightSelect;
        QWidget *mainWidget;
        TreeModelCompleter *completer;
        Ui::ChooseLabel *ui;
        A2LFILE *a2l;
        HexFile *hex;
        Csv *csv;
        CdfxFile *cdfx;

        void createActions();

    private slots:
        void searchItem();
        void selectedItem(QListWidgetItem*,QListWidgetItem*);
        void on_export_2_clicked();
        void on_import_2_clicked();
        void on_leftButton_clicked();
        void on_add_all_clicked();
        void on_remove_all_clicked();
        void on_buttonBox_accepted();
        void on_lineEdit_textChanged(QString str = "");
        void on_rightButton_clicked();
};

#endif // CHOOSELABEL_H
