#ifndef ECU_H
#define ECU_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ECU : public Item
{
    public:
        ECU(Node *parentNode);
        ~ECU();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, ECU> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ECU_H
