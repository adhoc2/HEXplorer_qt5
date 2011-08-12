#ifndef NO_AXIS_PTS_X_H
#define NO_AXIS_PTS_X_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class NO_AXIS_PTS_X : public Item
{
    public:
        NO_AXIS_PTS_X(QTextStream &in, Node *parentNode);
        ~NO_AXIS_PTS_X();
        QMap<std::string, std::string> getParameters();
        static Factory<Item,NO_AXIS_PTS_X> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar, QTextStream &in);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // NO_AXIS_PTS_X_H
