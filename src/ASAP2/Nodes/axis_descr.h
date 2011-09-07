#ifndef AXIS_DESCR_H
#define AXIS_DESCR_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/format.h"
#include "Items/extended_limits.h"
#include "Items/deposit.h"
#include "Items/axis_pts_ref.h"
#include "Items/fix_axis_par.h"

class AXIS_DESCR : public Node
{
    public:
        AXIS_DESCR(Node *parentNode);
        ~AXIS_DESCR();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, AXIS_DESCR> nodeFactory;
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

#endif // CHARACTERISTIC_H
