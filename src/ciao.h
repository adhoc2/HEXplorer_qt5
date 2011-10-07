#ifndef CIAO_H
#define CIAO_H

#include "ui_ciao.h"

class Ciao : public QDialog, private Ui::Ciao
{
    Q_OBJECT

public:
    explicit Ciao(bool &bl, QWidget *parent = 0);
private slots:
    void on_checkBox_clicked();
    void on_buttonBox_accepted();

private:
    bool* _bool;
};

#endif // CIAO_H
