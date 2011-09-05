#ifndef ALIGNMENT_FLOAT32_IEEE_H
#define ALIGNMENT_FLOAT32_IEEE_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ALIGNMENT_FLOAT32_IEEE : public Item
{
    public:
        ALIGNMENT_FLOAT32_IEEE(QTextStream  &in, Node *parentNode);
        ~ALIGNMENT_FLOAT32_IEEE();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ALIGNMENT_FLOAT32_IEEE> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream  &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ALIGNMENT_FLOAT32_IEEE_H
