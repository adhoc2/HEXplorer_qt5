#ifndef DEPOSIT_H
#define DEPOSIT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class DEPOSIT : public Item
{
    public:
        DEPOSIT( Node *parentNode);
        ~DEPOSIT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, DEPOSIT> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // DEPOSIT_H
