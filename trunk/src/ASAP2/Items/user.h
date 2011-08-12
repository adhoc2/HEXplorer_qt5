#ifndef USER_H
#define USER_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class USER : public Item
{
    public:
        USER(QTextStream &in, Node *parentNode);
        ~USER();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,USER> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // USER_H
