#ifndef CPU_TYPE_H
#define CPU_TYPE_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class CPU_TYPE : public Item
{
    public:
        CPU_TYPE(QTextStream &in, Node *parentNode);
        ~CPU_TYPE();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,CPU_TYPE> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // CPU_TYPE_H
