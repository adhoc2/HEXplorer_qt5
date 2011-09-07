#ifndef ANNOTATION_ORIGIN_H
#define ANNOTATION_ORIGIN_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class ANNOTATION_ORIGIN : public Item
{
    public:
        ANNOTATION_ORIGIN(Node *parentNode);
        ~ANNOTATION_ORIGIN();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, ANNOTATION_ORIGIN> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ANNOTATION_ORIGIN_H
