// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoël>
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
