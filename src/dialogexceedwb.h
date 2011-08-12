#ifndef DIALOGEXCEEDWB_H
#define DIALOGEXCEEDWB_H

#include "ui_dialogexceedwb.h"

class DialogExceedWB : public QDialog, private Ui::DialogExceedWB
{
    Q_OBJECT

public:
    explicit DialogExceedWB(bool *bl, QWidget *parent = 0);

private:
    bool *again;

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // DIALOGEXCEEDWB_H
