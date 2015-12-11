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

private:
        QFileSystemModel *fmodel;
        A2lTreeModel *model;
        MDImain *mdimain;
        QString rootPath;

        void populateNodeTreeview(QString str, Node *node);
};

#endif // WORKINGDIRECTORY_H
