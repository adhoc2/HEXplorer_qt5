#ifndef FORMULA_INV_H
#define FORMULA_INV_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class FORMULA_INV : public Item
{
    public:
        FORMULA_INV(QTextStream &in, Node *parentNode);
        ~FORMULA_INV();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,FORMULA_INV> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // FORMULA_INV_H
