// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

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
