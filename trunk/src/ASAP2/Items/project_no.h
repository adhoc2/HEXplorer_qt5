#ifndef PROJECT_NO_H
#define PROJECT_NO_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class PROJECT_NO : public Item
{
    public:
        PROJECT_NO(QTextStream &in, Node *parentNode);
        ~PROJECT_NO();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,PROJECT_NO> itemFactory;
        static void initialize();
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // PROJECT_NO_H
