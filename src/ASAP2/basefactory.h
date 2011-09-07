#ifndef BASEFACTORY_H
#define BASEFACTORY_H

#include "node.h"
#include "lexer.h"

// Templates for the generic factory and factory plant
template <typename BT>
class FactoryPlant
   {
   public:
      FactoryPlant() {}
      virtual ~FactoryPlant() {}
      virtual BT *createInstance(Node*) = 0;
   };

template <typename BT,typename ST>
class Factory : public FactoryPlant<BT>
   {
   public:
      Factory() {}
      virtual ~Factory() {}
      virtual BT *createInstance(Node *parentNode)
      {
          return new ST(parentNode);
      }
   };

#endif // BASEFACTORY_H
