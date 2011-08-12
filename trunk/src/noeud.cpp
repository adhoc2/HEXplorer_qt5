#include "noeud.h"

Noeud::Noeud(Type type, const QString &str)
{
    this->type = type;
    this->str = str;
    parent = 0;
}

Noeud::~Noeud()
{
    qDeleteAll(children);
}
