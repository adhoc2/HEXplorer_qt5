#ifndef REF_UNIT_H
#define REF_UNIT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class REF_UNIT : public Item
{
    public:
        REF_UNIT( Node *parentNode);
        ~REF_UNIT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, REF_UNIT> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // REF_UNIT_H
