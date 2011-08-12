#ifndef CHOOSELABEL_H
#define CHOOSELABEL_H

#include <QDialog>
#include "a2ltreemodel.h"
#include <QListWidgetItem>
#include <QStringListModel>
#include "treemodelcompleter.h"
class QTreeView;
class A2LFILE;
class ProjectListWidget;

namespace Ui {
    class ChooseLabel;
}

class ChooseLabel : public QDialog
{
    Q_OBJECT

    public:
        ChooseLabel(A2LFILE *_a2l, QWidget *parent = 0);
        ~ChooseLabel();

    protected:
        void changeEvent(QEvent *e);

    private:
        QStringList list1;
        QStringList list2;
        QStringListModel *model1;
        QStringListModel *model2;
        QWidget *mainWidget;
        TreeModelCompleter *completer;
        Ui::ChooseLabel *ui;
        A2LFILE *a2lSrc;
        A2LFILE *a2lTrg;

    private slots:
        void on_leftButton_clicked();
        void on_add_all_clicked();
        void on_remove_all_clicked();
        void on_buttonBox_accepted();
        void on_lineEdit_textChanged(QString );
        void on_rightButton_clicked();
};

#endif // CHOOSELABEL_H
