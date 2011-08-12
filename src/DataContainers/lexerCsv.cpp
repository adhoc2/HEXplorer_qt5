#include "lexerCsv.h"
#include <sstream>
#include "qdebug.h"

/**********  Class Lexer *************/
LexerCsv::LexerCsv(QObject *parent) : QObject(parent)
{
    // --- define Keywords
    QList<std::string> list;
    list << "FUNCTION_HDR" << "VARIANT_HDR" << "VALUE" << "CURVE" << "MAP" << "X_AXIS_PTS"
            << "Y_AXIS_PTS" << "FUNCTION" << "AXIS_PTS" << "VAL_BLK" << "ASCII";

    foreach (std::string str, list)
    {
        KeywordsMap.insert(QString(str.c_str()), Keyword);
    }
    list.clear();


    // --- initialize members
    buffer = new Buffer();
    valueSeparator = 0;
    commentIndicator = 0;
    stringDelimiter = 0;
    decimalPointSeparator = '.';
    position = 0;
}

LexerCsv::~LexerCsv()
{
    delete buffer;
}

std::string LexerCsv::getLexem()
{
    return lexem;
}

std::string LexerCsv::toString(TokenTyp type)
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

TokenTyp LexerCsv::getNextToken(QTextStream &in)
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

TokenTyp LexerCsv::begin(QTextStream &in, char ch)
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
            if (isValueSeparator(ch))
            {
               token = indentation(in, ch);
               if (token == Indentation)
               {
                   return token;
               }
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

TokenTyp LexerCsv::indentation(QTextStream &in, char &ch)
{
    lexem = "";
    lexem += ch;

    // check the next character
    buffer->read(in);
    if (isValueSeparator(buffer->check()))
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

TokenTyp LexerCsv::commentM(QTextStream &in)
{
    TokenTyp token;

    token = Comment;
    lexem = "/*";

    bool exit = false;
    while (!exit)
    {
        buffer->read(in);
        if (buffer->check() == '*')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            while (buffer->check() == '*')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }

            if (buffer->check() == '/')
            {
                lexem += buffer->getAndClear();
                exit = true;
            }
            else if (buffer->check() == 0)
                exit = true;
            else
            {
                if (buffer->check() == '\n')
                    this->incLine();
                lexem +=  buffer->getAndClear();
            }

        }
        else if (buffer->check() == 0)
        {
            exit = true;
        }
        else
        {
            if (buffer->check() == '\n')
                this->incLine();
            lexem +=  buffer->getAndClear();
        }

    }
    return token;
}

TokenTyp LexerCsv::commentL(QTextStream &in)
{
    TokenTyp token;

    token = Comment;
    lexem = "*";

    buffer->read(in);
    while (buffer->check() != '\n' && buffer->check() != '\r' && buffer->check() != 0)
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
    }
    return token;
}

TokenTyp LexerCsv::block(QTextStream &in, char &ch)
{
    TokenTyp token;
    lexem += '/';
    lexem += ch;

    buffer->read(in);
    while (isLetter(buffer->check()) || isDigit(buffer->check()) || buffer->check() == '_')
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

TokenTyp LexerCsv::identifier(QTextStream &in, char &ch)
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
            if (buffer->check() == '.')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            else if (isNewLine(buffer->check()))
            {
                exit = true;
            }
            else if (isValueSeparator(buffer->check()))
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

TokenTyp LexerCsv::number(QTextStream &in, char &ch)
{
    TokenTyp token;
    lexem = "";
    lexem += ch;

    buffer->read(in);
    while(isDigit(buffer->check()))
    {        
        lexem += buffer->getAndClear();
        buffer->read(in);
    }

    if (buffer->check() == decimalPointSeparator)
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
        while(isDigit(buffer->check()))
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->check() == 'E' || buffer->check() == 'e')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            if (buffer->check() == '+' || buffer->check() == '-')
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
                while(isDigit(buffer->check()))
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
    else if(buffer->check() == 'E' || buffer->check() == 'e')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
        if (buffer->check() == '+' || buffer->check() == '-')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            while(isDigit(buffer->check()))
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
    else if (buffer->check() == 'x')
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

TokenTyp LexerCsv::string(QTextStream &in)
{
    TokenTyp token;
    lexem = stringDelimiter;

    buffer->read(in);

    token = String;
    bool exit = false;
    while (!exit)
    {        
        while (buffer->check() != stringDelimiter && buffer->check() != '\\' && buffer->check() != '\n' && buffer->check() != 0 )
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->check() == stringDelimiter)
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
            if (buffer->check() == stringDelimiter)
            {
                lexem += buffer->getAndClear();
                buffer->read(in);
            }
            else
                exit = true;

        }
        else if (buffer->check() == '\\')
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

TokenTyp LexerCsv::hexadecimal(QTextStream &in)
{
    TokenTyp token;
    token = Hex;
    lexem = "0x";

    buffer->read(in);
    while (isHexDigit(buffer->check()))
    {
        lexem += buffer->getAndClear();
        buffer->read(in);
    }
    return token;
}

TokenTyp LexerCsv::getPartialString(QTextStream &in)
{
    TokenTyp token = Identifier;

    while (isDigit(buffer->check()) || isLetter(buffer->check()) || buffer->check() == '_')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);        
    }

    if (buffer->check() == '[')
    {
        lexem += buffer->getAndClear();
        buffer->read(in);        

        while (isDigit(buffer->check()) || isLetter(buffer->check()) || buffer->check() == '_')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }

        if (buffer->check() == ']')
        {
            lexem += buffer->getAndClear();
            buffer->read(in);
        }
        else
            token = myUnknown;
    }
    else if (buffer->check() == '.')
    {
    }
    else if (isValueSeparator(buffer->check()))
    {
    }
    else if (!isNewLine(buffer->check()))
    {
        lexem += buffer->getAndClear();
        token = myUnknown;
        buffer->read(in);
    }

    return token;
}

void LexerCsv::incLine()
{
    nTotalLines++;
}

int LexerCsv::getLine()
{
    return this->nTotalLines;
}

void LexerCsv::initialize()
{
    nTotalLines = 0;
}

void LexerCsv::backward(QTextStream &in)
{
    int l = lexem.length() + 1;
    in.seek(in.pos() - l);
}

bool LexerCsv::isNewLine(char ch)
{
    if (ch == '\r' || ch == '\n')
        return true;
    else
        return false;
}

bool LexerCsv::isValueSeparator(char ch)
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

bool LexerCsv::isDigit(char ch)
{
    if ((ch <= '9') && (ch >= '0'))
        return true;
    else
        return false;
}

bool LexerCsv::isHexDigit(char ch)
{
    if ((('0' <= ch) && (ch <= '9')) || (('A' <= ch) && (ch <= 'F'))  || (('a' <= ch) && (ch <= 'f')))
        return true;
    else
        return false;
}

bool LexerCsv::isLetter(char ch)
{
    if ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A'))
        return true;
    else
        return false;
}

bool LexerCsv::isA2mlSym(char ch)
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
