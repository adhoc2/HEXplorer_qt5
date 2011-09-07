#ifndef COEFFS_H
#define COEFFS_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class COEFFS : public Item
{
    public:
        COEFFS( Node *parentNode);
        ~COEFFS();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, COEFFS> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // COEFFS_H
