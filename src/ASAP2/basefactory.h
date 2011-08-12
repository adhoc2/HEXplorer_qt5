#ifndef BASEFACTORY_H
#define BASEFACTORY_H

#include <QTextStream>
#include "node.h"
#include "lexer.h"

// Templates for the generic factory and factory plant
template <typename BT>
class FactoryPlant
   {
   public:
      FactoryPlant() {}
      virtual ~FactoryPlant() {}
      virtual BT *createInstance(QTextStream&, Node*) = 0;
   };

template <typename BT,typename ST>
class Factory : public FactoryPlant<BT>
   {
   public:
      Factory() {}
      virtual ~Factory() {}
      virtual BT *createInstance(QTextStream &in, Node *parentNode)
      {
          return new ST(in, parentNode);
      }
   };

#endif // BASEFACTORY_H
