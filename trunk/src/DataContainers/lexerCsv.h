#ifndef LEXERCSV_H
#define LEXERCSV_H

#include <QList>
#include <QString>
#include <QTextStream>
#include <QHash>
#include "lexer.h"


class LexerCsv : public QObject
{
    Q_OBJECT

    public:
        LexerCsv(QObject *parent = 0);
        ~LexerCsv();

        TokenTyp getNextToken(QTextStream &in);
        std::string toString(TokenTyp type);
        std::string getLexem();
        TokenTyp getToken() {return actToken;}
        int getLine();
        int getIndex();
        void initialize();
        void backward(QTextStream &in);

        char valueSeparator;
        char decimalPointSeparator;
        char commentIndicator;
        char stringDelimiter;

    private:

        int position;
        Buffer *buffer;
        void incLine();
        int nTotalLines;
        std::string lexem;
        TokenTyp actToken;
        QHash<QString, TokenTyp> KeywordsMap;
        TokenTyp begin(QTextStream &in, char ch);
        TokenTyp indentation(QTextStream &in, char &ch);
        TokenTyp identifier(QTextStream &in, char &ch);
        TokenTyp  string(QTextStream &in);
        TokenTyp  commentL(QTextStream &in);
        TokenTyp  commentM(QTextStream &in);
        TokenTyp  number(QTextStream &in, char &ch);
        TokenTyp  hexadecimal(QTextStream &in);
        TokenTyp  block(QTextStream &in, char &ch);
        bool isNewLine(char ch);
        bool isValueSeparator(char ch);
        bool isDigit(char ch);
        bool isHexDigit(char ch);
        bool isLetter(char ch);
        bool isA2mlSym(char ch);
        TokenTyp getPartialString(QTextStream &in);

    signals:
        void returnedToken(int n);
};

#endif // LEXERCSV_H
