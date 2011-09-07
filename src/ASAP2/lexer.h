#ifndef LEXER_H
#define LEXER_H

#include <QList>
#include <QString>
#include <QTextStream>
#include <QHash>

class A2lGrammar;

enum TokenTyp
{
    Identifier ,String, StringFormat, Float, Integer, Comment, Hex, myUnknown, Plus, Minus, BlockComment,
    BlockBegin, BlockEnd, Eof,
    Keyword, DataType, Datasize, Addrtype, Byteorder, Indexorder, ConversionType, Type, Attribute, Mode,
    PrgType, MemoryType, MemAttribute, IndexMode, Indentation
};

enum Occurence
{
   Zero, ZeroOrOne, ZeroOrMore
};

class Buffer
{
    public:
        Buffer();
        void read(QTextStream *in);
        void read(QTextStream &in);
        char getAndClear();
        char getValue();
        void clear();
        bool isFull();

    private:
        bool state;
        char value;
};

class A2lLexer : public QObject
{
    Q_OBJECT

    public:
        A2lLexer(QTextStream &in, QObject *parent = 0);
        ~A2lLexer();

        TokenTyp getNextToken();
        std::string toString(TokenTyp type);
        std::string getLexem();
        int getLine();
        void initialize();
        void backward();
        A2lGrammar *grammar;
        QTextStream *in;

    private:        
        Buffer *buffer;
        int position;
        int previousLine;
        int line;
        int index;
        std::string lexem;
        QHash<QString, TokenTyp> keywordsList;
        QString keywords;
        TokenTyp begin(char ch);
        TokenTyp identifier(char &ch);
        TokenTyp string();
        TokenTyp commentL();
        TokenTyp commentM();
        TokenTyp number(char &ch);
        TokenTyp hexadecimal();
        TokenTyp block(char &ch);
        TokenTyp getPartialString();

        bool isSeparator(char ch);
        bool isDigit(char ch);
        bool isHexDigit(char ch);
        bool isLetter(char ch);
        bool isA2mlSym(char ch);


    signals:
        void returnedToken(int );
};

#endif // A2LLEXER_H
