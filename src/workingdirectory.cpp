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
    QObject::connect(fmodel, &QFileSystemModel::directoryLoaded, [=](const QString &newValue) {
     this->populateNodeTreeview(newValue, this);
     } );


}

WorkingDirectory::~WorkingDirectory()
{
    delete name;
    delete fmodel;
}

void WorkingDirectory::populateNodeTreeview(QString str, Node *node)
{
    if (listWorkProjects.contains(str))
        return;


    //index of selected folder in model
    QModelIndex parentIndex = fmodel->index(str);
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
               //model->beginReset();
               node->addChildNode(wp->a2lFile);
               wp->a2lFile->setParentNode(node);
               node->sortChildrensName();
               model->dataInserted(node, node->childNodes.indexOf(wp->a2lFile));
               //model->endReset();

               hasA2l = true;

               listWorkProjects.append(str);
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
                   //model->beginReset();
                   wp->addHex(hex);
                   //model->endReset();
               }
               else if (file.suffix().toLower() == "s19")
               {
                   SrecFile* srec = new SrecFile(file.absoluteFilePath(), wp);
                   //model->beginReset();
                   wp->addSrec(srec);
                   //model->endReset();
               }
           }
        }
    }
}

QString WorkingDirectory::getFullPath()
{
    return rootPath;
}
