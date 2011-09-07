#ifndef A2LFILE_H
#define A2LFILE_H

#include "node.h"
#include "lexer.h"
#include <QMap>
#include <QPixmap>
#include "Nodes/project.h"


class WorkProject;
class A2lGrammar;

class A2LFILE : public Node
{
    public:
        A2LFILE(Node *parentNode, A2lLexer *lexer,
                QStringList *errorList, QString fullFileName = "");       
        ~A2LFILE();

        std::string pixmap();
        QString fullName();
        PROJECT *getProject();
        bool isConform();

    private:
        PROJECT *project; //ASAP2 file must contain exactly one PROJECT
        QString fullA2lName;
        void parser();
        void getAsap2Version();
        void getA2mlVersion();

        QMap<std::string, std::string> *optParameters;

};

#endif // BLOCKA2LFILE_H
