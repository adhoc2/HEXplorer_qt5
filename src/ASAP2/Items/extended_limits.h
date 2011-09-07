#ifndef EXTENDED_LIMITS_H
#define EXTENDED_LIMITS_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class EXTENDED_LIMITS : public Item
{
    public:
        EXTENDED_LIMITS(Node *parentNode);
        ~EXTENDED_LIMITS();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,EXTENDED_LIMITS> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // EXTENDED_LIMITS_H
