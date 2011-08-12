#ifndef FUNCTION_VERSION_H
#define FUNCTION_VERSION_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class FUNCTION_VERSION : public Item
{
    public:
        FUNCTION_VERSION(QTextStream &in, Node *parentNode);
        ~FUNCTION_VERSION();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,FUNCTION_VERSION> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // FUNCTION_VERSION_H
