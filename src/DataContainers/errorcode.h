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
    Data* inc;
    Data* dec;
    Data* prethd;
    Data* debounce;
    Data* setTime;
    Data* resetTime;
    Data* eventType;
    Data* agicycidn;
    Data* agicycthd;
    Data* opercycidn;
    Data* opercycthd;
    Data* exclsncdn;
    Data* prio;
    Data* fltreactnid_0;
    Data* fltreactnid_1;
    Data* fltreactnid_2;
    Data* fltreactnid_3;
    Data* fltreactnid_4;
    Data* fltreactnid_5;
    Data* fltreactnid_6;
    Data* delayid_0;
    Data* delayid_1;
    Data* delayid_2;
    Data* delayid_3;
    Data* delayid_4;
    Data* delayid_5;
    Data* delayid_6;

};

#endif // ERRORCODE_H
