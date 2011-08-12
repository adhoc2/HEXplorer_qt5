#ifndef COMPU_TAB_REF_H
#define COMPU_TAB_REF_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class COMPU_TAB_REF : public Item
{
    public:
        COMPU_TAB_REF(QTextStream &in, Node *parentNode);
        ~COMPU_TAB_REF();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, COMPU_TAB_REF> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // COMPU_TAB_REF_H
