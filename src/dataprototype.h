#ifndef DATAPROTOTYPE_H
#define DATAPROTOTYPE_H

#include <QObject>
#include <QScriptable>

class DataPrototype : public QObject, public QScriptable
{
    Q_OBJECT
    Q_PROPERTY (QString name WRITE setName READ getName)
    Q_PROPERTY (QList<double> x WRITE setX READ getX)
    Q_PROPERTY (QList<double> y WRITE setY READ getY)

public:
    DataPrototype(QObject *parent = 0);

public slots:
    QList<double> getX();
    QList<double> getY();
    double getX(int i);
    double getY(int i);
    QList<double> getZ();
    void setX(QList<double> list);
    void setXitem(int i, double value);
    void setY(QList<double> list);
    void setZ(QList<double> list);
    QString getName() const;
    void setName(QString str);
    QString toString() const;
};

#endif // DATAPROTOTYPE_H
