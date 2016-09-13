#include "workingdirectory.h"
#include "workproject.h"
#include "qdebug.h"
#include <QStringList>


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

void WorkingDirectory::parseDir(QString dirPath, WorkProject *wp)
{
    //set a QDir
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot |QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList list = dir.entryInfoList();


    //create a pointer to  WP
    //WorkProject *wp = nullptr;

    //create a list to store .hex and .s19
    QFileInfoList listDatasets;
    QFileInfoList listDirs;

    Node* subDir = nullptr;

    bool dirHasA2l = 0;
    if (wp)
    {
        dirHasA2l = 1;

        //create a node subFolder
        char* name = new char[dir.dirName().length() + 1];
        strcpy(name, dir.dirName().toLocal8Bit().data());
        subDir = new Node(name);
        subDir->setParentNode(wp);
        wp->addChildNode(subDir);
        wp->sortChildrensName();

        model->dataInserted(wp, wp->childNodes.indexOf(subDir));
    }

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
                   //check if a wp has already the same name (fileName) to prevent unique path in treeview model
                   QString displayNameOrg = file.fileName();
                   QString displayName = file.fileName();
                   int i = 0;
                   while (getNode(displayName))
                   {
                       displayName = displayNameOrg + " (" + QString::number(i) + ")";
                       i++;
                   }


                   // create a new Wp
                   wp = new WorkProject(file.absoluteFilePath(), model, mdimain, displayName);
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

               fileWatcher.addPath(file.absoluteFilePath());
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
           listDirs.append(file);
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

                   if (subDir)
                   {
                       wp->addHex(hex, subDir);
                   }
                   else
                       wp->addHex(hex, wp);

                   fileWatcher.addPath(file.absoluteFilePath());
               }
               else if (file.suffix().toLower() == "s19")
               {
                   SrecFile* srec = new SrecFile(file.absoluteFilePath(), wp);
                   if (subDir)
                   {
                       wp->addSrec(srec, subDir);
                   }
                   else
                       wp->addSrec(srec, wp);

                   fileWatcher.addPath(file.absoluteFilePath());
               }
           }
        }
    }

    //parse the under directories at the end with the wp if exists
    foreach (QFileInfo file, listDirs)
    {
       if (file.isDir())
       {
           parseDir(file.absoluteFilePath(), wp);
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
