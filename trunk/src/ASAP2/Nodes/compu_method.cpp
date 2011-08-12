#include "Nodes/compu_method.h"
#include "lexer.h"
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,COMPU_METHOD> COMPU_METHOD::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

COMPU_METHOD::COMPU_METHOD(QTextStream &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->compu_method.namePar;
    typePar = &gram->compu_method.typePar;
    factoryOptNode = &gram->compu_method.factoryOptNode;
    factoryOptItem = &gram->compu_method.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"compu_method";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar(in);

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken(in);
        if (token == Keyword && lex->getLexem() == "COMPU_METHOD")
        {
            //Sort the childNodes
            //qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd COMPU_METHOD\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end COMPU_METHOD\nfind : " + s1 + " " + s2);
    }
}

COMPU_METHOD::~COMPU_METHOD()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void COMPU_METHOD::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken(in);
        if (token == typePar->at(i))
        {
            //parameters.insert(namePar->at(i), lex->getLexem());
            char *c = new char[lex->getLexem().length()+1];
            strcpy(c, lex->getLexem().c_str());
            parameters.append(c);
        }
        else
        {
            QString t(lex->toString(typePar->at(i)).c_str());
            QString s(lex->toString(token).c_str());
            this->showError("expected token : " + t +"\nfind token : " + s);
        }
    }
}

TokenTyp COMPU_METHOD::parseOptPar(QTextStream &in)
{
    //opt parameters
    QMap<std::string, Occurence> nameOptPar;
    nameOptPar.insert("FORMULA", ZeroOrOne);
    nameOptPar.insert("COEFFS", ZeroOrOne);
    nameOptPar.insert("COEFFS_LINEAR", ZeroOrOne);
    nameOptPar.insert("COMPU_TAB_REF", ZeroOrOne);
    nameOptPar.insert("REF_UNIT", ZeroOrOne);
    nameOptPar.insert("STATUS_STRING_REF", ZeroOrOne);

    if (nameOptPar.isEmpty())
        return nextToken(in);
    else
    {
        TokenTyp token = nextToken(in);
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = this->nextToken(in);
                if (token == Keyword)
                {
                    std::string lexem = lex->getLexem();
                    if (nameOptPar.contains(lexem))
                    {
                        if (nameOptPar.value(lexem) == ZeroOrOne)
                        {
                           nameOptPar.insert(lexem, Zero);
                            Node  *instance = factoryOptNode->value(lexem)->createInstance(in, this);
                            this->addChildNode(instance);
                            token = nextToken(in);
                        }
                        else if (nameOptPar.value(lexem) == ZeroOrMore)
                        {
                            Node  *instance = factoryOptNode->value(lexem)->createInstance(in, this);
                            this->addChildNode(instance);
                            token = nextToken(in);
                        }
                        else
                        {
                            QString s(lexem.c_str());
                            this->showError(" Keyword : " + s + " can only be once declared");
                            return token;
                        }
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        this->showError("unknown Keyword : " + s );
                        return token;
                    }
                }
                else
                {
                    QString s(lex->toString(token).c_str());
                    this->showError("expected token : BlockBegin or Keyword\nfind token : " + s );
                    return token;
                }
            }
            //Items
            else if (token == Keyword)
            {
                std::string lexem = lex->getLexem();
                if (nameOptPar.contains(lexem))
                {
                    if (nameOptPar.value(lexem) == ZeroOrOne)
                    {
                        nameOptPar.insert(lexem, Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance(in, this);
                        this->addOptItem(instance);
                        token = nextToken(in);
                    }
                    else if (nameOptPar.value(lexem) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem)->createInstance(in, this);
                        this->addOptItem(instance);
                        token = nextToken(in);
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        this->showError(" Keyword : " + s + " can only be once declared");
                        return token;
                    }
                }
                else
                {
                    QString s(lexem.c_str());
                    this->showError("unknown Keyword : " + s );
                    return token;
                }
            }
        }
        return token;
    }
}

QMap<std::string, std::string> *COMPU_METHOD::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string COMPU_METHOD::pixmap()
{
    return ":/icones/CHAR.bmp";
}

char* COMPU_METHOD::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    if (i > 0)
        return parameters.at(i);
    else
        return NULL;
}
