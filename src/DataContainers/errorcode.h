#ifndef ERRORCODE_H
#define ERRORCODE_H

#include <QString>
#include "data.h"

class ErrorCode
{
public:
    ErrorCode(QString name);

    QString name;
    QString dtc;


};

#endif // ERRORCODE_H
