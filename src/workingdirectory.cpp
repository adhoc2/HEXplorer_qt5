#include "workingdirectory.h"
#include "workproject.h"
#include "qdebug.h"


WorkingDirectory::WorkingDirectory(QString rootPath, A2lTreeModel *model = NULL, MDImain *parent = 0 ) : Node()
{
    //set node Icon
    this->_pixmap = ":/icones/milky_classeur.png";

    //node name
    char* name = new char[(QFileInfo(rootPath).fileName()).toLocal8Bit().count() + 1];
    strcpy(name, (QFileInfo(rootPath).fileName()).toLocal8Bit().data());
    this->name = name;
    this->rootPath = rootPath;

    // set treeView model from mdiMain
    mdimain = parent;
    this->model = model;

    //set parent of this Node WorkingDirectory
    Node* rootNode = model->getRootNode();
    if (rootNode == NULL)
        model->createRootNode();
    this->setParentNode(model->getRootNode());

    //directory filters
    QStringList filters;
    filters << "*.a2l" << "*.hex" << "*.s19";

    //parse rootPath
    parseDir(rootPath);

    QObject::connect(&fileWatcher, &QFileSystemWatcher::directoryChanged, [=](const QString &path) {
     this->directoryChanged(path);
     } );

    QObject::connect(&fileWatcher, &QFileSystemWatcher::fileChanged, [=](const QString &path) {
     this->fileChanged(path);
     } );
}

WorkingDirectory::~WorkingDirectory()
{
    delete name;
}

void WorkingDirectory::parseDir(QString dirPath)
{
    //set a QDir
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot |QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();

    //create a pointer to  WP
    WorkProject *wp = nullptr;

    //create a list to store .hex and .s19
    QFileInfoList listDatasets;

    bool dirHasA2l = false;
    foreach (QFileInfo file, list)
    {
       if (file.isFile())
       {
           if (file.suffix().toLower() == "a2l" && !dirHasA2l)
           {
               //check if .a2l already exists
               wp = mdimain->getWp(file.absoluteFilePath());

               //if wp does not exist creates a new WorkProject
               if (!wp)
               {
                   // create a new Wp
                   wp = new WorkProject(file.absoluteFilePath(), model, mdimain);
                   wp->init(); //init but do not parse the file
                   wp->attach(mdimain);
                   mdimain->insertWp(wp);

                   //update the ui->treeView
                   this->addChildNode(wp);
                   wp->setParentNode(this);
                   this->sortChildrensName();
                   model->dataInserted(this, this->childNodes.indexOf(wp));

                   listWorkProjects.append(dirPath);
               }

               dirHasA2l = true;
           }
           else if (file.suffix().toLower() == "hex")
           {
               listDatasets.append(file);
           }
           else if (file.suffix().toLower() == "s19")
           {
               listDatasets.append(file);
           }
       }
       else if (file.isDir())
       {
           parseDir(file.absoluteFilePath());

       }
    }

    //foreach file .hex or .s19 under selected folder and if .a2l file already exists
    if (dirHasA2l)
    {
        foreach (QFileInfo file, listDatasets)
        {

           if (file.isFile())
           {
               if (file.suffix().toLower() == "hex")
               {
                   HexFile* hex = new HexFile(file.absoluteFilePath(), wp);
                   wp->addHex(hex);
                   fileWatcher.addPath(file.absoluteFilePath());
               }
               else if (file.suffix().toLower() == "s19")
               {
                   SrecFile* srec = new SrecFile(file.absoluteFilePath(), wp);
                   wp->addSrec(srec);
                   fileWatcher.addPath(file.absoluteFilePath());
               }
           }
        }
    }

    //set fileWatcher to control external changes to the root path
    fileWatcher.addPath(dirPath);

}

void WorkingDirectory::directoryChanged(QString dirPath)
{
    qDebug() << dirPath;
}

void WorkingDirectory::fileChanged(QString filePath)
{
    qDebug() << filePath;
}

QString WorkingDirectory::getFullPath()
{
    return rootPath;
}


