#include "errorcode.h"

ErrorCode::ErrorCode(QString name)
{
    //initialsize the properties
    this->dec = "empty";
    this->dtc = "empty";
    this->inc = "empty";
    this->name = name;
    this->prethd = "empty";
    this->agicycidn = "empty";
    this->agicycthd = "empty";
    this->exclsncdn = "empty";
    this->opercycidn = "empty";
    this->opercycthd = "empty";
    this->prio = "empty";
    this->fltreactnid_0 = "empty";
    this->fltreactnid_1 = "empty";
    this->fltreactnid_2 = "empty";
    this->fltreactnid_3 = "empty";
    this->fltreactnid_4 = "empty";

}
