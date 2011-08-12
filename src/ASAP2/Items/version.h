#ifndef VERSION_H
#define VERSION_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class VERSION : public Item
{
    public:
        VERSION(QTextStream &in, Node *parentNode);
        ~VERSION();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,VERSION> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // VERSION_H
