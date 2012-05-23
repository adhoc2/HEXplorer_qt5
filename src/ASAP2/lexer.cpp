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

#include "lexer.h"
#include "a2lgrammar.h"
#include <sstream>
#include "qdebug.h"
#include <QTime>

/**********  Class Buffer *************/

Buffer::Buffer()
{
    state= false;
    value = 0;
}

bool Buffer::isFull()
{
    return state;
}

void Buffer::read(QTextStream *in)
{
        char c;
        *in >> c;

        if (c != 0 && c < 128)
        {
            value = c;
            state = true;
        }
        else if (c > 127)
        {
            value = '?';
            state = true;
        }
        else
        {
            value = 0;
            state = false;
        }


//    QChar c;
//    in >> c;

    
//    if (c != 0 && c.unicode() < 128)
//    {
//        value = c.toAscii();
//        state = true;
//    }
//    else if (c.unicode() > 127)
//    {
//        value = '?';
//        state = true;
//    }

//    else
//    {
//        value = 0;
//        state = false;
//    }
}

void Buffer::read(QTextStream &in)
{
        char c;
        in >> c;

        if (c != 0 && c < 128)
        {
            value = c;
            state = true;
        }
        else if (c > 127)
        {
            value = '?';
            state = true;
        }
        else
        {
            value = 0;
            state = false;
        }
}

char Buffer::getAndClear()
{
    char c = value;
    clear();
    return c;
}

char Buffer::getValue()
{
    //char c = value;
    return value;
}

void Buffer::clear()
{
    this->value = 0;
    this->state = false;
}

/**********  Class Lexer *************/

A2lLexer::A2lLexer(QTextStream &in, QObject *parent) : QObject(parent)
{
    grammar = new A2lGrammar();
    keywordsList = grammar->initKeywords();
    buffer = new Buffer();
    index = 0;
    position = 0;
    this->in = &in;
}

A2lLexer::A2lLexer(QObject *parent) : QObject(parent)
{
        buffer = new Buffer();
}

A2lLexer::~A2lLexer()
{
    delete grammar;
    delete buffer;
}

std::string A2lLexer::getLexem()
{
    return lexem;
}

std::string A2lLexer::toString(TokenTyp type)
{
    switch (type)
    {
        case myUnknown:
            return "Unknown";
            break;
        case Identifier:
            return "Identifier";
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
        case Mode:
            return "Mode";
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
        case DataType:
            return "DataType";
            break;
        case Datasize:
            return "Datasize";
            break;
        case Addrtype:
            return "Addrtype";
            break;
        case Byteorder:
            return "Byteorder";
            break;
        case Indexorder:
            return "Indexorder";
            break;
        case IndexMode:
            return "Indexmode";
            break;
        case ConversionType:
            return "ConversionType";
            break;
        case Type:
            return "Type";
            break;
        case Attribute:
            return "Attribute";
            break;
        case PrgType:
            return "PrgType";
            break;
        case MemoryType:
            return "MemoryType";
            break;
        case MemAttribute:
            return "MemAttribute";
            break;
        case UnitType:
            return "UnitType";
            break;
        default:
            return "Unknown";
    }
}

TokenTyp A2lLexer::getNextToken()
{
    lexem = "";
    TokenTyp token = myUnknown;
    char ch;

    //First check if the buffer is empty or full
    //and process it if necessary
    if (buffer->isFull())
    {
        token = begin(buffer->getAndClear());
    }
    //Else read next char
    else
    {
        if (in->atEnd())
        {
            token = Eof;
            buffer->clear();
        }
        else
        {
//            in >> ch;
//            token = begin(in, ch);
            buffer->read(this->in);
            token = begin(buffer->getAndClear());
        }
    }

    // emit return token for progressBar
    if (in->pos() - position > 40000 || in->atEnd())
    {
        emit returnedToken(in->pos() - position);
        position = in->pos();
    }

    return token;
}

bool A2lLexer::isSeparator(char ch)
{
    if (ch == '\n' || ch == '\r' || ch == '\t' || ch == 0 || ch == 32)
        return true;
    else
        return false;
}

bool A2lLexer::isDigit(char ch)
{
    if ((ch <= '9') && (ch >= '0'))
        return true;
    else
        return false;
}

bool A2lLexer::isHexDigit(char ch)
{
    if ((('0' <= ch) && (ch <= '9')) || (('A' <= ch) && (ch <= 'F'))  || (('a' <= ch) && (ch <= 'f')))
        return true;
    else
        return false;
}

bool A2lLexer::isLetter(char ch)
{
    if ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A'))
        return true;
    else
        return false;
}

bool A2lLexer::isA2mlSym(char ch)
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

