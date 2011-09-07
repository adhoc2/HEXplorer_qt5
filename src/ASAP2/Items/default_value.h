#ifndef DEFAULT_VALUE_H
#define DEFAULT_VALUE_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class DEFAULT_VALUE : public Item
{
    public:
        DEFAULT_VALUE(Node *parentNode);
        ~DEFAULT_VALUE();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,DEFAULT_VALUE> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // DEFAULT_VALUE_H
