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

#include "lexerDcm.h"
#include <sstream>
#include "qdebug.h"

/**********  Class Lexer *************/
LexerDcm::LexerDcm(QObject *parent) : QObject(parent)
{
    // --- define Keywords
    QList<std::string> list;
    list << "KONSERVIERUNG_FORMAT" << "END" << "FUNKTIONEN" << "FKT" << "KENNLINIE"
         << "VARIANENTKODIERUNG" << "KRITERIUM" << "MODULKOPF" << "FESTWERT" << "LANGNAME"
         << "DISPLAYNAME" << "VAR" << "FUNKTION" << "EINHEIT_W" << "WERT" << "TEXT"
         << "FESTWERTEBLOCK" << "ST/X" << "EINHEIT_X" << "KENNFELD" << "EINHEIT_Y" << "ST/Y"
         << "FESTKENNLINIE" << "FESTKENNFELD" << "GRUPPENKENNLINIE" << "GRUPPENKENNFELD"
         << "STUETZSTELLENVERTEILUNG";

    foreach (std::string str, list)
    {
        KeywordsMap.insert(QString(str.c_str()), Keyword);
    }
    list.clear();


    // --- initialize members
    buffer = new Buffer();
    valueSeparator = 0;
    commentIndicator = '*';
    stringDelimiter = '"';
    decimalPointSeparator = '.';

    position = 0;
}

LexerDcm::~LexerDcm()
{
    delete buffer;
}

std::string LexerDcm::getLexem()
{
    return lexem;
}

std::string LexerDcm::toString(TokenTyp type)
{
    switch (type)
    {
        case myUnknown:
            return "Unknown";
            break;
        case Identifier:
            return "Identifier";
            break;
        case Indentation:
            return "Indentation";
            break;
        case String:
            return "String";
            break;
        case Hex:
            return "Hex";
            break;
        case Integer:
            return "Integer";
            break;
        case Comment:
            return "Comment";
            break;
        case Float:
            return "Float";
            break;
        case Plus:
            return "+";
            break;
        case Minus:
            return "-";
            break;
        case BlockBegin:
            return "BlockBegin";
            break;
        case BlockEnd:
            return "BlockEnd";
            break;
        case Eof:
            return "Eof";
            break;
        case StringFormat:
            return "StringFormat";
            break;
        case Keyword:
            return "Keyword";
            break;
        default:
            return "Unknown";
    }
}

TokenTyp LexerDcm::getNextToken(QTextStream &in)
{
    lexem = "";
    TokenTyp token;
    char ch;

    //First check if the buffer is empty or full
    //and process it if necessary
    if (buffer->isFull())
    {
        token = begin(in, buffer->getAndClear());
    }
    //Else read next char
    else
    {
        if (in.atEnd())
        {
            token = Eof;
            buffer->clear();
        }
        else
        {
            in >> ch;
            token = begin(in, ch);
        }
    }

    actToken = token;

    // emit erturn token for progressBar
    if (in.pos() - position > 20000 || in.atEnd())
    {
        emit returnedToken(in.pos() - position);
        position = in.pos();
    }


    return token;
}

TokenTyp LexerDcm::begin(QTextStream &in, char ch)
{
    TokenTyp token = myUnknown;

    while(1)
    {
        //count new lines
        while (isNewLine(ch))
        {
            if (ch == '\n') incLine();
            in >> ch;
        }

        //if at end of file
        if (ch == 0)
        {
            token = Eof;
            buffer->clear();
            return token;
        }
        else
        {
            if (isSeparator(ch))
            {
                buffer->read(in);
                ch = buffer->getAndClear();
            }
            else if (ch == commentIndicator)
            {
                 token = commentL(in);
                 return token;
            }
            else if (ch == stringDelimiter)
            {
                token = string(in);
                return token;
            }
            else if (ch == '+' || ch == '-' || isDigit(ch))
            {
                token = number(in, ch);
                return token;
            }
            else if(isLetter(ch) || ch == '_')
            {
                token = identifier(in, ch);
                return token;
            }
            else if (isNewLine(ch))
            {
            }
            else
            {
                lexem += ch;
                buffer->clear();
                return token;
            }
        }
    }
}

