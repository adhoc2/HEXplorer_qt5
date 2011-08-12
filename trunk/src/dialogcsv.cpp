#include "dialogcsv.h"
#include "node.h"
#include "a2ltreemodel.h"

DialogCsv::DialogCsv(QWidget *parent, QStringList *list) :   QDialog(parent)
{
    setupUi(this);

    //Model
    model = NULL;
    this->listWidget->setDragEnabled(true);
    this->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->listWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    this->listWidget_2->setAcceptDrops(true);

    listlabel = list;

}

void DialogCsv::setModel(A2lTreeModel *mod)
{
    this->model = mod;
    this->listWidget->setModel(mod);
    this->listWidget->setColumnHidden(1, true);
}

void DialogCsv::on_buttonBox_accepted()
{
    listWidget_2->selectAll();
    foreach(QListWidgetItem *item, listWidget_2->selectedItems())
        listlabel->append(item->text());
}

void DialogCsv::on_remove_all_clicked()
{
    this->listWidget_2->clear();
}

void DialogCsv::on_add_all_clicked()
{
    Node *root = model->getRootNode();
    foreach (Node *node, root->childNodes)
    {
        foreach(Node *data, node->childNodes)
        {
            QModelIndex index = model->getIndex(data);
            listWidget_2->addItem(model->getFullNodeName(index));
        }
    }
}
