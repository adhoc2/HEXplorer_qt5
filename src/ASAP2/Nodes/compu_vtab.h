#ifndef COMPU_VTAB_H
#define COMPU_VTAB_H

#include "node.h"
#include "basefactory.h"
#include "Items/default_value.h"


class COMPU_VTAB : public Node
{
    public:
        COMPU_VTAB(Node *parentNode);
        ~COMPU_VTAB();
        static Factory<Node, COMPU_VTAB> nodeFactory;

        QMap<std::string, std::string> *getParameters();
        std::string pixmap();
        QString getValue(int i);
        QStringList getValueList();
        int getPos(QString str);
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
        QMap<int, std::string> valuePairs;
        QList<int> listKeyPairs;
        QList<std::string> listValuePairs;

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        //#pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parsePairs();
        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar();
};

#endif // COMPU_VTAB_H
