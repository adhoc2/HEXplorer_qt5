#ifndef LABELPROPERTIES_H
#define LABELPROPERTIES_H

#include <QDialog>
#include "data.h"

namespace Ui {
class LabelProperties;
}

class LabelProperties : public QDialog
{
    Q_OBJECT

public:
    explicit LabelProperties(Data *data, QWidget *parent = 0);
    ~LabelProperties();

private:
    Ui::LabelProperties *ui;
    Data* data;
    displayProperties();
};

#endif // LABELPROPERTIES_H
