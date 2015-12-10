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

private:
        QFileSystemModel *fmodel;
        A2lTreeModel *model;
        MDImain *mdimain;

        void populateNodeTreeview(QString str, Node *node);
};

#endif // WORKINGDIRECTORY_H
