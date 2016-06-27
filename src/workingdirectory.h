#ifndef WORKINGDIRECTORY_H
#define WORKINGDIRECTORY_H

#include "node.h"
#include "a2ltreemodel.h"
#include "mdimain.h"
#include <QDir>
#include <QFileSystemWatcher>

class WorkingDirectory : public Node
{
    public:
        WorkingDirectory(QString rootPath, A2lTreeModel *model, MDImain *parent);
        ~WorkingDirectory();
        QString getFullPath();
        void deleteFileModel();
        void parseDir(QString dirPath);

private:
        QFileSystemWatcher fileWatcher;
        A2lTreeModel *model;
        MDImain *mdimain;
        QString rootPath;
        QStringList listWorkProjects;

        void rootPathChanged(QString newpath, Node *node);
        void fileRenamed(QString path, QString oldname, QString newname, Node *node);
        void directoryChanged(QString dirPath);
        void fileChanged(QString filePath);
};

#endif // WORKINGDIRECTORY_H
