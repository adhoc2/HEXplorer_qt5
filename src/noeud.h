#ifndef NOEUD_H
#define NOEUD_H

#include <QString>
#include <QList>

class Noeud
{
public:
    enum Type {characteristic, subset};

    Noeud(Type type, const QString &str = "");
    ~Noeud();

    Type type;
    QString str;
    Noeud *parent;
    QList<Noeud*> children;
};

#endif // NOEUD_H
