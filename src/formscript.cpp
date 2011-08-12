#include "formscript.h"
#include "QTextStream"
#include "QSettings"
#include "QFileDialog"
#include "QFile"
#include "QMessageBox"
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qsciapis.h>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValueIterator>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include "data.h"
#include "hexfile.h"
#include <qdebug.h>


// ______ REGISTER CLASSES and TYPES _______ //

    //  class Hex* into QtScript :
Q_DECLARE_METATYPE(HexFile*)

QScriptValue hexToScriptValue(QScriptEngine *engine, HexFile* const &in)
{
    return engine->newQObject(in);
}

void hexFromScriptValue(const QScriptValue &object, HexFile* &out)
{
    out = qobject_cast<HexFile*>(object.toQObject());
}

    //  class Data into QtScript :
Q_DECLARE_METATYPE(Data*)

QScriptValue dataToScriptValue(QScriptEngine *engine, Data* const &in)
{
    return engine->newQObject(in);
}

void dataFromScriptValue(const QScriptValue &object, Data* &out)
{
    out = qobject_cast<Data*>(object.toQObject());
}

    //  type QList<double> into QtScript :
Q_DECLARE_METATYPE(QList<double>)
Q_DECLARE_METATYPE(QList<QList<double> >)

// ___________ my FUNCTIONS__________________ //

QScriptValue myPrintFunction(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i)
    {
        if (i > 0)  result.append(" ");
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    QTextEdit *edit = qobject_cast<QTextEdit*>(calleeData.toQObject());

    QString text = edit->toPlainText().append(">> " + result + "\n");
    edit->setText(text);
    edit->moveCursor(QTextCursor::End);


    return engine->undefinedValue();
}

QScriptValue includeFunction(QScriptContext *context, QScriptEngine *engine)
{

    for (int i = 0; i < context->argumentCount(); ++i)
    {

        //search a script with the given name into the QtScript path
        QString fileName = context->argument(i).toString();

        //update currentJsPath
        QSettings settings;
        QString path = settings.value("currentScriptPath").toString();

        QString filePath = path + "/" + fileName;
        QFileInfo info(filePath);

        //if found script execute
        if (info.isFile() && info.isReadable())
        {
            // read the file
            QFile file(filePath);
            if (!file.open(QFile::ReadOnly))
            {
                return context->throwError("could not read file : " + context->argument(i).toString());
            }
            QTextStream in(&file);
            QString contents = in.readAll();
            file.close();

            context->setActivationObject(context->parentContext()->activationObject());
            context->setThisObject(context->parentContext()->thisObject());
            engine->evaluate(contents, fileName);
        }
        else if (!info.isFile())
        {
            return context->throwError("could not find file : " + context->argument(i).toString());
        }
        else if (!info.isReadable())
        {
            return context->throwError(context->argument(i).toString() + " is not readable");
        }
    }
    return engine->undefinedValue();
}

