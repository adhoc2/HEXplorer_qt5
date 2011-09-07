#ifndef SRC_ADDR_X_H
#define SRC_ADDR_X_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class SRC_ADDR_X : public Item
{
    public:
        SRC_ADDR_X( Node *parentNode);
        ~SRC_ADDR_X();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,SRC_ADDR_X> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // SRC_ADDR_X_H
