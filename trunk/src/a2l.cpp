#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtAlgorithms>
#include <QTime>
#include <QMap>
#include <QFileInfo>
#include <qtconcurrentrun.h>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "a2l.h"
#include "hexfile.h"
#include "lexer.h"
#include "node.h"
#include "item.h"
#include "threadparse.h"
#include "a2lgrammar.h"
#include <sstream>

#include <iostream>
#include <fstream>
#include <string>
using namespace std;


A2l::A2l(QString fullFileName, QObject *parent): QObject(parent)
{
    fullA2lName = fullFileName;
    a2lFile = 0;
    progressVal = 0;
    progBarMaxValue = 0;
    omp_init_lock(&lockValue);
    is_Ok = true;
}

A2l::~A2l()
{
   delete a2lFile;
   omp_destroy_lock(&lockValue);
}

string A2l::getFullA2lFileName()
{
    return fullA2lName.toLocal8Bit().data();
}

void A2l::setFullA2lFileName(string str)
{
    fullA2lName = QString(str.c_str());
}

void A2l::setFullA2lFileName(QString str)
{
    fullA2lName = str;
}

void A2l::parse()
{
    //start a timer
    QTime timer;
    timer.start();

    //clear the "console"
    outputList.clear();

    //parse in 1 or 2 threads based on processor architecture
    bool myDebug = 0;
#ifdef MY_DEBUG
    myDebug = 1;
#endif
    if (omp_get_num_procs() > 1 && !myDebug)
    {
        parseOpenMPA2l();
    }
    else
    {
        parseSTA2l();
    }

    //test if parsing could be achieved otherwise return;
    if (a2lFile == 0)
        return;
    else
    {
        QTime timer;
        timer.start();
        readSubset();
        qDebug() << "5- readSubset " << timer.elapsed();
    }

    //get and write the timer value
    double t = timer.elapsed();
    QString sec ;
    sec.setNum(t/1000);
    this->outputList.append("elapsed time to parse the A2L file = " + sec + " s");

}

void A2l::parseSTA2l()
{
//    QFile file(fullA2lName);
//    if (!file.open(QFile::ReadOnly))
//    {
//        this->outputList.append("Cannot read file " + this->fullA2lName);
//        return;
//    }

//    QTextStream in1(&file);
//    QString str;
//    str = in1.readAll();
//    file.close();

    //create a stream into the file (Merci Oscar...)
    FILE* fid = fopen(fullA2lName.toStdString().c_str(),"r");
    if (!fid)
    {
        this->outputList.append("Cannot read file " + this->fullA2lName);
        return;
    }
    fseek(fid, 0, SEEK_END);
    long size = ftell(fid);
    rewind(fid);
    char* buffer = (char*)malloc(size*sizeof(char));
    fread(buffer, sizeof(char), size, fid);
    fclose(fid);
    QString strr(buffer);
    QTextStream in(&strr, QIODevice::ReadOnly);

    // set the maximum for the progressbar
    progBarMaxValue = strr.length();

    //delete previous tree and create a new rootNode
    A2lLexer *lexer = new A2lLexer();
    connect(lexer, SIGNAL(returnedToken(int)), this, SLOT(checkProgressStream(int)),
            Qt::DirectConnection);
    lexer->initialize();
    QStringList *errorList = new QStringList();

    //create an ASAP2 file Node to start parsing
    A2LFILE *nodeA2l = new A2LFILE(in, 0, lexer, errorList, fullA2lName);
    nodeA2l->name = new char[(QFileInfo(fullA2lName).fileName()).toLocal8Bit().count() + 1];
    strcpy(nodeA2l->name, QFileInfo(fullA2lName).fileName().toLocal8Bit().data());
    a2lFile = nodeA2l;

    qDebug() << lexer->tamere;

    // show error
    if (errorList->isEmpty())
    {
        this->outputList.append("file parsed with success ...");
    }
    else
    {
        is_Ok = false;
        outputList.append(*errorList);
    }

}

