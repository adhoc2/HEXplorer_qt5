#ifndef FUNCTION_H
#define FUNCTION_H

#include "node.h"
#include "basefactory.h"
#include "Nodes/def_characteristic.h"
#include "Nodes/ref_characteristic.h"
#include "Nodes/loc_measurement.h"
#include "Nodes/in_measurement.h"
#include "Nodes/out_measurement.h"
#include "Nodes/sub_function.h"
#include "Items/function_version.h"

class FUNCTION : public Node
{
    public:
        FUNCTION( Node *parentNode);
        ~FUNCTION();
        static Factory<Node, FUNCTION> nodeFactory;

        QMap<std::string, std::string> *getParameters();
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

#endif // FUNCTION_H
