#ifndef AXIS_PTS_H
#define AXIS_PTS_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/format.h"
#include "Items/extended_limits.h"
#include "Nodes/if_data.h"
#include "Nodes/function.h"
#include "Nodes/annotation.h"
#include "Items/deposit.h"

class AXIS_PTS : public Node
{
    public:
        AXIS_PTS(QTextStream &in, Node *parentNode);
        ~AXIS_PTS();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, AXIS_PTS> nodeFactory;
        char* getPar(std::string str);
        QString getSubsetName();
        void setSubset(FUNCTION *fun);

    private:
        // specific for this node Characteristic
        FUNCTION *subset;

        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in);
};

#endif // AXIS_PTS_H
