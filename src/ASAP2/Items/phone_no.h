#ifndef PHONE_NO_H
#define PHONE_NO_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class PHONE_NO : public Item
{
    public:
        PHONE_NO( Node *parentNode);
        ~PHONE_NO();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, PHONE_NO> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // PHONE_NO_H
