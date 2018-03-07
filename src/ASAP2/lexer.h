// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

#ifndef LEXER_H
#define LEXER_H

#include <QList>
#include <QString>
#include <QTextStream>
#include <QHash>
#include "a2l_quex_lexer"

using namespace quex;


class A2lGrammar;

enum TokenTyp
{
    Identifier ,String, StringFormat, Float, Integer, Comment, Hex, myUnknown, Plus, Minus, BlockComment,
    BlockBegin, BlockEnd, Eof, Eol, ValueSeparator, Text,
    Keyword, DataType, Datasize, Addrtype, Byteorder, Indexorder, ConversionType, Type, Attribute, Mode,
    PrgType, MemoryType, MemAttribute, IndexMode, Indentation, UnitType
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
        //A2lLexer(QTextStream &in, QObject *parent = 0);
        A2lLexer(QTextStream &in, QObject *parent = 0);
        A2lLexer(QObject *parent = 0);
        ~A2lLexer();

        virtual TokenTyp getNextToken();
        virtual std::string toString(TokenTyp type);
        virtual std::string getLexem();
        virtual int getLine();
        virtual void initialize();
        virtual void backward(int i = 0);
        A2lGrammar *grammar;
        QTextStream *in;

    private:        
        Buffer *buffer;
        int position;
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

#include <sstream>

class A2lQuexLexer : public A2lLexer
{
     Q_OBJECT

    public:
        //A2lQuexLexer(std::istringstream &in, QObject *parent = 0);
        A2lQuexLexer(QUEX_NAME(ByteLoader)* byteLoader, QObject *parent = 0);

        ~A2lQuexLexer();

        TokenTyp getNextToken();
        std::string toString(TokenTyp type);
        std::string getLexem();
        int getLine();
        int getIndex();
        void initialize();
        void backward(int i = 0);

    private:
        quex::a2l_quex_lexer  *qlex;
        quex::Token *token_p;
        int position;
        int line;
        int index;
        int previousLine;
        std::string lexem;
        QHash<QString, TokenTyp> keywordsList;
        TokenTyp myToken(quex::Token* token_p);

};

#endif // A2LLEXER_H
