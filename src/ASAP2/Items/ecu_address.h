#ifndef ECU_ADDRESS_H
#define ECU_ADDRESS_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ECU_ADDRESS : public Item
{
    public:
        ECU_ADDRESS(Node *parentNode);
        ~ECU_ADDRESS();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ECU_ADDRESS> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ECU_ADDRESS_H
