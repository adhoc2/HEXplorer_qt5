#ifndef OUT_MEASUREMENT_H
#define OUT_MEASUREMENT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class OUT_MEASUREMENT : public Node
{
    public:
        OUT_MEASUREMENT( Node *parentNode);
        ~OUT_MEASUREMENT();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,OUT_MEASUREMENT> nodeFactory;
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

#endif // OUT_MEASUREMENT_H
