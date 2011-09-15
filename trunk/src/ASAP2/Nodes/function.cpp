#include "Nodes/function.h"
#include "lexer.h"
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,FUNCTION> FUNCTION::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

FUNCTION::FUNCTION( Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->function.namePar;
    typePar = &gram->function.typePar;
    factoryOptNode = &gram->function.factoryOptNode;
    factoryOptItem = &gram->function.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"function";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar();

    //finalize parsing
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "FUNCTION")
        {          
            //Sort the childNodes
            foreach (Node *node, childNodes)
                node->sortChildrensName();

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd FUNCTION\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end FUNCTION\nfind : " + s1 + " " + s2);
    }

    sortChildrensName();
}

FUNCTION::~FUNCTION()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void FUNCTION::parseFixPar(QList<TokenTyp> *typePar)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken();
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

TokenTyp FUNCTION::parseOptPar()
{
    //opt parameters
    QHash<QString, Occurence> nameOptPar;
    nameOptPar.insert("DEF_CHARACTERISTIC", ZeroOrOne);
    nameOptPar.insert("REF_CHARACTERISTIC", ZeroOrOne);
    nameOptPar.insert("LOC_MEASUREMENT", ZeroOrOne);
    nameOptPar.insert("FUNCTION_VERSION", ZeroOrOne);
    nameOptPar.insert("IN_MEASUREMENT", ZeroOrOne);
    nameOptPar.insert("OUT_MEASUREMENT", ZeroOrOne);
    nameOptPar.insert("SUB_FUNCTION", ZeroOrOne);

    if (nameOptPar.isEmpty())
        return nextToken();
    else
    {
        TokenTyp token = nextToken();
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = this->nextToken();
                if (token == Keyword)
                {
                    std::string lexem = lex->getLexem();
                    if (nameOptPar.contains(lexem.c_str()))
                    {
                        if (nameOptPar.value(lexem.c_str()) == ZeroOrOne)
                        {
                           nameOptPar.insert(lexem.c_str(), Zero);
                           Node  *instance = factoryOptNode->value(lexem)->createInstance( this);
                           this->addChildNode(instance);
                           token = nextToken();
                        }
                        else if (nameOptPar.value(lexem.c_str()) == ZeroOrMore)
                        {
                            Node  *instance = factoryOptNode->value(lexem)->createInstance( this);
                            this->addChildNode(instance);
                            token = nextToken();
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
                        this->showError(s + " is not a keyword accepted in FUNCTION block ");
                        return token;
                    }
                }
                else
                {
                    QString s2(lex->getLexem().c_str());
                    this->showError("unknown keyword in grammar " + s2 );
                    return token;
                }
            }
            //Items
            else if (token == Keyword)
            {
                std::string lexem = lex->getLexem();
                if (nameOptPar.contains(lexem.c_str()))
                {
                    if (nameOptPar.value(lexem.c_str()) == ZeroOrOne)
                    {
                        nameOptPar.insert(lexem.c_str(), Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else if (nameOptPar.value(lexem.c_str()) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
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

QMap<std::string, std::string> *FUNCTION::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

std::string FUNCTION::pixmap()
{
    return ":/icones/CHAR.bmp";
}

char* FUNCTION::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