bool A2l::parseOpenMPA2l()
{
    QTime time;
    time.start();

//    //open the selected file
//    QFile file(fullA2lName);
//    if (!file.open(QFile::ReadOnly))
//    {
//        this->outputList.append("Cannot read file " + this->fullA2lName);
//        return true;
//    }

//    //create a stream into the file
//    QTextStream in(&file);
//    QString str = in.readAll();
//    file.close();

    //create a stream into the file (Merci Oscar...)
    FILE* fid = fopen(fullA2lName.toStdString().c_str(),"r");
    if (!fid)
    {
        this->outputList.append("Cannot read file " + this->fullA2lName);
        return false;
    }
    fseek(fid, 0, SEEK_END);
    long size = ftell(fid);
    rewind(fid);
    char* buffer = (char*)malloc(size*sizeof(char));
    fread(buffer, sizeof(char), size, fid);
    fclose(fid);
    QString str(buffer);

    qDebug() << "\n1- read " << time.elapsed();
    time.restart();

    //trunk a2lfile into 2 parts for multi-threading
    QString str1;
    QString str2;
    if (!trunkA2l(str, str1, str2))
    {
        parseSTA2l();
        return true;
    }

    // set the maximum for the progressbar
    progBarMaxValue = str.length();

    qDebug() << "2- trunk " << time.elapsed();
    time.restart();

    //create nodeA2l
    A2LFILE *nodeA2l1 = 0;
    A2LFILE *nodeA2l2 = 0;

    //parse in parallel
    omp_set_dynamic(0);
    omp_set_num_threads(2);
    double t_ref1 = 0, t_final1 = 0;
    double t_ref2 = 0, t_final2 = 0;
    #pragma omp parallel
      {
          #pragma omp sections
            {
                //thread1
                #pragma omp section
                {
                    // start timer
                    t_ref1 = omp_get_wtime();

                    // create a new lexer
                    A2lLexer *lexer1 = new A2lLexer();
                    connect(lexer1, SIGNAL(returnedToken(int)), this, SLOT(checkProgressStream(int)),
                            Qt::DirectConnection);
                    lexer1->initialize();
                    QStringList *errorList1 = new QStringList();
                    QTextStream out1(&str1);

                    // start parsing the file
                    nodeA2l1 = new A2LFILE(out1, 0, lexer1, errorList1, fullA2lName);

                    // change the name
                    nodeA2l1->name = new char[(QFileInfo(fullA2lName).fileName()).toLocal8Bit().count() + 1];
                    strcpy(nodeA2l1->name, QFileInfo(fullA2lName).fileName().toLocal8Bit().data());

                    // stop timer
                    t_final1 = omp_get_wtime();
                }

                //thread2
                #pragma omp section
                {
                    // start timer
                    t_ref2 = omp_get_wtime();

                    // create a new lexer
                    A2lLexer *lexer2 = new A2lLexer();
                    connect(lexer2, SIGNAL(returnedToken(int)), this, SLOT(checkProgressStream(int)),
                            Qt::DirectConnection);
                    lexer2->initialize();
                    QStringList *errorList2 = new QStringList();
                    QTextStream out2(&str2);

                    // start parsing the file
                    nodeA2l2 = new A2LFILE(out2, 0, lexer2, errorList2);

                    // stop timer
                    t_final2 = omp_get_wtime();
                }
            }
      }

    //display parsing errors
    if (nodeA2l1->errorList->isEmpty() && nodeA2l2->errorList->isEmpty())
    {
        QString num1;
        num1.setNum(t_final1 - t_ref1);
        this->outputList.append("thread 1 parsed in " + num1 + " sec");
        QString num2;
        num2.setNum(t_final2 - t_ref2);
        outputList.append("thread 2 parsed in " + num2 + " sec");
    }
    else
    {
        is_Ok = false;
        outputList.append("ASAP file parser error ...");
        if (!nodeA2l1->errorList->isEmpty())
        {
            outputList.append("error in thread 1:");
            outputList.append(*nodeA2l1->errorList);
        }
        if (!nodeA2l2->errorList->isEmpty())
        {
            outputList.append("error in thread 2:");
            outputList.append(*nodeA2l2->errorList);
        }

        QString num1;
        num1.setNum(t_final1 - t_ref1);
        outputList.append("thread 1 executed in " + num1 + " sec");
        QString num2;
        num2.setNum(t_final2 - t_ref2);
        outputList.append("thread 2 executed in " + num2 + " sec");
    }

    qDebug() << "3- parse " << time.elapsed();
    time.restart();

    //merge the 2 a2lFile
    merge(nodeA2l2, nodeA2l1);
    a2lFile = nodeA2l1;

    qDebug() << "4- merge " << time.elapsed();


    //delete nodeA2l2; MEMORY LEAK !!!
    //cannot be deleted because of the lexer and grammar

    return true;
}

