#ifndef MATEXP_H
#define MATEXP_H

#include "matexp_global.h"
#include "mat.h"
#include "iostream"

class MATEXPSHARED_EXPORT Matexp {
public:
    Matexp();

    void createMat(std::string str);
    void closeMat();
    void addMatrix(double* data);

private:
    MATFile *pmat;
};

#endif // MATEXP_H
