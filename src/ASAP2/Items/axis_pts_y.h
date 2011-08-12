#ifndef AXIS_PTS_Y_H
#define AXIS_PTS_Y_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class AXIS_PTS_Y : public Item
{
    public:
        AXIS_PTS_Y(QTextStream &in, Node *parentNode);
        ~AXIS_PTS_Y();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,AXIS_PTS_Y> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar,  QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // AXIS_PTS_Y_H
