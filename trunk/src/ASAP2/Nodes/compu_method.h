#ifndef COMPU_METHOD_H
#define COMPU_METHOD_H

#include "node.h"
#include "basefactory.h"
#include "Items/coeffs.h"
#include "Items/coeffs_linear.h"
#include "Items/compu_tab_ref.h"
#include "Items/ref_unit.h"
#include "Items/status_string_ref.h"
#include "Nodes/formula.h"


class COMPU_METHOD : public Node
{
    public:
        COMPU_METHOD(QTextStream &in, Node *parentNode);
        ~COMPU_METHOD();
        static Factory<Node, COMPU_METHOD> nodeFactory;

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

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QTextStream &in);
};

#endif // COMPU_METHOD_H
