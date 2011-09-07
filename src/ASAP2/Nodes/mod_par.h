#ifndef MOD_PAR_H
#define MOD_PAR_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/memory_segment.h"
#include "Nodes/memory_layout.h"
#include "Nodes/calibration_method.h"
#include "Items/version.h"
#include "Items/addr_epk.h"
#include "Items/epk.h"
#include "Items/customer_no.h"
#include "Items/user.h"
#include "Items/phone_no.h"
#include "Items/ecu.h"
#include "Items/cpu_type.h"
#include "Items/system_constant.h"

class MOD_PAR : public Node
{
    public:
        MOD_PAR( Node *parentNode);
        ~MOD_PAR();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, MOD_PAR> nodeFactory;
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

       // #pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar);
};

#endif // MOD_PAR_H
