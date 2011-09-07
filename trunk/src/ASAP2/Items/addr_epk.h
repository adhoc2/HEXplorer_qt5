#ifndef ADDR_EPK_H
#define ADDR_EPK_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ADDR_EPK : public Item
{
    public:
        ADDR_EPK(Node *parentNode);
        ~ADDR_EPK();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, ADDR_EPK> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

};

#endif // ADDR_EPK_H