QList<QwtPlot*> listPlot;
QScriptValue plotFunction(QScriptContext *context, QScriptEngine *engine)
{
    //check number of arguments
    if (context->argumentCount() < 2)
    {
         return context->throwError(QScriptContext::SyntaxError,"plot() takes at least 2 arguments");
    }

    //check arguments
    if (context->argument(0).isArray() && context->argument(1).isArray())
    {
        QScriptValue arrayX = context->argument(0);
        int lengthX = arrayX.property("length").toInteger();

        QScriptValue arrayY = context->argument(1);
        int lengthY = arrayY.property("length").toInteger();

        if (lengthX != lengthY)
        {
            return context->throwError(QScriptContext::RangeError, "plot() : X and Y axis must have the same length");
        }
    }
    else if (!context->argument(0).isArray() || !context->argument(1).isArray())
    {
        return context->throwError("plot() takes exactly 2 arrays");
    }

    //create X axis
    QScriptValue arrayX = context->argument(0);
    int length = arrayX.property("length").toInteger();
    QVector<double> xData(length);
    for(int i = 0; i < length; i++)
    {
        if (arrayX.property(i).isNumber())
        {
            xData[i] = arrayX.property(i).toNumber();
        }
        else
        {
            return context->throwError(QScriptContext::TypeError, "plot() : X axis must be an array of numbers");
        }
    }

    //create Z axis
    QScriptValue arrayZ = context->argument(1);
    QVector<double> zData(length);
    for(int i = 0; i < length; i++)
    {
        if (arrayZ.property(i).isNumber())
        {
            zData[i] = arrayZ.property(i).toNumber();
        }
        else
        {
            return context->throwError("plot() : Z axis must be an array of numbers");
        }
    }

    //check possible other arguments ("grid on","title",...)
    bool gridOn = false;
    bool symbolOn = false;
    QwtSymbol::Style symbolStyle;
    bool colorOn = false;
    QColor color;
    QString title = "";
    if(context->argumentCount() > 2)
    {
        for (int  i = 2; i < context->argumentCount(); i++)
        {
            if (context->argument(i).isString())
            {
                QString property = context->argument(i).toString();

                if (property == "grid on")
                {
                    gridOn = true;
                }
                else if (property == "o")
                {
                    symbolOn = true;
                    symbolStyle = QwtSymbol::Ellipse;
                }
                else if (property == "+")
                {
                    symbolOn = true;
                    symbolStyle = QwtSymbol::Cross;
                }
                else if (property == "b")
                {
                    colorOn = true;
                    color = Qt::blue;
                }
                else if(property == "r")
                {
                    colorOn = true;
                    color = Qt::red;
                }
                else if(property == "y")
                {
                    colorOn = true;
                    color = Qt::yellow;
                }
                else
                {
                    return context->throwError(QScriptContext::SyntaxError,"plot() : unknown property.");
                }
            }
            else
            {
                return context->throwError(QScriptContext::SyntaxError,"plot() : properties must be declared as strings");
            }
        }
    }

    //create or get a plot
    QwtPlot *plot = NULL;
    if (listPlot.isEmpty())
    {
        //create a new plot
        plot = new QwtPlot();

        //add the plot to the list
        listPlot.append(plot);
    }
    else
    {
        plot = listPlot.last();
    }

    // add data to the plot
    QwtPlotCurve *curve = new QwtPlotCurve(0);
    curve->setSamples(xData, zData);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->attach(plot);

    // add grid if necessary
    if (gridOn)
    {
        QwtPlotGrid *grid = new QwtPlotGrid;
        grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
        grid->attach(plot);
    }

    // set symbol if necessary
    if (symbolOn)
    {
        QwtSymbol *sym = new QwtSymbol();
        sym->setStyle(symbolStyle);
        sym->setSize(QSize(8, 8));
        if (colorOn)
        {
            sym->setPen(QColor(color));
        }
        curve->setSymbol(sym);
    }

    // set curve color if necessary
    if (colorOn)
    {
        curve->setPen(QPen(color));
    }

    //display plot
    plot->resize(600,400);
    plot->setCanvasBackground(Qt::white);
    plot->show();

    return engine->undefinedValue();
}

QScriptValue linspaceFunction(QScriptContext *context, QScriptEngine *engine)
{
    //check number of arguments
    if (context->argumentCount() < 3)
    {
         return context->throwError(QScriptContext::SyntaxError,"linspace() takes exactly 3 arguments");
    }

    //check arguments
    if (!context->argument(0).isNumber() || !context->argument(1).isNumber()
        || !context->argument(2).isNumber())
    {
        return context->throwError("linspace() takes exactly 3 numbers as arguments.");
    }

    if (context->argument(2).toNumber() == 0)
    {
        return context->throwError("linspace() : num must be greater then 0");
    }

    double xi = context->argument(0).toNumber();
    double xf = context->argument(1).toNumber();
    double num = context->argument(2).toNumber();
    double step = (xf - xi) / (num - 1);

    //create a new script value into the engine
    QList<double> list;
    for (int i = 0; i < num; i++)
    {
        list.append(xi + i * step);
    }

    return engine->toScriptValue(list);
}

QScriptValue closeAllFunction(QScriptContext *context, QScriptEngine *engine)
{
    foreach (QwtPlot *plot, listPlot)
    {
        if (plot)
        {
            plot->close();
        }
    }
    listPlot.clear();

    return context->throwValue("all figures are closed.");
}

