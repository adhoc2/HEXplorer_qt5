#ifndef IF_DATA_H
#define IF_DATA_H

#include "node.h"
#include "basefactory.h"



class IF_DATA : public Node
{
    public:
        IF_DATA(QTextStream  &in, Node *parentNode);
        ~IF_DATA();
        static Factory<Node, IF_DATA> nodeFactory;

    private:
        void parse(QTextStream  &in);
};

#endif // IF_DATA_H
