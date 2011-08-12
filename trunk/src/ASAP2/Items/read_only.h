#ifndef READ_ONLY_H
#define READ_ONLY_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class READ_ONLY : public Item
{
    public:
        READ_ONLY(QTextStream &in, Node *parentNode);
        ~READ_ONLY();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, READ_ONLY> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // READ_ONLY_H