TokenTyp A2lLexer::begin(char ch)
{
    TokenTyp token = myUnknown;

    //ignore white space and tabs and new line as first character
    while (ch == '\t'  || ch == '\r' || ch == 32 || ch == '\n')
    {        
        if (ch == '\n')
            line++;
        //in >> ch;
        buffer->read(in);
        ch = buffer->getAndClear();
    }

    index = in->pos() - 1;
    //if at end of file
    if (ch == 0)
    {
        token = Eof;
        buffer->clear();
    }
    else
    {
        if(isLetter(ch) || ch == '_')
        {
            token = this->identifier(ch);
        }
        else if (isDigit(ch))
        {
            token = this->number(ch);
        }
        else if (ch == '/')
        {
            *in >> ch;
            if (ch == '/')
                token = commentL();
            else if (ch == '*')
                token = commentM();
            else if (isLetter(ch))
                token = block(ch);
            else
            {
                token = myUnknown;
                lexem = "/" + ch;
            }
        }
        else if (ch == '"')
        {
            token = string();
        }
        else if (ch == '-')
        {
            //token = Minus;
            //lexem = '-';
            //buffer->clear();
            token = this->number(ch);
        }
        else if (ch == '+')
        {
            //token = Plus;
            //lexem = '+';
            //buffer->clear();
            token = this->number(ch);
        }
        /*else if (this->isA2mlSym(ch))
        {
            token = new Token;
            token = A2ml;
            lexem += ch;
            buffer->clear();
        }*/
        else
        {
            lexem += ch;
            buffer->clear();
        }
    }
     return token;
}

TokenTyp A2lLexer::commentM()
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
                    line++;
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
                line++;
            lexem +=  buffer->getAndClear();
        }

    }
    return token;
}

TokenTyp A2lLexer::commentL()
{
    TokenTyp token;

    token = Comment;
    lexem = "//";

    buffer->read(in);
    while (buffer->getValue() != '\n' && buffer->getValue() != '\r' && buffer->getValue() != 0)
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
    }
    return token;
}

TokenTyp A2lLexer::block(char &ch)
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

TokenTyp A2lLexer::identifier(char &ch)
{
    TokenTyp token = Identifier;
    lexem += ch;

    buffer->read(in);
    bool exit = false;
    while (!exit)
    {
        token = getPartialString();
        if (token == Identifier)
        {
            if (buffer->getValue() == '.')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            else if (isSeparator(buffer->getValue()))
                exit = true;
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

    //check if the identifier is a keyword
    if (token == Identifier)
    {
        TokenTyp tok = keywordsList.value(lexem.c_str());
        if (tok != 0)
            token = tok;
    }

    return token;
}

TokenTyp A2lLexer::number(char &ch)
{
    TokenTyp token;
    lexem += ch;

    buffer->read(in);
    while(isDigit(buffer->getValue()))
    {        
        lexem += buffer->getAndClear();
        buffer->read(in);
    }

    if (buffer->getValue() == '.')
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
         token = hexadecimal();
         return token;
    }
    else
    {
        token = Integer;
        return token;
    }
}

TokenTyp A2lLexer::string()
{
    TokenTyp token;
    lexem = '"';

    buffer->read(in);

    // StringFormat
    if (buffer->getValue() == '%')
    {
        token = StringFormat;
        lexem += buffer->getAndClear();
        buffer->read(in);
        while(isDigit(buffer->getValue()))
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }
        if (buffer->getValue() == '.')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            while(isDigit(buffer->getValue()))
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            if (buffer->getValue() == '"')
            {
                lexem += buffer->getAndClear();
                return token;
            }
        }
    }

    // String
    token = String;
    bool exit = false;
    while (!exit)
    {        
        while (buffer->getValue() != '"' && buffer->getValue() != '\\' && buffer->getValue() != '\n' && buffer->getValue() != 0 )
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->getValue() == '"')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            if (buffer->getValue() == '"')
            {
                lexem += buffer->getAndClear();
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

TokenTyp A2lLexer::hexadecimal()
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

TokenTyp A2lLexer::getPartialString()
{
    TokenTyp token = Identifier;

    char _check = buffer->getValue();
    while (isDigit(_check) || isLetter(_check) || _check == '_')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
        _check = buffer->getValue();
    }

    if (_check == '[')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);        

        _check = buffer->getValue();
        while (isDigit(_check) || isLetter(_check) || _check == '_')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            _check = buffer->getValue();
        }

        if (_check == ']')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }
        else
            token = myUnknown;
    }
    else if (_check == '.') {}
    else if (!isSeparator(_check))
    {
        lexem += buffer->getAndClear();
        token = myUnknown;
        buffer->read(in);
    }

    return token;
}

int A2lLexer::getLine()
{
    return line;
}

