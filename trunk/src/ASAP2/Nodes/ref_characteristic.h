#ifndef REF_CHARACTERISTIC_H
#define REF_CHARACTERISTIC_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class REF_CHARACTERISTIC : public Node
{
    public:
        REF_CHARACTERISTIC(Node *parentNode);
        ~REF_CHARACTERISTIC();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,REF_CHARACTERISTIC> nodeFactory;
        std::string pixmap();
        QStringList getCharList();
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
        void parseFixPar(QList<TokenTyp> *typePar);

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;
        TokenTyp parseOptPar(QTextStream  &in);

        // CHARACTERISTICS list
        QList<std::string> charList;
        TokenTyp parseListChar();
};

#endif // REF_CHARACTERISTIC_H
