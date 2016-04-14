#ifndef WORKINGDIRECTORY_H
#define WORKINGDIRECTORY_H

#include "node.h"
#include "qfilesystemmodel.h"
#include "a2ltreemodel.h"
#include "mdimain.h"

class WorkingDirectory : public Node
{
    public:
        WorkingDirectory(QString rootPath, A2lTreeModel *model, MDImain *parent);
        ~WorkingDirectory();
        QString getFullPath();
        void deleteFileModel();

private:
        QFileSystemModel *fmodel;
        A2lTreeModel *model;
        MDImain *mdimain;
        QString rootPath;
        QStringList listWorkProjects;

        void populateNodeTreeview(QString path, Node *node);
        void rootPathChanged(QString newpath, Node *node);
        void fileRenamed(QString path, QString oldname, QString newname, Node *node);
};

#endif // WORKINGDIRECTORY_H
