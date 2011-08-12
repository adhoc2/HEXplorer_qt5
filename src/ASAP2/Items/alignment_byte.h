#ifndef ALIGNMENT_BYTE_H
#define ALIGNMENT_BYTE_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ALIGNMENT_BYTE : public Item
{
    public:
        ALIGNMENT_BYTE(QTextStream &in, Node *parentNode);
        ~ALIGNMENT_BYTE();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ALIGNMENT_BYTE> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ALIGNMENT_BYTE_H
