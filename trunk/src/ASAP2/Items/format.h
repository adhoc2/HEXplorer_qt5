#ifndef FORMAT_H
#define FORMAT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class FORMAT : public Item
{
    public:
        FORMAT(Node *parentNode);
        ~FORMAT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,FORMAT> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // FORMAT_H