bool A2l::trunkA2l(QString &str, QString &str1, QString &str2)
{
    //devide the file into 2 QString
    int a = str.indexOf(QRegExp("/begin MODULE"));
    int b = str.lastIndexOf(QRegExp("/end MODULE"));
    int middle = str.size() / 2;

    if ((a + 13 < middle) && (middle < b))
    {
        //cut str into the middle
        if (str.size() % 2 == 0)
        {
            str1 = str.left(middle);
            str2 = str.right(middle);
        }
        else
        {
            str1 = str.left(middle + 1);
            str2 = str.right(middle);
        }

        //search for a MODULE child node
        QRegExp rxlen("/begin (\\w+)");
        QString nodeType = "";
        int num = str1.length();
        int lastBeginChar = 0;

        while (nodeType != "CHARACTERISTIC" &&
               nodeType != "MEASUREMENT" &&
               nodeType != "RECORD_LAYOUT" &&
               nodeType != "FUNCTION" &&
               nodeType != "AXIS_PTS" &&
               nodeType != "RECORD_LAYOUT")
        {
            lastBeginChar = str1.left(num).lastIndexOf(rxlen);
            nodeType = rxlen.cap(1);
            num = lastBeginChar;
        }

        //create 2 ASAP file for parallel parsing
        str2 = "ASAP2_VERSION  1 4 \n"
               "/begin PROJECT PRO \"EDC17CV41\"\n"
               "  /begin HEADER \"\"\n"
               "    VERSION    \"P_662_EDC17CV41_420\"\n"
               "    PROJECT_NO P_662\n"
               "  /end HEADER \n"
               "  /begin MODULE CHUNKstart DIM \"\"\n" + str1.right(str1.size()- lastBeginChar) + str2;


        str1 = str1.left(lastBeginChar) + "\nCHUNKend";

        return true;
    }
    else
        return false;
}

