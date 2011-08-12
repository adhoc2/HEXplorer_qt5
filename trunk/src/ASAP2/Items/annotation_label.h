#ifndef ANNOTATION_LABEL_H
#define ANNOTATION_LABEL_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ANNOTATION_LABEL : public Item
{
    public:
        ANNOTATION_LABEL(QTextStream &in, Node *parentNode);
        ~ANNOTATION_LABEL();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, ANNOTATION_LABEL> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // ANNOTATION_LABEL_H
