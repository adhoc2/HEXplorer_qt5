#ifndef NUMBER_H
#define NUMBER_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class NUMBER : public Item
{
    public:
        NUMBER(QTextStream &in, Node *parentNode );
        ~NUMBER();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, NUMBER> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // NUMBER_H
