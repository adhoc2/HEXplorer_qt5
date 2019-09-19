#ifndef ERRORCODE_H
#define ERRORCODE_H

#include <QString>

class ErrorCode
{
public:
    ErrorCode(QString name);

    QString name;
    QString dtc;
    QString inc;
    QString dec;
    QString prethd;
    QString agicycidn;
    QString agicycthd;
    QString opercycidn;
    QString opercycthd;
    QString exclsncdn;
    QString prio;
    QString fltreactnid_0;
    QString fltreactnid_1;
    QString fltreactnid_2;
    QString fltreactnid_3;
    QString fltreactnid_4;
    QString fltreactnid_5;
    QString fltreactnid_6;
    QString delayid_0;
    QString delayid_1;
    QString delayid_2;
    QString delayid_3;
    QString delayid_4;
    QString delayid_5;
    QString delayid_6;

};

#endif // ERRORCODE_H
