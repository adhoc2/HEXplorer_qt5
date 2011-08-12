#include "dataprototype.h"
#include "data.h"
#include <QtScript/QScriptEngine>

#include "qdebug.h"


Q_DECLARE_METATYPE(Data*)

DataPrototype::DataPrototype(QObject *parent) : QObject(parent)
{

}

QString DataPrototype::toString() const
{
    return QString("Data(name = %0)").arg(getName());
}

QString DataPrototype::getName() const
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
        return item->getName();
    return QString();
}

void DataPrototype::setName(QString str)
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
        item->setName(str);
}

QList<double> DataPrototype::getX()
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
    {
        return item->x();
    }
    return QList<double>();
}

double DataPrototype::getX(int i)
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
    {
        return item->x(i);
    }
    return 0;
}

QList<double> DataPrototype::getY()
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
    {
        return item->y();
    }
    return QList<double>();
}

double DataPrototype::getY(int i)
{
    Data *item = qscriptvalue_cast<Data*>(thisObject());
    if (item)
    {
        return item->y(i);
    }
    return 0;
}

QList<double> DataPrototype::getZ()
{
    QList<double> vec;
    return vec;
}

void DataPrototype::setX(QList<double> list)
{
    Data *data = qscriptvalue_cast<Data*>(thisObject());
    if (data)
    {
        if (list.count() == data->xCount() || data->xCount() == 0)
        {
            data->setX(list);
        }
        else
            context()->throwError("you must assign the same length array");
    }
}

void DataPrototype::setXitem(int index, double value)
{
    Data *data = qscriptvalue_cast<Data*>(thisObject());
    if (data)
    {
        if (index < data->xCount())
            data->setX(index, value);
        else
            context()->throwError("index exceeded");
    }

}

void DataPrototype::setY(QList<double> list)
{
    Data *data = qscriptvalue_cast<Data*>(thisObject());
    if (data)
    {
        if (list.count() == data->yCount() || data->yCount() == 0)
        {
            data->setY(list);
        }
        else
            context()->throwError("you must assign the same length array");
    }
}

void DataPrototype::setZ(QList<double> list)
{

}

