#ifndef EPK_H
#define EPK_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class EPK : public Item
{
    public:
        EPK(Node *parentNode);
        ~EPK();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, EPK> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // EPK_H
