#ifndef CALIBRATION_METHOD_H
#define CALIBRATION_METHOD_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/calibration_handle.h"

class CALIBRATION_METHOD : public Node
{
    public:
        CALIBRATION_METHOD(QTextStream &in, Node *parentNode);
        ~CALIBRATION_METHOD();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, CALIBRATION_METHOD> nodeFactory;
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

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in);
};

#endif // CALIBRATION_METHOD_H
