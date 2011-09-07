#ifndef SRC_ADDR_Y_H
#define SRC_ADDR_Y_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class SRC_ADDR_Y : public Item
{
    public:
        SRC_ADDR_Y( Node *parentNode);
        ~SRC_ADDR_Y();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,SRC_ADDR_Y> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // SRC_ADDR_Y_H
