// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
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

#include "comboboxdelegate.h"
#include "data.h"
#include "sptablemodel.h"
#include "comparemodel.h"
#include "graphmodel.h"
#include "obdMergeModel.h"
#include "Nodes/compu_vtab.h"
#include "spreadsheetview.h"
#include "obdsortfilterproxymodel.h"
#include <typeinfo>
#include <QtWidgets>


 ComboBoxDelegate::ComboBoxDelegate(QObject *parent)  : QItemDelegate(parent)
 {
     _parent = parent;
 }

 QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
 {
     QString name = typeid(*index.model()).name();
     if (name.toLower().endsWith("sptablemodel"))
     {
         QList<Data*> *dat = ((SpTableModel*)index.model())->getList();

         // find the selected label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat->at(ind)->size;
         Data *data = dat->at(ind);


         // define Editor
         if (data->xCount() == 0) //Value
         {
             if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisZ()->getValueList());
                 return editor;
             }
             else
                 return QItemDelegate::createEditor(parent, option, index);

         }
         else if (data->xCount() > 0 && data->yCount() == 0) //Curve
         {
             if (row - sum == 2 && column >= 1 && column <= data->xCount()
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisX()->getValueList());
                 return editor;
             }
             else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisZ()->getValueList());
                 return editor;
             }
             else
                 return QItemDelegate::createEditor(parent, option, index);
         }
         else if (data->xCount() > 0 && data->yCount() > 0) //Map
         {
             if (row - sum == 2 && column >= 2 && column <= data->xCount() + 1
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisX()->getValueList());
                 return editor;
             }
             else if (row - sum >= 3 && row - sum < data->size - 1)
             {
                 if (column == 1 && data->getCompuTabAxisY() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisY()->getValueList());
                     return editor;
                 }
                 else if (column >= 2 && column <= data->xCount() + 1
                          && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisZ()->getValueList());
                     return editor;
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
             else
                 return QItemDelegate::createEditor(parent, option, index);
         }
     }
     else if (name.toLower().endsWith("comparemodel"))
     {
         QList<Data*> *dat1 = ((CompareModel*)index.model())->getList1();
         QList<Data*> *dat2 = ((CompareModel*)index.model())->getList2();

         // determine the label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat1->at(ind)->size + dat2->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat1->at(ind)->size + dat2->at(ind)->size;

         // determine if dat1 or dat2
         bool bdata1 = true;
         if (row - sum >= dat1->at(ind)->size)
         {
             sum += dat1->at(ind)->size;
             bdata1 = false;
         }

         // define Editor
         if (bdata1)
         {
             Data *data = dat1->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisZ()->getValueList());
                     return editor;
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 2 && column >= 1 && column <= data->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisX()->getValueList());
                     return editor;
                 }
                 else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisZ()->getValueList());
                     return editor;
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
             else if (data->xCount() > 0 && data->yCount() > 0) //Map
             {
                 if (row - sum == 2 && column >= 2 && column <= dat1->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisX()->getValueList());
                     return editor;
                 }
                 else if (row - sum >= 3 && row - sum < dat1->at(ind)->size - 1)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *editor = new QComboBox(parent);
                         editor->addItems(data->getCompuTabAxisY()->getValueList());
                         return editor;
                     }
                     else if (column >= 2 && column <= dat1->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *editor = new QComboBox(parent);
                         editor->addItems(data->getCompuTabAxisZ()->getValueList());
                         return editor;
                     }
                     else
                         return QItemDelegate::createEditor(parent, option, index);
                 }
                 else
                 {
                     return QItemDelegate::createEditor(parent, option, index);
                 }
             }
         }
         else
         {
             Data *data = dat2->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 1 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisZ()->getValueList());
                     return editor;
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 1 && column >= 1 && column <= dat2->at(ind)->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisX()->getValueList());
                     return editor;
                 }
                 else if (row - sum == 2 && column >= 1 &&  column <= dat2->at(ind)->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisZ()->getValueList());
                     return editor;
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
             else if (dat2->at(ind)->xCount() > 0 && dat2->at(ind)->yCount() > 0) //Map
             {
                 if (row - sum == 1 && column >= 2 && column <= dat2->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *editor = new QComboBox(parent);
                     editor->addItems(data->getCompuTabAxisX()->getValueList());
                     return editor;
                 }
                 else if (row - sum >= 2 && row - sum < dat2->at(ind)->size - 2)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *editor = new QComboBox(parent);
                         editor->addItems(data->getCompuTabAxisY()->getValueList());
                         return editor;
                     }
                     else if (column >= 2 && column <= dat2->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *editor = new QComboBox(parent);
                         editor->addItems(data->getCompuTabAxisZ()->getValueList());
                         return editor;
                     }
                     else
                         return QItemDelegate::createEditor(parent, option, index);
                 }
                 else
                     return QItemDelegate::createEditor(parent, option, index);
             }
         }

     }
     else if (name.toLower().endsWith("graphmodel"))
     {
         return QItemDelegate::createEditor(parent, option, index);
     }
     else if (name.toLower().endsWith("obdmergemodel"))
     {
         QList<Data*> *dat = ((SpTableModel*)index.model())->getList();

         // find the selected label in function of the row
         int row = index.row();
         int column = index.column();
         Data *data = ((ObdMergeModel*)index.model())->getData(row, column);
         if (!data) return QItemDelegate::createEditor(parent, option, index);

         // define Editor
         if (data->xCount() == 0) //Value
         {
             if (data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisZ()->getValueList());
                 return editor;
             }
             else
                 return QItemDelegate::createEditor(parent, option, index);

         }
     }
     else if (name.toLower().endsWith("obdsortfilterproxymodel"))
     {
         Data *data = ((obdSortFilterProxyModel*)index.model())->getData(index);
         if (!data) return QItemDelegate::createEditor(parent, option, index);



         // define Editor
         if (data->xCount() == 0) //Value
         {
             if (data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *editor = new QComboBox(parent);
                 editor->addItems(data->getCompuTabAxisZ()->getValueList());
                 return editor;
             }
             else
                 return QItemDelegate::createEditor(parent, option, index);
         }
     }
     else
     {
         return QItemDelegate::createEditor(parent, option, index);
     }
 }

 void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
 {
     QString value = index.model()->data(index, Qt::EditRole).toString();
     QList<Data*> *dat = ((SpTableModel*)index.model())->getList();

     QString name = typeid(*index.model()).name();
     if (name.toLower().endsWith("sptablemodel"))
     {
         // determine the label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat->at(ind)->size;
         Data *data = dat->at(ind);

         // setData in Editor
         if (data->xCount() == 0) //Value
         {
             if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 int index = 0;
                 QStringList list = data->getCompuTabAxisZ()->getValueList();
                 index = list.indexOf(value);
                 combo->setCurrentIndex(index);
             }
             else
                QItemDelegate::setEditorData(editor, index);

         }
         else if (data->xCount() > 0 && data->yCount() == 0) //Curve
         {
             if (row - sum == 2 && column >= 1 && column <= data->xCount()
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 int index = 0;
                 QStringList list = data->getCompuTabAxisX()->getValueList();
                 index = list.indexOf(value);
                 combo->setCurrentIndex(index);
             }
             else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 int index = 0;
                 QStringList list = data->getCompuTabAxisZ()->getValueList();
                 index = list.indexOf(value);
                 combo->setCurrentIndex(index);
             }
             else
                 QItemDelegate::setEditorData(editor, index);
         }
         else if (data->xCount() > 0 && data->yCount() > 0) //Map
         {
             if (row - sum == 2 && column >= 2 && column <= data->xCount() + 1
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 int index = 0;
                 QStringList list = data->getCompuTabAxisX()->getValueList();
                 index = list.indexOf(value);
                 combo->setCurrentIndex(index);
             }
             else if (row - sum >= 3 && row - sum < data->size - 1)
             {
                 if (column == 1 && data->getCompuTabAxisY() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisY()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else if (column >= 2 && column <= data->xCount() + 1
                          && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisZ()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else
                     QItemDelegate::setEditorData(editor, index);
             }
             else
                 QItemDelegate::setEditorData(editor, index);
         }

     }
     else if (name.toLower().endsWith("comparemodel"))
     {
         QList<Data*> *dat1 = ((CompareModel*)index.model())->getList1();
         QList<Data*> *dat2 = ((CompareModel*)index.model())->getList2();

         // determine the label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat1->at(ind)->size + dat2->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat1->at(ind)->size + dat2->at(ind)->size;

         // determine if dat1 or dat2
         bool bdata1 = true;
         if (row - sum >= dat1->at(ind)->size)
         {
             sum += dat1->at(ind)->size;
             bdata1 = false;
         }

         // define Editor
         if (bdata1)
         {
             Data *data = dat1->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisZ()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else
                    QItemDelegate::setEditorData(editor, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 2 && column >= 1 && column <= data->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisX()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisZ()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else
                      QItemDelegate::setEditorData(editor, index);
             }
             else if (data->xCount() > 0 && data->yCount() > 0) //Map
             {
                 if (row - sum == 2 && column >= 2 && column <= dat1->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisX()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else if (row - sum >= 3 && row - sum < dat1->at(ind)->size - 1)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         int index = 0;
                         QStringList list = data->getCompuTabAxisY()->getValueList();
                         index = list.indexOf(value);
                         combo->setCurrentIndex(index);
                     }
                     else if (column >= 2 && column <= dat1->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         int index = 0;
                         QStringList list = data->getCompuTabAxisZ()->getValueList();
                         index = list.indexOf(value);
                         combo->setCurrentIndex(index);
                     }
                     else
                         QItemDelegate::setEditorData(editor, index);
                 }
                 else
                 {
                     QItemDelegate::setEditorData(editor, index);
                 }
             }
         }
         else
         {
             Data *data = dat2->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 1 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisZ()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else
                      QItemDelegate::setEditorData(editor, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 1 && column >= 1 && column <= dat2->at(ind)->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisX()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else if (row - sum == 2 && column >= 1 &&  column <= dat2->at(ind)->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisZ()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else
                     QItemDelegate::setEditorData(editor, index);
             }
             else if (dat2->at(ind)->xCount() > 0 && dat2->at(ind)->yCount() > 0) //Map
             {
                 if (row - sum == 1 && column >= 2 && column <= dat2->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     int index = 0;
                     QStringList list = data->getCompuTabAxisX()->getValueList();
                     index = list.indexOf(value);
                     combo->setCurrentIndex(index);
                 }
                 else if (row - sum >= 2 && row - sum < dat2->at(ind)->size - 2)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         int index = 0;
                         QStringList list = data->getCompuTabAxisY()->getValueList();
                         index = list.indexOf(value);
                         combo->setCurrentIndex(index);
                     }
                     else if (column >= 2 && column <= dat2->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         int index = 0;
                         QStringList list = data->getCompuTabAxisZ()->getValueList();
                         index = list.indexOf(value);
                         combo->setCurrentIndex(index);
                     }
                     else
                         QItemDelegate::setEditorData(editor, index);
                 }
                 else
                 {
                     QItemDelegate::setEditorData(editor, index);
                 }
             }
         }

     }
     else if (name.toLower().endsWith("graphmodel"))
     {
         QItemDelegate::setEditorData(editor, index);
     }
     else
     {
         QItemDelegate::setEditorData(editor, index);
     }
 }

 void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
     QList<Data*> *dat = ((SpTableModel*)index.model())->getList();

     QString name = typeid(*index.model()).name();
     if (name.toLower().endsWith("sptablemodel"))
     {
         // determine the label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat->at(ind)->size;
         Data *data = dat->at(ind);

         // setData in Editor
         if (data->xCount() == 0) //Value
         {
             if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 ((SpTableModel*)model)->setData(index, combo->currentText(), Qt::EditRole);

             }
             else
                QItemDelegate::setModelData(editor, model, index);

         }
         else if (data->xCount() > 0 && data->yCount() == 0) //Curve
         {
             if (row - sum == 2 && column >= 1 && column <= data->xCount()
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 model->setData(index, combo->currentText(), Qt::EditRole);
             }
             else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                 && data->getCompuTabAxisZ() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 model->setData(index, combo->currentText(), Qt::EditRole);
             }
             else
                 QItemDelegate::setModelData(editor, model, index);
         }
         else if (data->xCount() > 0 && data->yCount() > 0) //Map
         {
             if (row - sum == 2 && column >= 2 && column <= data->xCount() + 1
                 && data->getCompuTabAxisX() != NULL)
             {
                 QComboBox *combo = static_cast<QComboBox*>(editor);
                 model->setData(index, combo->currentText(), Qt::EditRole);
             }
             else if (row - sum >= 3 && row - sum < data->size - 1)
             {
                 if (column == 1 && data->getCompuTabAxisY() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     model->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else if (column >= 2 && column <= data->xCount() + 1
                          && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     model->setData(index, combo->currentText(), Qt::EditRole);                     
                 }
                 else
                     QItemDelegate::setModelData(editor, model, index);
             }
             else
                 QItemDelegate::setModelData(editor, model, index);
         }

     }
     else if (name.toLower().endsWith("comparemodel"))
     {
         QList<Data*> *dat1 = ((CompareModel*)index.model())->getList1();
         QList<Data*> *dat2 = ((CompareModel*)index.model())->getList2();

         // determine the label in function of the row
         int row = index.row();
         int column = index.column();
         int ind = 0;
         int sum = 0;
         while (row >= sum)
         {
             sum += dat1->at(ind)->size + dat2->at(ind)->size;
             ind++;
         }
         ind--;
         sum -= dat1->at(ind)->size + dat2->at(ind)->size;

         // determine if dat1 or dat2
         bool bdata1 = true;
         if (row - sum >= dat1->at(ind)->size)
         {
             sum += dat1->at(ind)->size;
             bdata1 = false;
         }

         // define Editor
         if (bdata1)
         {
             Data *data = dat1->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 2 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else
                    QItemDelegate::setModelData(editor, model, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 2 && column >= 1 && column <= data->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else if (row - sum == 3 && column >= 1 &&  column <= data->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else
                      QItemDelegate::setModelData(editor, model, index);
             }
             else if (data->xCount() > 0 && data->yCount() > 0) //Map
             {
                 if (row - sum == 2 && column >= 2 && column <= dat1->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else if (row - sum >= 3 && row - sum < dat1->at(ind)->size - 1)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                     }
                     else if (column >= 2 && column <= dat1->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                     }
                     else
                         QItemDelegate::setModelData(editor, model, index);
                 }
                 else
                     QItemDelegate::setModelData(editor, model, index);
             }
         }
         else
         {
             Data *data = dat2->at(ind);
             if (data->xCount() == 0) //Value
             {
                 if (row - sum == 1 && column == 1 && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else
                      QItemDelegate::setModelData(editor, model, index);
             }
             else if (data->xCount() > 0 && data->yCount() == 0) //Curve
             {
                 if (row - sum == 1 && column >= 1 && column <= dat2->at(ind)->xCount()
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else if (row - sum == 2 && column >= 1 &&  column <= dat2->at(ind)->zCount()
                     && data->getCompuTabAxisZ() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else
                     QItemDelegate::setModelData(editor, model, index);
             }
             else if (dat2->at(ind)->xCount() > 0 && dat2->at(ind)->yCount() > 0) //Map
             {
                 if (row - sum == 1 && column >= 2 && column <= dat2->at(ind)->xCount() + 1
                     && data->getCompuTabAxisX() != NULL)
                 {
                     QComboBox *combo = static_cast<QComboBox*>(editor);
                     ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                 }
                 else if (row - sum >= 2 && row - sum < dat2->at(ind)->size - 2)
                 {
                     if (column == 1 && data->getCompuTabAxisY() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                     }
                     else if (column >= 2 && column <= dat2->at(ind)->xCount() + 1
                              && data->getCompuTabAxisZ() != NULL)
                     {
                         QComboBox *combo = static_cast<QComboBox*>(editor);
                         ((CompareModel*)model)->setData(index, combo->currentText(), Qt::EditRole);
                     }
                     else
                         QItemDelegate::setModelData(editor, model, index);
                 }
                 else
                     QItemDelegate::setModelData(editor, model, index);
             }
         }

     }
     else if (name.toLower().endsWith("graphmodel"))
     {
         QItemDelegate::setModelData(editor, model, index);
     }
     else
     {
         QItemDelegate::setModelData(editor, model, index);
     }
 }

 void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }
