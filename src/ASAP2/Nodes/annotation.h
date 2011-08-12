#ifndef ANNOTATION_H
#define ANNOTATION_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/annotation_label.h"
#include "Items/annotation_origin.h"
#include "Nodes/annotation_text.h"

class ANNOTATION : public Node
{
    public:
        ANNOTATION(QTextStream &in, Node *parentNode);
        ~ANNOTATION();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, ANNOTATION> nodeFactory;
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

        void parse(QTextStream &in);
    };

#endif // VERSION_H
