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

#ifndef FORMSCRIPT_H
#define FORMSCRIPT_H

#include "ui_formscript.h"
#include "chtextedit.h"
#include "workproject.h"
#include <QCloseEvent>
#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <QTextEdit>

class FormScript : public QMainWindow, private Ui::FormScript
{
    Q_OBJECT

public:
    explicit FormScript(WorkProject *wp,QWidget *parent = 0);


protected:
     void closeEvent(QCloseEvent * event);

private:
    void initializeGlobal();
    void updateGlobal();
    QScriptValue evaluateScript(QString str);
    void printOutput(QString str);
    void createToolbar();
    bool maybeSave();

    QStringList globalVar;
    QScriptEngine engine;
    ChTextEdit* textEdit;

private slots:
    void openFile();
    void runScript();
    void debugScript();
    void evaluateString();

private:
    WorkProject *workProject;
};

#endif // FORMSCRIPT_H
