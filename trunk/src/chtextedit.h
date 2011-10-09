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

#ifndef CHTEXTEDIT_H
#define CHTEXTEDIT_H


#include <Qsci/qsciscintilla.h>

class ChTextEdit : public QsciScintilla
{
    Q_OBJECT

    public:
        ChTextEdit(QWidget *parent = 0);
        void loadFile(const QString &fileName);

    private:
        QString curFile;

        QAction *saveAct;
        QAction *saveAsAct;
        QAction *find;
        QAction *findPrevious;
        QAction *findNext;
        QString searchText;
        QAction *goToLine;

        int lineNumberAreaWidth();
        void setNumberAreaWidth();
        bool saveFile(const QString &fileName);
        void setCurrentFile(const QString &fileName);
        QString strippedName(const QString &fullFileName);

    private slots:
        bool saveAs();
        void openFindDialog();
        void findNextText();
        void findPreviousText();
        void refresh();
        void gotoline();
        void contextMenuEvent( QPoint p );

    public slots:
        bool save();
};

#endif // CHTEXTEDIT_H
