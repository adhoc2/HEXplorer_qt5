#include "choosesubset.h"
#include "ui_choosesubset.h"
#include "Nodes/a2lfile.h"
#include "hexfile.h"
#include "Nodes/def_characteristic.h"
#include "formcompare.h"
#include "mdimain.h"
#include <QFileDialog>
#include <QMessageBox>

ChooseSubset::ChooseSubset(A2LFILE *_a2l, HexFile *_hex, QStringList &list, QWidget *parent) : QDialog(parent), ui(new Ui::ChooseSubset)
{    
    // setup Ui
    ui->setupUi(this);
    setWindowTitle("HEXplorer :: select Subsets into " + QString(_a2l->name));
    QIcon icon(":/icones/Add record.png");
    setWindowIcon(icon);

    //initialize pointers
    mainWidget = parent;
    a2l = _a2l;
    hex = _hex;
    subsetList = &list;


    //define the 2 QListWidget as Sorted
    ui->listWidget->setSortingEnabled(true);
    ui->listWidget_2->setSortingEnabled(true);

    // select the text from lineEdit
    ui->lineEdit->setText("enter a subset name ...");
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    ui->lineEdit->selectAll();

    QList<QKeySequence> listShortCutsLeft;
    listShortCutsLeft.append(Qt::Key_Right);
    listShortCutsLeft.append(Qt::Key_Space);

    QAction *rightSelect = new QAction(ui->listWidget);
    rightSelect->setShortcutContext(Qt::WidgetShortcut);
    rightSelect->setShortcuts(listShortCutsLeft);
    connect(rightSelect, SIGNAL(triggered()), this, SLOT(on_rightButton_clicked()));

    ui->listWidget->addAction(rightSelect);

}

ChooseSubset::~ChooseSubset()
{
    delete ui;
}

void ChooseSubset::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChooseSubset::on_lineEdit_textChanged(QString )
{
    QString moduleName = hex->getModuleName();
    Node *fun = a2l->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");

    //display the FUNCTION starting with text
    if (fun != NULL && ui->lineEdit->text().size() > 0 && !ui->lineEdit->text().startsWith('*'))
    {
        ui->listWidget->clear();

        QStringList str;
        QString name;

        QStringList choosenList;
        for (int i = 0; i < ui->listWidget_2->count(); i++)
            choosenList.append(ui->listWidget_2->item(i)->text());

        foreach (Node *node, fun->childNodes)
        {
            name = node->name;
            QStringList::iterator i;
            if (name.toLower().startsWith(ui->lineEdit->text().toLower()))
            {
                //check if the labels are already chosen or not
                i = qBinaryFind(choosenList.begin(), choosenList.end(), name);
                if (i == choosenList.end())
                    str.append(name);
            }
        }

        ui->listWidget->addItems(str);
    }
    else if (fun != NULL && ui->lineEdit->text().size() > 0 && ui->lineEdit->text().startsWith('*'))
    {
        ui->listWidget->clear();

        QStringList str;
        QString name;

        QStringList choosenList;
        for (int i = 0; i < ui->listWidget_2->count(); i++)
            choosenList.append(ui->listWidget_2->item(i)->text());

        foreach (Node *node, fun->childNodes)
        {
            name = node->name;
            QStringList::iterator i;
            if (name.toLower().endsWith(ui->lineEdit->text().mid(1).toLower()))
            {
                //check if the labels are already chosen or not
                i = qBinaryFind(choosenList.begin(), choosenList.end(), name);
                if (i == choosenList.end())
                    str.append(name);
            }
        }       

        ui->listWidget->addItems(str);
    }
    else
    {
        ui->listWidget->clear();
    }

}

void ChooseSubset::on_rightButton_clicked()
{
    foreach(QListWidgetItem *item, ui->listWidget->selectedItems())
    {
        QListWidgetItem *newItem = item->clone();
        ui->listWidget_2->addItem(newItem);
        delete item;
    }
}

void ChooseSubset::on_remove_all_clicked()
{
    ui->listWidget_2->clear();
    QString str;
    on_lineEdit_textChanged(str);
}

void ChooseSubset::on_add_all_clicked()
{
    ui->listWidget->selectAll();
    on_rightButton_clicked();
}

void ChooseSubset::on_leftButton_clicked()
{
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
    {
        QListWidgetItem *newItem = item->clone();
        ui->listWidget->addItem(newItem);
        delete item;
    }
}

void ChooseSubset::on_buttonBox_accepted()
{
    ui->listWidget_2->selectAll();
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
        subsetList->append(item->text());
}

void ChooseSubset::on_import_2_clicked()
{
    QString moduleName = hex->getModuleName();

    //select a File from disk
    QString filename = QFileDialog::getOpenFileName(this,
                                      tr("select a lab file (lab)"), ".",
                                      tr("ASAP files (*.subset);all files (*.*)"));

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    //read the file and save it into a QStringList
    QStringList lines;
    while (!in.atEnd())
       lines << in.readLine();

    file.close();

    if (lines.count() != 0)
    {
        if (lines.at(0) != "[Subset]")
            return;
        else
        {
            Node *node = a2l->getProject()->getNode("MODULE/" + moduleName + "/FUNCTION");
            if (node)
            {
                QStringList choosenList;
                for (int i = 0; i < ui->listWidget_2->count(); i++)
                    choosenList.append(ui->listWidget_2->item(i)->text());

                QStringList strList;

                for (int i = 1;i < lines.count(); i++)
                {
                    if (node->isChild(lines.at(i).toLocal8Bit().data()))
                    {
                        //check if label already in chosenList
                        QStringList::iterator it = qBinaryFind(choosenList.begin(), choosenList.end(), lines.at(i));
                        if (it == choosenList.end())
                            strList.append(lines.at(i));
                    }
                }
                ui->listWidget->addItems(strList);
            }
        }
    }
    else
        return;
}

void ChooseSubset::on_export_2_clicked()
{
    QStringList strList;
    strList.append("[Subset]");
    ui->listWidget_2->selectAll();
    foreach(QListWidgetItem *item, ui->listWidget_2->selectedItems())
        strList.append(item->text());

    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach (QString str, strList)
        out << str << "\r\n";
    QApplication::restoreOverrideCursor();
}
