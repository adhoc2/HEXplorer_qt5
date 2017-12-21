#include "labelproperties.h"
#include "ui_labelproperties.h"

LabelProperties::LabelProperties(Data *data, QWidget *parent) : QDialog(parent), ui(new Ui::LabelProperties)
{
    ui->setupUi(this);
    this->data = data;
    displayProperties();

}

LabelProperties::~LabelProperties()
{
    delete ui;
}

LabelProperties::displayProperties()
{
    //count how many rows
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(2);
    Node *label = data->getA2lNode();

    if (label)
    {
        //display fix parameters
        QMap<std::string, std::string> *map = label->getParameters();
        ui->tableWidget->setRowCount(map->count());
        int i = 0;
        foreach (std::string str, map->keys())
        {
            QTableWidgetItem *item = new QTableWidgetItem(str.c_str());
            ui->tableWidget->setItem(i, 0, item);
            QTableWidgetItem *item1 = new QTableWidgetItem(QString(map->value(str).c_str()));
            ui->tableWidget->setItem(i, 1, item1);
            i++;
        }

        //check optional items (parameters)
        QMap<QString, QString> *mapOptItems = label->getOptItems();
        int count = mapOptItems->count();
        ui->tableWidget->setRowCount(map->count()+ count);
        foreach (QString str, mapOptItems->keys())
        {
            QTableWidgetItem *item = new QTableWidgetItem(str);
            ui->tableWidget->setItem(i, 0, item);
            QTableWidgetItem *item1 = new QTableWidgetItem(mapOptItems->value(str));
            ui->tableWidget->setItem(i, 1, item1);
            i++;
        }


        //check optional nodes
        QList<Node*> childNodes;
    }
}