TokenTyp LexerDcm::indentation(QTextStream &in, char &ch)
{
    lexem = "";
    lexem += ch;

    // check the next character
    buffer->read(in);
    if (isValueSeparator(buffer->getValue()))
    {
        lexem += buffer->getAndClear();
        return Indentation;
    }
    else
    {
        ch = buffer->getAndClear();
        return myUnknown;
    }
}

TokenTyp LexerDcm::commentM(QTextStream &in)
{
    TokenTyp token;

    token = Comment;
    lexem = "/*";

    bool exit = false;
    while (!exit)
    {
        buffer->read(in);
        if (buffer->getValue() == '*')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            while (buffer->getValue() == '*')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }

            if (buffer->getValue() == '/')
            {
                lexem += buffer->getAndClear();
                exit = true;
            }
            else if (buffer->getValue() == 0)
                exit = true;
            else
            {
                if (buffer->getValue() == '\n')
                    this->incLine();
                lexem +=  buffer->getAndClear();
            }

        }
        else if (buffer->getValue() == 0)
        {
            exit = true;
        }
        else
        {
            if (buffer->getValue() == '\n')
                this->incLine();
            lexem +=  buffer->getAndClear();
        }

    }
    return token;
}

TokenTyp LexerDcm::commentL(QTextStream &in)
{
    TokenTyp token;

    token = Comment;
    lexem = commentIndicator;

    buffer->read(in);
    while (buffer->getValue() != '\n' && buffer->getValue() != '\r' && buffer->getValue() != 0)
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
    }
    return token;
}

TokenTyp LexerDcm::block(QTextStream &in, char &ch)
{
    TokenTyp token;
    lexem += '/';
    lexem += ch;

    buffer->read(in);
    while (isLetter(buffer->getValue()) || isDigit(buffer->getValue()) || buffer->getValue() == '_')
    {
         lexem += buffer->getAndClear();
         buffer->read(in);
    }

    if (lexem == "/begin")
        token = BlockBegin;
    else if (lexem == "/end")
        token = BlockEnd;
    else
        token = myUnknown;

    return token;
}

TokenTyp LexerDcm::identifier(QTextStream &in, char &ch)
{
    TokenTyp token = Identifier;
    lexem = "";
    lexem += ch;

    buffer->read(in);

    bool exit = false;
    while (!exit)
    {
        token = getPartialString(in);
        if (token == Identifier)
        {
            if (buffer->getValue() == '.')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            else if (isNewLine(buffer->getValue()))
            {
                exit = true;
            }
            else if (isSeparator(buffer->getValue()))
            {
                exit = true;
            }
            else
            {
                lexem += buffer->getAndClear();
                token = myUnknown;
                exit = true;
            }
        }
        else
            exit = true;
    }

   if (token == Identifier)
    {

        TokenTyp tok = KeywordsMap.value(QString(lexem.c_str()));
        if (tok != 0)
            token = tok;
    }

    return token;
}

TokenTyp LexerDcm::number(QTextStream &in, char &ch)
{
    TokenTyp token;
    lexem = "";
    lexem += ch;

    buffer->read(in);
    while(isDigit(buffer->getValue()))
    {        
        lexem += buffer->getAndClear();
        buffer->read(in);
    }

    if (buffer->getValue() == decimalPointSeparator)
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
        while(isDigit(buffer->getValue()))
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->getValue() == 'E' || buffer->getValue() == 'e')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            if (buffer->getValue() == '+' || buffer->getValue() == '-')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
                while(isDigit(buffer->getValue()))
                {
                    lexem += buffer->getAndClear();
                    buffer->read(in);
                }
                token = Float;
                return token;
            }
            else
            {
                token = myUnknown;
                return token;
            }

        }
        else
        {
            token = Float;
            return token;
        }

    }
    else if(buffer->getValue() == 'E' || buffer->getValue() == 'e')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
        if (buffer->getValue() == '+' || buffer->getValue() == '-')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            while(isDigit(buffer->getValue()))
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            token = Float;
            return token;
        }
        else
        {
            token = myUnknown;
            return token;
        }
    }
    else if (buffer->getValue() == 'x')
    {
         token = hexadecimal(in);
         return token;
    }
    else
    {
        token = Integer;
        return token;
    }
}

