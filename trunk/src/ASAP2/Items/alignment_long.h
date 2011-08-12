#ifndef ALIGNMENT_LONG_H
#define ALIGNMENT_LONG_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ALIGNMENT_LONG : public Item
{
    public:
        ALIGNMENT_LONG(QTextStream &in, Node *parentNode);
        ~ALIGNMENT_LONG();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ALIGNMENT_LONG> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ALIGNMENT_LONG_H
