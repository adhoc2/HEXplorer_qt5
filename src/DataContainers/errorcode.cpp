#include "errorcode.h"

ErrorCode::ErrorCode(QString name)
{
    //initialsize the properties
    this->dec = nullptr;
    this->dtc = "";
    this->inc = nullptr;
    this->debounce = nullptr;
    this->setTime = nullptr;
    this->resetTime = nullptr;
    this->eventType = nullptr;
    this->name = name;
    this->prethd = nullptr;
    this->agicycidn = nullptr;
    this->agicycthd = nullptr;
    this->exclsncdn = nullptr;
    this->opercycidn = nullptr;
    this->opercycthd = nullptr;
    this->prio = nullptr;
    this->fltreactnid_0 = nullptr;
    this->fltreactnid_1 = nullptr;
    this->fltreactnid_2 = nullptr;
    this->fltreactnid_3 = nullptr;
    this->fltreactnid_4 = nullptr;
    this->fltreactnid_5 = nullptr;
    this->fltreactnid_6 = nullptr;
    this->delayid_0 = nullptr;
    this->delayid_1 = nullptr;
    this->delayid_2 = nullptr;
    this->delayid_3 = nullptr;
    this->delayid_4 = nullptr;
    this->delayid_5 = nullptr;
    this->delayid_6 = nullptr;

}
