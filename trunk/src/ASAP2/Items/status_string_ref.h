#ifndef STATUS_STRING_REF_H
#define STATUS_STRING_REF_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class STATUS_STRING_REF : public Item
{
    public:
        STATUS_STRING_REF(QTextStream &in, Node *parentNode);
        ~STATUS_STRING_REF();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, STATUS_STRING_REF> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // STATUS_STRING_REF_H
