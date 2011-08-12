#ifndef FIX_AXIS_PAR_H
#define FIX_AXIS_PAR_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class FIX_AXIS_PAR : public Item
{
    public:
        FIX_AXIS_PAR(QTextStream &in, Node *parentNode);
        ~FIX_AXIS_PAR();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,FIX_AXIS_PAR> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // FIX_AXIS_PAR_H
