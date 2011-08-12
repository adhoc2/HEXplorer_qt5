#ifndef A2ML_H
#define A2ML_H

#include "node.h"
#include "basefactory.h"


class A2ML : public Node
{
    public:
        A2ML(QTextStream &in, Node *parentNode);
        static Factory<Node, A2ML> nodeFactory;       

    private:
        void parse(QTextStream &in);
};

#endif // BLOCKA2ML_H
