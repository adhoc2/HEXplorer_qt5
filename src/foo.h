#ifndef FOO_H
#define FOO_H


#include <mgl2/qt.h>
#include "data.h"

class Foo : public mglDraw
{
public:
    Foo(Data *dat);
    ~Foo();

  int Draw(mglGraph *gr);

private:
  Data *data;
};


#endif // FOO_H
