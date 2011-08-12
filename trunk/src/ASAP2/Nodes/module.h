#ifndef MODULE_H
#define MODULE_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/axis_pts.h"
#include "Nodes/a2ml.h"
#include "Nodes/measurement.h"
#include "Nodes/mod_par.h"
#include "Nodes/if_data.h"
#include "Nodes/function.h"
#include "Nodes/mod_common.h"
#include "Nodes/compu_method.h"
#include "Nodes/compu_vtab.h"
#include "Nodes/record_layout.h"
#include "Nodes/characteristic.h"

class MODULE : public Node
{
    public:
        MODULE(QTextStream &in, Node *parentNode);
        ~MODULE();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, MODULE> nodeFactory;
        char* getPar(std::string str);
        QStringList listChar;

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

      //  #pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in);
};

#endif // MODULE_H
