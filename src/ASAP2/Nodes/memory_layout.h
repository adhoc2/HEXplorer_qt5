#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/if_data.h"

class MEMORY_LAYOUT : public Node
{
    public:
        MEMORY_LAYOUT( Node *parentNode);
        ~MEMORY_LAYOUT();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node,MEMORY_LAYOUT> nodeFactory;
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        //#pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar);
};

#endif // MEMORY_LAYOUT_H
