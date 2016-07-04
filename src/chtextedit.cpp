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

#include <QAction>
#include <QDir>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include "chtextedit.h"
#include "Qsci/qsciscintilla.h"


ChTextEdit::ChTextEdit(QWidget *parent) : QsciScintilla(parent)
{
    this->setAcceptDrops(false);

    //recalculate the lineNumberColumnWidth
    connect(this, SIGNAL(textChanged()), this, SLOT(refresh()));

    //Context Menu
    find = new QAction(tr("find"), this);
    find->setIcon(QIcon(":/icones/find.png"));
    find->setStatusTip(tr("find"));
    find->setShortcut(tr("Ctrl+F"));
    find->setShortcutContext(Qt::WidgetShortcut);
    connect(find, SIGNAL(triggered()), this, SLOT(openFindDialog()));
    this->addAction(find);

    findPrevious = new QAction(tr("find previous"), this);
    findPrevious->setIcon(QIcon(":/icones/Go-Previous.png"));
    findPrevious->setShortcut(Qt::Key_F1);
    findPrevious->setStatusTip(tr("find previous"));
    findPrevious->setDisabled(true);
    connect(findPrevious, SIGNAL(triggered()), this, SLOT(findPreviousText()));
    this->addAction(findPrevious);

    findNext = new QAction(tr("find next"), this);
    findNext->setIcon(QIcon(":/icones/Go-Next.png"));
    findNext->setShortcut(Qt::Key_F2);
    findNext->setStatusTip(tr("find next"));
    findNext->setDisabled(true);
    connect(findNext, SIGNAL(triggered()), this, SLOT(findNextText()));
    this->addAction(findNext);

    goToLine = new QAction(tr("go to line"), this);
    goToLine->setIcon(QIcon(":/icones/find.png"));
    goToLine->setStatusTip(tr("go to line"));
    connect(goToLine, SIGNAL(triggered()), this, SLOT(gotoline()));
    this->addAction(goToLine);

    saveAct = new QAction(tr("save"), this);
    saveAct->setIcon(QIcon(":/icones/save.png"));
    saveAct->setStatusTip(tr("save"));
    saveAct->setShortcut(tr("Ctrl+S"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    this->addAction(saveAct);

    saveAsAct = new QAction(tr("save as"), this);
    saveAsAct->setIcon(QIcon(":/icones/save.png"));
    saveAsAct->setStatusTip(tr("save as"));
    saveAsAct->setShortcut(tr("Ctrl+W"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    this->addAction(saveAsAct);

    //congigure context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuEvent(QPoint)));

    // Set the textEditor
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(8);
    this->setFont(font);
    this->setMarginLineNumbers(1, true);
}

void ChTextEdit::contextMenuEvent ( QPoint p )
{
    QMenu *menu = new QMenu(this);

    menu->addAction(saveAct);
    menu->addAction(saveAsAct);
    menu->addSeparator();
    menu->addAction(find);
    menu->addAction(findPrevious);
    menu->addAction(findNext);
    menu->addSeparator();
    menu->addAction(goToLine);

    menu->exec(QCursor::pos());
}

void ChTextEdit::openFindDialog()
{
     bool ok;
     searchText = QInputDialog::getText(this, tr("HEXplorer :: find()"),
                                          tr("enter your text:"), QLineEdit::Normal,
                                          "your text", &ok);
     if (ok && !searchText.isEmpty())
     {
         this->findFirst(searchText, false, false, false, false, true, 0, 0, true);
         this->findNext->setDisabled(false);
         this->findPrevious->setDisabled(false);
     }

}

void ChTextEdit::findNextText()
{
    if (!searchText.isEmpty())
    {
           this->findFirst(searchText, false, false, false, false, true);
    }
}

 void ChTextEdit::findPreviousText()
 {
     //int length = text.length();
     if (!searchText.isEmpty())
            this->findFirst(searchText, true, false, false, true, false);
 }

 int ChTextEdit::lineNumberAreaWidth()
 {
     int digits = 1;
     int max = qMax(1, this->lines());
     while (max >= 10)
     {
         max /= 10;
         ++digits;
     }

     int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

     return space;
 }

 void ChTextEdit::setNumberAreaWidth()
 {
     int i = this->lineNumberAreaWidth();
     this->setMarginWidth (1, i);
 }

 void ChTextEdit::refresh()
 {
     this->setNumberAreaWidth();
 }

 void ChTextEdit::gotoline()
 {
     bool ok;
     QString line = QInputDialog::getText(this, tr("HEXplorer :: gotoline"),
                                          tr("enter the line number:"), QLineEdit::Normal,
                                          "line number", &ok);
     int iLine = line.toInt();
     if (ok && !line.isEmpty())
     {
         this->setCursorPosition(iLine - 1, 0);
         this->setSelection(iLine - 1 , 0, iLine - 1, this->lineLength(iLine -1) - 1);
     }
 }

 bool ChTextEdit::saveFile(const QString &fileName)
 {
     QFile file(fileName);
     if (!file.open(QFile::WriteOnly)) {
         QMessageBox::warning(this, tr("Application"),
                              tr("Cannot write file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
         return false;
     }

     QTextStream out(&file);
     QApplication::setOverrideCursor(Qt::WaitCursor);
     out << this->text();
     QApplication::restoreOverrideCursor();

     setCurrentFile(fileName);
     return true;
 }

 bool ChTextEdit::save()
 {
     if (curFile.isEmpty())
     {
         return saveAs();
     } else
     {
         return saveFile(curFile);
     }
 }

 bool ChTextEdit::saveAs()
 {
     QString fileName = QFileDialog::getSaveFileName(this);
     if (fileName.isEmpty())
         return false;

     return saveFile(fileName);
 }

 void ChTextEdit::setCurrentFile(const QString &fileName)
 {
     curFile = fileName;
     this->setModified(false);
     setWindowModified(false);

     QString shownName;
     if (curFile.isEmpty())
         shownName = "untitled.txt";
     else
         shownName = strippedName(curFile);

     setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Application")));
 }

 QString ChTextEdit::strippedName(const QString &fullFileName)
 {
     return QFileInfo(fullFileName).fileName();
 }

 void ChTextEdit::loadFile(const QString &fileName)
 {
     QFile file(fileName);
     if (!file.open(QFile::ReadOnly)) {
         QMessageBox::warning(this, tr("Application"),
                              tr("Cannot read file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
         return;
     }

     QTextStream in(&file);
     QApplication::setOverrideCursor(Qt::WaitCursor);
     QString str = in.readAll();
     this->setText(str);
     QApplication::restoreOverrideCursor();
     file.close();

     setCurrentFile(fileName);
 }
