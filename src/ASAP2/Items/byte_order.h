#ifndef Byte_Order_H
#define Byte_Order_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class Byte_Order : public Item
{
    public:
        Byte_Order(Node *parentNode);
        ~Byte_Order();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,Byte_Order> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // Byte_Order_H
