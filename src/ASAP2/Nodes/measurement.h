#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/format.h"
#include "Items/ecu_address.h"
#include "Items/array_size.h"
#include "Items/bit_mask.h"
#include "Items/byte_order.h"
#include "Nodes/if_data.h"

class MEASUREMENT : public Node
{
    public:
        MEASUREMENT(QTextStream &in, Node *parentNode);
        ~MEASUREMENT();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, MEASUREMENT> nodeFactory;
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

       // #pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QTextStream &in);
};

#endif // MEASUREMENT_H
