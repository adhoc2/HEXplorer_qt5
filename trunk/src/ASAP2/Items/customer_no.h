#ifndef CUSTOMER_NO_H
#define CUSTOMER_NO_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class CUSTOMER_NO : public Item
{
    public:
        CUSTOMER_NO(QTextStream &in, Node *parentNode);
        ~CUSTOMER_NO();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, CUSTOMER_NO> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // CUSTOMER_NO_H