void A2lLexer::initialize()
{
    line = 1;
}

void A2lLexer::backward(int i)
{
    int l = lexem.length() + 1;
    in->seek(in->pos() - l);
}

/**********  Class Quex Lexer *************/

A2lQuexLexer::A2lQuexLexer(std::istringstream &in, QObject *parent) : A2lLexer()
{
    grammar = new A2lGrammar();
    keywordsList = grammar->initKeywords();
    index = 0;
    position = 0;
    previousLine = 0;

    //std::istringstream* in = new std::istringstream(strr);
    qlex = new quex::a2l_quex_lexer(&in);
    token_p = qlex->token_p();
}

A2lQuexLexer::~A2lQuexLexer()
{
    delete qlex;
    //delete token_p;
}

TokenTyp A2lQuexLexer::getNextToken()
{
    qlex->receive(&token_p);

    int pos = qlex->tell();
    if ((pos - position) > 40000)
    {
        emit returnedToken(pos - position + getLine() - previousLine);
        position = pos;
        previousLine = getLine();
    }

    //qDebug() << getLine() << " : " << token_p->type_id_name().c_str() << " : " << token_p->pretty_char_text().c_str();
    return myToken(token_p);
}

std::string A2lQuexLexer::getLexem()
{
    return token_p->pretty_char_text();
}

std::string A2lQuexLexer::toString(TokenTyp type)
{
    switch (type)
    {
        case myUnknown:
            return "Unknown";
            break;
        case Identifier:
            return "Identifier";
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
        case Mode:
            return "Mode";
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
        case DataType:
            return "DataType";
            break;
        case Datasize:
            return "Datasize";
            break;
        case Addrtype:
            return "Addrtype";
            break;
        case Byteorder:
            return "Byteorder";
            break;
        case Indexorder:
            return "Indexorder";
            break;
        case IndexMode:
            return "Indexmode";
            break;
        case ConversionType:
            return "ConversionType";
            break;
        case Type:
            return "Type";
            break;
        case Attribute:
            return "Attribute";
            break;
        case PrgType:
            return "PrgType";
            break;
        case MemoryType:
            return "MemoryType";
            break;
        case MemAttribute:
            return "MemAttribute";
            break;
        case UnitType:
            return "UnitType";
            break;

        default:
            return "Unknown";
    }
}

int A2lQuexLexer::getLine()
{
    return token_p->line_number();
}

void A2lQuexLexer::initialize()
{
    line = 1;
}

void A2lQuexLexer::backward(int i)
{
    qlex->seek_backward(i);
}

TokenTyp A2lQuexLexer::myToken(quex::Token *token_p)
{
    TokenTyp token = myUnknown;

    switch (token_p->type_id())
    {
    case  QUEX_TKN_MODE :
        return Mode;
        break;

    case QUEX_TKN_ADDRTYPE :
        return Addrtype;
        break;

    case QUEX_TKN_ATTRIBUTE :
        return Attribute;
        break;

    case QUEX_TKN_BLOCKBEGIN :
        return BlockBegin;
        break;

    case QUEX_TKN_BLOCKEND :
        return BlockEnd;
        break;

    case QUEX_TKN_BYTEORDER :
        return Byteorder;
        break;

    case QUEX_TKN_COMMENT :
        return Comment;
        break;

    case  QUEX_TKN_CONVERSIONTYPE :
        return ConversionType;
        break;

    case  QUEX_TKN_DATASIZE   :
        return Datasize;
        break;

    case QUEX_TKN_DATATYPE :
        return DataType;
        break;

    case  QUEX_TKN_FLOAT :
        return Float;
        break;

    case  QUEX_TKN_HEX :
        return Hex;
        break;

    case  QUEX_TKN_IDENTIFIER :
        return Identifier;
        break;

    case  QUEX_TKN_INDEXMODE :
        return IndexMode;
        break;

    case QUEX_TKN_INDEXORDER  :
        return Indexorder;
        break;

    case QUEX_TKN_INTEGER :
        return Integer;
        break;

    case QUEX_TKN_KEYWORD :
        return Keyword;
        break;

    case QUEX_TKN_MEMATTRIBUTE :
        return MemAttribute;
        break;

    case QUEX_TKN_MEMORYTYPE :
        return MemoryType;
        break;

    case QUEX_TKN_PRGTYPE  :
        return PrgType;
        break;

    case QUEX_TKN_STRING :
        return String;
        break;

    case QUEX_TKN_STRINGFORMAT :
        return StringFormat;
        break;

    case QUEX_TKN_UNITTYPE :
        return UnitType;
        break;

    case QUEX_TKN_TERMINATION :
        return Eof;
        break;

    case  QUEX_TKN_TYPE :
        return Type;
        break;

    }

    return token;
}
