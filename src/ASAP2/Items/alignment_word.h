#ifndef ALIGNMENT_WORD_H
#define ALIGNMENT_WORD_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ALIGNMENT_WORD : public Item
{
    public:
        ALIGNMENT_WORD(QTextStream &in, Node *parentNode);
        ~ALIGNMENT_WORD();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,ALIGNMENT_WORD> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ALIGNMENT_WORD_H
