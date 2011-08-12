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
