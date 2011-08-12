#ifndef COEFFS_LINEAR_H
#define COEFFS_LINEAR_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class COEFFS_LINEAR : public Item
{
    public:
        COEFFS_LINEAR(QTextStream &in, Node *parentNode);
        ~COEFFS_LINEAR();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, COEFFS_LINEAR> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // COEFFS_LINEAR_H
