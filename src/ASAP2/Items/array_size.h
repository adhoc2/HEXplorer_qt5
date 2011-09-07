#ifndef ARRAY_SIZE_H
#define ARRAY_SIZE_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class ARRAY_SIZE : public Item
{
    public:
        ARRAY_SIZE(Node *parentNode);
        ~ARRAY_SIZE();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ARRAY_SIZE> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ARRAY_SIZE_H
