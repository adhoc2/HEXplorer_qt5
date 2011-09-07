#ifndef RECORD_LAYOUT_H
#define RECORD_LAYOUT_H

#include "node.h"
#include "basefactory.h"
#include "Items/no_axis_pts_x.h"
#include "Items/no_axis_pts_y.h"
#include "Items/axis_pts_x.h"
#include "Items/axis_pts_y.h"
#include "Items/fnc_values.h"
#include "Items/src_addr_x.h"
#include "Items/src_addr_y.h"


class RECORD_LAYOUT : public Node
{
    public:
        RECORD_LAYOUT( Node *parentNode);
        ~RECORD_LAYOUT();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,RECORD_LAYOUT> nodeFactory;
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

#endif // RECORD_LAYOUT_H
