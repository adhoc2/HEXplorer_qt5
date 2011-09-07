#ifndef CALIBRATION_HANDLE_H
#define CALIBRATION_HANDLE_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/calibration_handle_text.h"

class CALIBRATION_HANDLE : public Node
{
    public:
        CALIBRATION_HANDLE( Node *parentNode);
        ~CALIBRATION_HANDLE();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, CALIBRATION_HANDLE> nodeFactory;
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

};

#endif // CALIBRATION_HANDLE_H
