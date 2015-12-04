#ifndef WORKINGDIRECTORY_H
#define WORKINGDIRECTORY_H

#include "node.h"

class WorkingDirectory : public Node
{
    public:
        WorkingDirectory(char* nodeName);
        ~WorkingDirectory();
};

#endif // WORKINGDIRECTORY_H
