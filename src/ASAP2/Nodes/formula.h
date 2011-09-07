#ifndef FORMULA_H
#define FORMULA_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/formula_inv.h"

class FORMULA : public Node
{
    public:
        FORMULA(Node *parentNode);
        ~FORMULA();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,FORMULA> nodeFactory;
        std::string pixmap();
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

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar();
};

#endif // FORMULA_H
