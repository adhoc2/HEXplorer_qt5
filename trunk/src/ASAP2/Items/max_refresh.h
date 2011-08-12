#ifndef MAX_REFRESH_H
#define MAX_REFRESH_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class MAX_REFRESH : public Item
{
    public:
        MAX_REFRESH(QTextStream &in, Node *parentNode);
        ~MAX_REFRESH();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,MAX_REFRESH> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // MAX_REFRESH_H
