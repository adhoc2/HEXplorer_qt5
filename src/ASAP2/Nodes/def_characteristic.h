#ifndef DEF_CHARACTERISTIC_H
#define DEF_CHARACTERISTIC_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class DEF_CHARACTERISTIC : public Node
{
    public:
        DEF_CHARACTERISTIC( Node *parentNode);
        ~DEF_CHARACTERISTIC();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,DEF_CHARACTERISTIC> nodeFactory;
        std::string pixmap();
        QStringList getCharList();
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        //#pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        // CHARACTERISTICS list
        QList<std::string> charList;

        TokenTyp parseListChar();
        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar();
};

#endif // DEF_CHARACTERISTIC_H