void A2l::merge(A2LFILE *src, A2LFILE *trg)
{
    //find Module i stopped in source file
    Node *pro = src->getProject();
    pro->sortChildrensName();
    Node *srcModule = pro->getNode("MODULE");
    if (srcModule == NULL)
    {
        this->outputList.append("ERROR : could not merge the 2 threads (source module NULL)");
		this->outputList.append("PROJECT name: " + QString(pro->name));
		outputList.append(*pro->errorList);
        return;
    }

    int i = 0;
    bool foundSrcStopped = false;
    while (!foundSrcStopped && i < srcModule->childNodes.count())
    {
        if (!srcModule->child(i)->stopped)
            i++;
        else if (srcModule->child(i)->stopped)
        {
            foundSrcStopped = true;
        }
    }

    if (foundSrcStopped)
    {
        //find Module stopped in target file;
        Node *trgModule= trg->getProject()->getNode("MODULE");

        if (trgModule == NULL)
        {
            this->outputList.append("ERROR : could not merge the 2 threads (target module NULL)");
            return;
        }

        int j = 0;
        bool foundTrgStopped = false;
        while (!foundTrgStopped && j < trgModule->childNodes.count())
        {
            if (!trgModule->child(j)->stopped)
                j++;
            else if (trgModule->child(j)->stopped)
            {
                foundTrgStopped = true;
            }
        }

        if (foundTrgStopped)
        {
            //merge the cut module nodes
            foreach(Node* node, srcModule->child(i)->childNodes)
            {
                if (trgModule->child(j)->isChild(node->name))
                {
                    Node *trgNode = trgModule->child(j)->child((QString)node->name, false);
                    foreach (Node* n, node->childNodes)
                    {
                        trgNode->addChildNode(n);
                        n->setParentNode(trgNode);
                        node->removeChildNode(n);
                    }
                    trgNode->sortChildrensName();
                }
                else
                {
                    trgModule->child(j)->addChildNode(node);
                    node->setParentNode(trgModule->child(j));
                    srcModule->child(i)->removeChildNode(node);
                }
            }

            //sort the nodes from target module
            trgModule->child(j)->sortChildrensName();

            //merge also the listChar from cut Module
            MODULE *srcMod = (MODULE*)srcModule->child(i);
            MODULE *trgMod = (MODULE*)trgModule->child(j);
            trgMod->listChar.append(srcMod->listChar);
            trgMod->listChar.sort();

            //add the other modules that were not splitted for parsing
            foreach(Node* node, srcModule->childNodes)
            {
                //complete Node
                if (!node->stopped)
                {
                    trgModule->addChildNode(node);
                    srcModule->removeChildNode(node);
                    node->setParentNode(trgModule);
                }
            }
        }
    }
}

QStringList A2l::_outputList()
{
    return this->outputList;
}

void A2l::readSubset()
{
    if (a2lFile)
    {
        PROJECT *project = a2lFile->getProject();
        if (project)
        {
            QList<MODULE*> listModule = a2lFile->getProject()->listModule();

            foreach (MODULE *module, listModule)
            {
                Node *fun = module->getNode("FUNCTION");
                Node *charac = module->getNode("CHARACTERISTIC");
                Node *axis_pts =  module->getNode("AXIS_PTS");

                if (fun)
                {
                    foreach (Node *subset, fun->childNodes)
                    {
                        DEF_CHARACTERISTIC *def = (DEF_CHARACTERISTIC*)subset->getNode("DEF_CHARACTERISTIC");
                        if (def)
                        {
                            QStringList listDefChar = def->getCharList();
                            foreach(QString str, listDefChar)
                            {
                                CHARACTERISTIC *node = (CHARACTERISTIC*)charac->getNode(str);
                                if (node)
                                {
                                    node->setSubset((FUNCTION*)subset);
                                }
                                else
                                {
                                    AXIS_PTS *node = (AXIS_PTS*)axis_pts->getNode(str);
                                    if (node)
                                    {
                                        node->setSubset((FUNCTION*)subset);
                                    }
                                }
                            }
                        }
                        else
                        {
                            REF_CHARACTERISTIC *ref = (REF_CHARACTERISTIC*)subset->getNode("REF_CHARACTERISTIC");
                            if (ref)
                            {
                                QStringList listDefChar = ref->getCharList();
                                foreach(QString str, listDefChar)
                                {
                                    CHARACTERISTIC *node = (CHARACTERISTIC*)charac->getNode(str);
                                    if (node)
                                    {
                                        node->setSubset((FUNCTION*)subset);
                                    }
                                    else
                                    {
                                        AXIS_PTS *node = (AXIS_PTS*)axis_pts->getNode(str);
                                        if (node)
                                        {
                                            node->setSubset((FUNCTION*)subset);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void A2l::checkProgressStream(int pos)
{
    omp_set_lock(&lockValue);
    progressVal += pos;

    double div = (double)progressVal/(double)progBarMaxValue;

    if (div > 0.98)
    {
        emit incProgressBar(progBarMaxValue, progBarMaxValue);
    }
    else
    {
         emit incProgressBar(progressVal, progBarMaxValue);
    }

    omp_unset_lock(&lockValue);
}

bool A2l::isOk()
{
    return is_Ok;
}
