#ifndef FNC_VALUES_H
#define FNC_VALUES_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class FNC_VALUES : public Item
{
    public:
        FNC_VALUES(Node *parentNode);
        ~FNC_VALUES();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,FNC_VALUES> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // FNC_VALUES_H
