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


    //fmodel
    fmodel = new QFileSystemModel();
    fmodel->setRootPath(QDir::cleanPath(rootPath));
    QStringList filters;
    filters << "*.a2l" << "*.hex" << "*.s19";
    fmodel->setNameFilters(filters);
    fmodel->setNameFilterDisables(false);

    //model
    mdimain = parent;
    this->model = model;

    //connect a slot to populate the treeView
    QObject::connect(fmodel, &QFileSystemModel::directoryLoaded, [=](const QString &path) {
     this->populateNodeTreeview(path, this);
     } );

    QObject::connect(fmodel, &QFileSystemModel::rootPathChanged, [=](const QString &newPath) {
        this->rootPathChanged(newPath, this);
        } );

    QObject::connect(fmodel, &QFileSystemModel::fileRenamed, [=](const QString &path, const QString &oldname,
                     const QString &newname) {
        this->fileRenamed(path, oldname, newname, this);
        } );

}

WorkingDirectory::~WorkingDirectory()
{
    delete name;
    delete fmodel;
}

void WorkingDirectory::populateNodeTreeview(QString path, Node *node)
{
    if (listWorkProjects.contains(path))
        return;


    //index of selected folder in model
    QModelIndex parentIndex = fmodel->index(path);
    int numRows = fmodel->rowCount(parentIndex);

    //create a pointer to  WP
    WorkProject *wp = nullptr;

    //foreach file under selected folder do something
    bool hasA2l = false;
    for (int row = 0; row < numRows; ++row)
    {
       QModelIndex index = fmodel->index(row, 0, parentIndex);

       QFileInfo file = fmodel->fileInfo(index);
       if (file.isFile())
       {
           if (file.suffix().toLower() == "a2l" && !hasA2l)
           {
               // create a new Wp
               wp = new WorkProject(file.absoluteFilePath(), this->model, mdimain);
               wp->init(); //init but do not parse the file
               wp->attach(mdimain);
               mdimain->insertWp(wp);

               //update the ui->treeView
               node->addChildNode(wp->a2lFile);
               wp->a2lFile->setParentNode(node);
               node->sortChildrensName();
               model->dataInserted(node, node->childNodes.indexOf(wp->a2lFile));

               hasA2l = true;

               listWorkProjects.append(path);
           }
       }
       else
       {
           fmodel->fetchMore(index);
       }
    }

    //foreach file under selected folder do something
    if (hasA2l)
    {
        for (int row = 0; row < numRows; ++row)
        {
           QModelIndex index = fmodel->index(row, 0, parentIndex);

           QFileInfo file = fmodel->fileInfo(index);
           if (file.isFile())
           {
               if (file.suffix().toLower() == "hex")
               {
                   HexFile* hex = new HexFile(file.absoluteFilePath(), wp);
                   wp->addHex(hex);
               }
               else if (file.suffix().toLower() == "s19")
               {
                   SrecFile* srec = new SrecFile(file.absoluteFilePath(), wp);
                   wp->addSrec(srec);
               }
           }
        }
    }
}

void WorkingDirectory::rootPathChanged(QString newpath, Node *node)
{

}

void WorkingDirectory::fileRenamed(QString path, QString oldname, QString newname, Node *node)
{

}

QString WorkingDirectory::getFullPath()
{
    return rootPath;
}
