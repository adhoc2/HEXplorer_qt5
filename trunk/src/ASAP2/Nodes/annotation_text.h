#ifndef ANNOTATION_TEXT_H
#define ANNOTATION_TEXT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class ANNOTATION_TEXT : public Node
{
    public:
        ANNOTATION_TEXT( Node *parentNode);
        ~ANNOTATION_TEXT();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, ANNOTATION_TEXT> nodeFactory;
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

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar);

        // only for ANNOTATION_TEXT
        TokenTyp parseListString();
};

#endif // ANNOTATION_TEXT_H
