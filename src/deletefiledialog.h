#ifndef DELETEFILEDIALOG_H
#define DELETEFILEDIALOG_H

#include <QDialog>

namespace Ui {
class DeleteFileDialog;
}

class DeleteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteFileDialog(QWidget *parent = 0);
    ~DeleteFileDialog();

    bool deletePermanently();
    void setFileNames(QStringList list);

private:
    Ui::DeleteFileDialog *ui;
};

#endif // DELETEFILEDIALOG_H
