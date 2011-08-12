#ifndef PROJECT_H
#define PROJECT_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/module.h"
#include "Nodes/header.h"


class PROJECT : public Node
{
    public:
        PROJECT(QTextStream &in, Node *parentNode, A2lLexer *lexer);
        ~PROJECT();
        QMap<std::string, std::string> *getParameters();
        std::string pixmap();
        char* getPar(std::string str);
        QList<MODULE*> listModule();

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in);
};

#endif // BLOCKPROJECT_H
