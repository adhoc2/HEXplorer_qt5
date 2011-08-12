#ifndef S_REC_LAYOUT_H
#define S_REC_LAYOUT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class S_REC_LAYOUT : public Item
{
    public:
        S_REC_LAYOUT(QTextStream &in, Node *parentNode);
        ~S_REC_LAYOUT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,S_REC_LAYOUT> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // S_REC_LAYOUT_H
