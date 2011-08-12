#ifndef MEMORY_SEGMENT_H
#define MEMORY_SEGMENT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/if_data.h"

class MEMORY_SEGMENT : public Node
{
    public:
        MEMORY_SEGMENT(QTextStream &in, Node *parentNode);
        ~MEMORY_SEGMENT();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node,MEMORY_SEGMENT> nodeFactory;
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

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in);
};

#endif // MEMORY_SEGMENT_H
