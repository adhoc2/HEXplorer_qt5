#ifndef SYSTEM_CONSTANT_H
#define SYSTEM_CONSTANT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class SYSTEM_CONSTANT : public Item
{
    public:
        SYSTEM_CONSTANT(QTextStream &in, Node *parentNode);
        ~SYSTEM_CONSTANT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, SYSTEM_CONSTANT> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // SYSTEM_CONSTANT_H