QScriptValue figureFunction(QScriptContext *context, QScriptEngine *engine)
{
    //check number of arguments
    if (context->argumentCount() > 1)
    {
         return context->throwError(QScriptContext::SyntaxError,"figure() takes max only one argument");
    }

    //create a new plot
    QString str;
    if (context->argumentCount() == 0)
    {
        str = "figure (" + QString::number(listPlot.count()) + " )";
    }
    else
    {
        if (!context->argument(0).isString())
        {
            return context->throwError(QScriptContext::SyntaxError,"figure() takes a string as argument");
        }
        str = context->argument(0).toString();
    }
    QwtPlot *plot = new QwtPlot();
    plot->setWindowTitle(str);

    //add the figure to the listPlot
    listPlot.append(plot);

    //display figure
    plot->resize(600,400);
    plot->setCanvasBackground(Qt::white);
    plot->show();

    return engine->undefinedValue();
}

// ___________________________________________ //

FormScript::FormScript(WorkProject *wp, QWidget *parent) :  QMainWindow(parent)
{
    setupUi(this);
    createToolbar();

    //initialize members
    workProject = wp;

    // configure textEdit
    textEdit = new ChTextEdit();
    QsciLexerJavaScript * newLexer = new QsciLexerJavaScript();
    textEdit->setLexer(newLexer);
    setCentralWidget(textEdit);
    textEdit->setAutoIndent(true);
    textEdit->setAutoCompletionFillupsEnabled(true);
    textEdit->setAutoCompletionSource(QsciScintilla::AcsAll);
    textEdit->setPaper(QColor("#fbffcb"));
    textEdit->setIndentationGuidesBackgroundColor(QColor("#e6e6de"));
    textEdit->setAutoCompletionThreshold(2);
    textEdit->setFolding(QsciScintilla::BoxedTreeFoldStyle);
    textEdit->setMarginLineNumbers(1,true);

    // configure lineEdit SIGNAL Key_enter pressed
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(evaluateString()));

    // ---------  MY FUNCTIONS --------- //

    // register myPrint function
    QScriptValue myPrintCtor = engine.newFunction(myPrintFunction);
    myPrintCtor.setData(engine.newQObject(console));
    engine.globalObject().setProperty("print", myPrintCtor);

    // register includeFunction function
    QScriptValue incFunCtor = engine.newFunction(includeFunction);
    engine.globalObject().setProperty("include", incFunCtor);

    // register plotFunction function
    QScriptValue plotFunCtor = engine.newFunction(plotFunction);
    engine.globalObject().setProperty("plot", plotFunCtor);

    // register linspaceFunction function
    QScriptValue linspaceFunCtor = engine.newFunction(linspaceFunction);
    engine.globalObject().setProperty("linspace", linspaceFunCtor);

    // register closeAllFunction function
    QScriptValue closeAllFunCtor = engine.newFunction(closeAllFunction);
    engine.globalObject().setProperty("closeAll", closeAllFunCtor);

    // register figureFunction function
    QScriptValue figureFunCtor = engine.newFunction(figureFunction);
    engine.globalObject().setProperty("figure", figureFunCtor);

    // ---------  MY DATA TYPES --------- //

    // register the Hex* type in the engine
    qScriptRegisterMetaType(&engine, hexToScriptValue, hexFromScriptValue);

    // register the Data* type in the engine
    qScriptRegisterMetaType(&engine, dataToScriptValue, dataFromScriptValue);

    qScriptRegisterSequenceMetaType<QList<double> >(&engine);
    qScriptRegisterSequenceMetaType<QList<QList<double> > >(&engine);

    // --------------------------------- //

    //display global variables
    initializeGlobal();
}

void FormScript::closeEvent(QCloseEvent *event)
{
    // check if doc has been changed
    maybeSave();

    // accept event;
    event->accept();
}

void FormScript::createToolbar()
{
    QAction *openFile = new QAction(this);
    openFile->setIcon(QIcon(":/icones/milky_readfloppy.png"));
    connect(openFile, SIGNAL(triggered()), this, SLOT(openFile()));
    mainToolBar->addAction(openFile);

    QAction* run = new QAction(this);
    run->setIcon(QIcon(":/icones/milky_stereoPlay.png"));
    run->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(run, SIGNAL(triggered()), this, SLOT(runScript()));
    mainToolBar->addAction(run);

    QAction *debug = new QAction(this);
    debug->setIcon(QIcon(":/icones/Bug.png"));
    connect(debug, SIGNAL(triggered()), this, SLOT(debugScript()));
    mainToolBar->addAction(debug);
}

