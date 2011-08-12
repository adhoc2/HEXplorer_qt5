#ifndef NO_AXIS_PTS_Y_H
#define NO_AXIS_PTS_Y_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class NO_AXIS_PTS_Y : public Item
{
    public:
        NO_AXIS_PTS_Y(QTextStream &in, Node *parentNode);
        ~NO_AXIS_PTS_Y();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,NO_AXIS_PTS_Y> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // NO_AXIS_PTS_Y_H
