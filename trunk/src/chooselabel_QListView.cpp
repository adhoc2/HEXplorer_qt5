#include "chooselabel.h"
#include "ui_chooselabel.h"
#include "Nodes/a2lfile.h"
#include "Nodes/def_characteristic.h"
#include "formcompare.h"
#include "mdimain.h"

ChooseLabel::ChooseLabel(A2LFILE *_a2l , QWidget *parent) :
    QDialog(parent), ui(new Ui::ChooseLabel)
{
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer::select Labels");
    QIcon icon(":/icones/Add record.ico");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2lSrc = _a2l;

    // activate the right radioButton
    QString list1(a2lSrc->name);
    ui->radioButton->setText(list1);
    ui->radioButton->setChecked(true);

    // copy the charList from formComapare into choosenList and listView_2
    model1 = new QStringListModel();
    model2 = new QStringListModel();
    FormCompare *fc = (FormCompare*)mainWidget;
    list2 = fc->charList;
    model2->setStringList(list2);
    ui->listWidget_2->setModel(model2);
}

ChooseLabel::~ChooseLabel()
{
    delete ui;
}

void ChooseLabel::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void ChooseLabel::on_lineEdit_textChanged(QString )
{
    //get CHARACTERISTIC node
    A2LFILE *a2l = a2lSrc;

    list1.clear();

    // display FUNCTIONS (subset)
    if (ui->lineEdit->text().startsWith('/') && ui->lineEdit->text().size() > 1)
    {
        Node *fun = a2l->getProject()->getNode("MODULE/DIM/FUNCTION");

        if (fun != NULL)
        {

            QString name;

            foreach (Node *node, fun->childNodes)
            {
                name = node->name;
                QString toto = ui->lineEdit->text().mid(1);
                if (name.toLower().startsWith(toto.toLower()))
                {
                    //get the label list
                    DEF_CHARACTERISTIC *node_char = (DEF_CHARACTERISTIC*)node->getNode("DEF_CHARACTERISTIC");
                    QStringList ll;
                    if (node_char != NULL)
                    {
                        ll = node_char->getCharList();

                        //do not display already chosen labels
                        QStringList::iterator i;
                        foreach(QString tutu, ll)
                        {
                            //check if the labels are already chosen or not
                            i = qBinaryFind(list2.begin(), list2.end(), tutu);
                            if (i == list2.end())
                                list1.append(tutu);
                        }
                    }
                }
            }
        }
    }
    else //display CHARACTERISTICS and AXIS_PTS
    {
        Node *charac = a2l->getProject()->getNode("MODULE/DIM/CHARACTERISTIC");
        Node *axisPts = a2l->getProject()->getNode("MODULE/DIM/AXIS_PTS");

        //display the CHARACTERISTICS starting with text
        if (charac != NULL && ui->lineEdit->text().size() > 0 && !ui->lineEdit->text().startsWith('*'))
        {           
            QString name;
            foreach (Node *node, charac->childNodes)
            {
                name = node->name;
                QStringList::iterator i;
                if (name.toLower().startsWith(ui->lineEdit->text().toLower()))
                {
                    //check if the labels are already chosen or not
                    i = qBinaryFind(list2.begin(), list2.end(), name);
                    if (i == list2.end())
                        list1.append(name);
                }
            }

            foreach (Node *node, axisPts->childNodes)
            {
                name = node->name;
                QStringList::iterator i;
                if (name.toLower().startsWith(ui->lineEdit->text().toLower()))
                {
                    //check if the labels are already chosen or not
                    i = qBinaryFind(list2.begin(), list2.end(), name);
                    if (i == list2.end())
                        list1.append(name);
                }
            }            
        }
        else if (charac != NULL && ui->lineEdit->text().size() > 0 && ui->lineEdit->text().startsWith('*'))
        {
            QString name;

            foreach (Node *node, charac->childNodes)
            {
                name = node->name;
                QStringList::iterator i;
                if (name.toLower().endsWith(ui->lineEdit->text().mid(1).toLower()))
                {
                    //check if the labels are already chosen or not
                    i = qBinaryFind(list2.begin(), list2.end(), name);
                    if (i == list2.end())
                        list1.append(name);
                }
            }

            foreach (Node *node, axisPts->childNodes)
            {
                name = node->name;
                QStringList::iterator i;
                if (name.toLower().endsWith(ui->lineEdit->text().mid(1).toLower()))
                {
                    //check if the labels are already chosen or not
                    i = qBinaryFind(list2.begin(), list2.end(), name);
                    if (i == list2.end())
                        list1.append(name);
                }
            }
        }       
    }

    list1.sort();
    model1->setStringList(list1);
    ui->listWidget->setModel(model1);
}

void ChooseLabel::on_rightButton_clicked()
{/*
    foreach(QListWidgetItem *item, ui->listWidget->selectedItems())
    {
        QListWidgetItem *newItem = item->clone();
        ui->listWidget_2->addItem(newItem);
        delete item;
    }
    */
}

void ChooseLabel::on_remove_all_clicked()
{
    list2.clear();
    model2->setStringList(list2);
    QString tt;
    on_lineEdit_textChanged(tt);
}

void ChooseLabel::on_add_all_clicked()
{
    list2.append(list1);
    model2->setStringList(list2);
    list1.clear();
    QString tt;
    on_lineEdit_textChanged(tt);
}

void ChooseLabel::on_leftButton_clicked()
{
    /*
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
    {
        QListWidgetItem *newItem = item->clone();
        ui->listWidget->addItem(newItem);
        delete item;
    }
    */
}

void ChooseLabel::on_buttonBox_accepted()
{

    FormCompare *fc = (FormCompare*)mainWidget;
    fc->charList.clear();
    fc->charList = list2;

}
