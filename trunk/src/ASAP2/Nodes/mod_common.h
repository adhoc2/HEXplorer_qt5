#ifndef MOD_COMMON_H
#define MOD_COMMON_H

#include "node.h"
#include "basefactory.h"
#include "Items/byte_order.h"
#include "Items/alignment_byte.h"
#include "Items/alignment_float32_ieee.h"
#include "Items/alignment_word.h"
#include "Items/alignment_long.h"
#include "Items/s_rec_layout.h"


class MOD_COMMON : public Node
{
    public:
        MOD_COMMON( Node *parentNode);
        ~MOD_COMMON();
        QMap<std::string, std::string> *getParameters();
        char* fixPar(int n);
        static Factory<Node,MOD_COMMON> nodeFactory;
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

#endif // MOD_COMMON_H