TokenTyp LexerDcm::string(QTextStream &in)
{
    TokenTyp token;
    //lexem = stringDelimiter;
    lexem = "";

    buffer->read(in);

    token = String;
    bool exit = false;
    while (!exit)
    {        
        while (buffer->getValue() != stringDelimiter && buffer->getValue() != '\\' && buffer->getValue() != '\n' && buffer->getValue() != 0 )
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->getValue() == stringDelimiter)
        {
            //lexem += buffer->getAndClear();
            buffer->read(in);
            if (buffer->getValue() == stringDelimiter)
            {
                //lexem += buffer->getAndClear();
                buffer->read(in);
            }
            else
                exit = true;

        }
        else if (buffer->getValue() == '\\')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            lexem += buffer->getAndClear();
            buffer->read(in);
        }
        else
        {
            token = myUnknown;
            exit = true;
        }
    }

    return token;
}

TokenTyp LexerDcm::hexadecimal(QTextStream &in)
{
    TokenTyp token;
    token = Hex;
    lexem = "0x";

    buffer->read(in);
    while (isHexDigit(buffer->getValue()))
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
    }
    return token;
}

TokenTyp LexerDcm::getPartialString(QTextStream &in)
{
    TokenTyp token = Identifier;

    while (isDigit(buffer->getValue()) || isLetter(buffer->getValue()) || buffer->getValue() == '_' ||  buffer->getValue() == '/')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);        
    }

    if (buffer->getValue() == '[')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);        

        while (isDigit(buffer->getValue()) || isLetter(buffer->getValue()) || buffer->getValue() == '_')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->getValue() == ']')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }
        else
            token = myUnknown;
    }
    else if (buffer->getValue() == '.')
    {
    }
    else if (isSeparator(buffer->getValue()))
    {
    }
    else if (!isNewLine(buffer->getValue()))
    {
        lexem += buffer->getAndClear();
        token = myUnknown;
        buffer->read(in);
    }

    return token;
}

void LexerDcm::incLine()
{
    nTotalLines++;
}

int LexerDcm::getLine()
{
    return this->nTotalLines;
}

void LexerDcm::initialize()
{
    nTotalLines = 0;
}

void LexerDcm::backward(QTextStream &in)
{
    int l = lexem.length() + 1;
    in.seek(in.pos() - l);
}

bool LexerDcm::isNewLine(char ch)
{
    if (ch == '\r' || ch == '\n')
        return true;
    else
        return false;
}

bool LexerDcm::isSeparator(char ch)
{
    if (ch == '\n' || ch == '\r' || ch == '\t' || ch == 0 || ch == 32)
        return true;
    else
        return false;
}

bool LexerDcm::isValueSeparator(char ch)
{
    if (ch == valueSeparator)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool LexerDcm::isDigit(char ch)
{
    if ((ch <= '9') && (ch >= '0'))
        return true;
    else
        return false;
}

bool LexerDcm::isHexDigit(char ch)
{
    if ((('0' <= ch) && (ch <= '9')) || (('A' <= ch) && (ch <= 'F'))  || (('a' <= ch) && (ch <= 'f')))
        return true;
    else
        return false;
}

bool LexerDcm::isLetter(char ch)
{
    if ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A'))
        return true;
    else
        return false;
}

bool LexerDcm::isA2mlSym(char ch)
{
    if (ch == '{' || ch == '}' ||
        ch == '[' || ch == ']' ||
        ch == '(' || ch == ')' ||
        ch == ';' || ch == '*' ||
        ch == '=' || ch == ',' ||
        ch == ':')
        return true;
    else
        return false;

}
