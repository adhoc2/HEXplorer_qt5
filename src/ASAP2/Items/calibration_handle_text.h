#ifndef CALIBRATION_HANDLE_TEXT_H
#define CALIBRATION_HANDLE_TEXT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class CALIBRATION_HANDLE_TEXT : public Item
{
    public:
        CALIBRATION_HANDLE_TEXT( Node *parentNode);
        ~CALIBRATION_HANDLE_TEXT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, CALIBRATION_HANDLE_TEXT> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // CALIBRATION_HANDLE_TEXT_H
