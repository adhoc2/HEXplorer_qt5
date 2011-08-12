#ifndef THREADPARSE_H
#define THREADPARSE_H

#include <QThread>
#include <Nodes/a2lfile.h>

class ThreadParse : public QThread
{
public:
   void run();
   QString str;
   A2LFILE *nodeA2l1;
};

#endif // THREADPARSE_H
