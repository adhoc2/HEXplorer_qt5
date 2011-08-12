#ifndef SUB_FUNCTION_H
#define SUB_FUNCTION_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"


class SUB_FUNCTION : public Node
{
    public:
        SUB_FUNCTION(QTextStream &in, Node *parentNode);
        ~SUB_FUNCTION();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,SUB_FUNCTION> nodeFactory;
        std::string pixmap();
        QStringList getCharList();
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

        // CHARACTERISTICS list
        QList<std::string> charList;

        TokenTyp parseListChar(QTextStream &in);
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        TokenTyp parseOptPar(QTextStream &in);
};

#endif // SUB_FUNCTION_H