void FormScript::openFile()
{
    QSettings settings;
    QString path = settings.value("currentScriptPath").toString();

    QString fileName =  QFileDialog::getOpenFileName(this,
                                      tr("select an Asap file (A2L)"), path,
                                      tr("Yoda files (*.js);;all files (*.*)"));

    if (fileName.isEmpty())
    {
       return;
    }
    else
    {
        //display the text
        textEdit->loadFile(fileName);

    }

    //update currentA2lPath
    QString currentPath = QFileInfo(fileName).absolutePath();
    settings.setValue("currentScriptPath", currentPath);
}

bool FormScript::maybeSave()
{
  if (textEdit->isModified())
   {
    int ret = QMessageBox::warning(this, tr("Script Editor"),
                 "save your changes ?",
                 QMessageBox::Yes | QMessageBox::Default,
                 QMessageBox::No,
                 QMessageBox::Cancel | QMessageBox::Escape);

    if (ret == QMessageBox::Yes)
    {
      return textEdit->save();
    }
    else if (ret == QMessageBox::Cancel)
    {
      return false;
    }
  }

  return true;
}

void FormScript::runScript()
{
    //clear listPlot
    listPlot.clear();

    // add workProject to the script engine
    engine.globalObject().setProperty("wp", engine.newQObject(workProject, engine.QtOwnership));

    // evaluate the script
    evaluateScript(textEdit->text());
}

void FormScript::debugScript()
{
    // create a new WorkProject and add it to the script engine
    //WorkProject *wp = new WorkProject();
    //QScriptValue wpValue = engine.newQObject(wp);
    //engine.globalObject().setProperty("wp", wpValue);

    // add a debugger to the engine
    QScriptEngineDebugger debugger;
    debugger.attachTo(&engine);
    debugger.action(QScriptEngineDebugger::InterruptAction)->trigger();

    // evaluate the script
    evaluateScript(textEdit->text());

}

QScriptValue FormScript::evaluateScript(QString str)
{
    QScriptValue val = engine.evaluate(str);
    if (val.isError())
    {
        QString errorMessage = val.toString() + " at line "
                               + QString::number(engine.uncaughtExceptionLineNumber());
        printOutput( errorMessage );
    }

    updateGlobal();

    return val;
}

void FormScript::evaluateString()
{
    QScriptValue val = evaluateScript(lineEdit->text());

    if (!val.isError())
    {
        printOutput(val.toString());
        lineEdit->clear();
    }
}

void FormScript::printOutput(QString str)
{
    QString text = console->toPlainText().append(">> " + str + "\n");
    console->setText(text);
    console->moveCursor(QTextCursor::End);
}

void FormScript::initializeGlobal()
{
    QScriptValue global = engine.globalObject();

    QScriptValueIterator it(global);
    while (it.hasNext())
    {
        it.next();
        if (!globalVar.contains(it.name()))
        {
            globalVar.append(it.name());
        }
    }
}

void FormScript::updateGlobal()
{
    listWidget->clear();
    QScriptValue obj = engine.globalObject();

    while (obj.isObject())
    {
        QScriptValueIterator it(obj);
        while (it.hasNext())
        {
            it.next();
            if (!globalVar.contains(it.name()))
            {
                if (!it.value().isNull())
                {
                    if (it.value().isFunction())
                    {
                    }
                    else if (it.value().isArray())
                    {
                        listWidget->addItem(it.name() + " = [" + it.value().toString() + "]");
                    }
                    else if (it.value().isNumber())
                    {
                        listWidget->addItem(it.name() + " = " + it.value().toString());
                    }
                    else if (it.value().isString())
                    {
                        listWidget->addItem(it.name() + " = \"" + it.value().toString() + "\"");
                    }
                    else
                    {
                         listWidget->addItem(it.name() + " = " + it.value().toString());
                    }
                }
            }
        }
        obj = obj.prototype();
    }
}
