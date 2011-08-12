#include "threadparse.h"
#include "a2lgrammar.h"

void ThreadParse::run()
{
    QTextStream out1(&str);

    A2lLexer *lexer1 = new A2lLexer();
    lexer1->initialize();

    QStringList *errorList1 = new QStringList();
    A2lGrammar *gram = new A2lGrammar();
    nodeA2l1 = new A2LFILE(out1, 0, lexer1, errorList1, gram);
    nodeA2l1->name = (char*)"fileName";

    delete lexer1;
}

