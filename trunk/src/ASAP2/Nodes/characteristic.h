#ifndef CHARACTERISTIC_H
#define CHARACTERISTIC_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/format.h"
#include "Items/extended_limits.h"
#include "Items/max_refresh.h"
#include "Nodes/if_data.h"
#include "Nodes/annotation.h"
#include "Nodes/axis_descr.h"
#include "Nodes/function.h"
#include "Items/number.h"
#include "Items/read_only.h"

class CHARACTERISTIC : public Node
{
    public:
        CHARACTERISTIC(QTextStream &in, Node *parentNode);
        ~CHARACTERISTIC();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,CHARACTERISTIC> nodeFactory;
        std::string pixmap();
        char* getPar(std::string str);
        QString getSubsetName();
        void setSubset(FUNCTION *fun);

    private:
        // specific only for this node Characteristic
        FUNCTION *subset;

        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;        
        TokenTyp parseOptPar(QTextStream &in);
};

#endif // CHARACTERISTIC_H
