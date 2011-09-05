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
        A2lLexer(QObject *parent = 0);
        ~A2lLexer();

        TokenTyp getNextToken(QTextStream &in);
        std::string toString(TokenTyp type);
        std::string getLexem();
        int getLine();
        int getIndex();
        void initialize();
        void backward(QTextStream &in);
        A2lGrammar *grammar;

        int tamere;

    private:        
        Buffer *buffer;
        int position;
        int previousLine;
        int line;
        int index;
        std::string lexem;
        QHash<QString, TokenTyp> keywordsList;
        TokenTyp begin(QTextStream &in, char ch);
        TokenTyp identifier(QTextStream &in, char &ch);
        TokenTyp string(QTextStream &in);
        TokenTyp commentL(QTextStream &in);
        TokenTyp commentM(QTextStream &in);
        TokenTyp number(QTextStream &in, char &ch);
        TokenTyp hexadecimal(QTextStream &in);
        TokenTyp block(QTextStream &in, char &ch);
        TokenTyp getPartialString(QTextStream &in);

        bool isSeparator(char ch);
        bool isDigit(char ch);
        bool isHexDigit(char ch);
        bool isLetter(char ch);
        bool isA2mlSym(char ch);


    signals:
        void returnedToken(int );
};

#endif // A2LLEXER_H
