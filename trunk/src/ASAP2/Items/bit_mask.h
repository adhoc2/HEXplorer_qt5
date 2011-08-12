#ifndef BIT_MASK_H
#define BIT_MASK_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class BIT_MASK : public Item
{
    public:
        BIT_MASK(QTextStream &in, Node *parentNode);
        ~BIT_MASK();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, BIT_MASK> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // BIT_MASK_H
