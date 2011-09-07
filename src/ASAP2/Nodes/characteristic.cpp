#include "Nodes/characteristic.h"
#include <QMessageBox>
#include <typeinfo>
#include <QHash>
#include "a2lgrammar.h"


//initialise static variables
Factory<Node,CHARACTERISTIC> CHARACTERISTIC::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

CHARACTERISTIC::CHARACTERISTIC( Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    // specific for this node Characteristic
    subset = NULL;

    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->characteristic.namePar;
    typePar = &gram->characteristic.typePar;
    factoryOptNode = &gram->characteristic.factoryOptNode;
    factoryOptItem = &gram->characteristic.factoryOptItem;

    //Set the line where the Node starts in ASAP file
    a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"characteristic";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar();

    //fianlize parsing
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "CHARACTERISTIC")
        {
            //Sort the childNodes
            foreach (Node *node, childNodes)
                node->sortChildrensName();
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd CHARACTERISTIC\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end CHARACTERISTIC\nfind : " + s1 + " " + s2);
    }
    this->sortChildrensName();
}

CHARACTERISTIC::~CHARACTERISTIC()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
}

void CHARACTERISTIC::parseFixPar(QList<TokenTyp> *typePar)
{
    //Mandatory PARAMETERS
    TokenTyp token;
    for (int i = 0; i < typePar->count(); i++)
    {
        token = this->nextToken();
        if (token == typePar->at(i))
        {
            char *c = new char[lex->getLexem().length()+1];
            strcpy(c, lex->getLexem().c_str());
            parameters.append(c);
        }
        else if(typePar->at(i) == Float && token == Integer)
        {
            char *c = new char[parentNode->lex->getLexem().length()+1];
            strcpy(c, parentNode->lex->getLexem().c_str());
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

TokenTyp CHARACTERISTIC::parseOptPar()
{
    //opt parameters
    QMap<std::string, Occurence> nameOptPar;
    nameOptPar.insert("FORMAT", ZeroOrOne);
    nameOptPar.insert("IF_DATA", ZeroOrMore);
    nameOptPar.insert("EXTENDED_LIMITS", ZeroOrOne);
    nameOptPar.insert("MAX_REFRESH", ZeroOrOne);
    nameOptPar.insert("NUMBER", ZeroOrOne);
    nameOptPar.insert("READ_ONLY", ZeroOrOne);
    nameOptPar.insert("ANNOTATION", ZeroOrMore);
    nameOptPar.insert("AXIS_DESCR", ZeroOrMore);

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
                    if (nameOptPar.contains(lexem))
                    {
                        if (nameOptPar.value(lexem) == ZeroOrOne)
                        {
                           nameOptPar.insert(lexem, Zero);
                           Node  *instance = factoryOptNode->value(lexem)->createInstance(this);
                           this->addChildNode(instance);
                           token = nextToken();
                        }
                        else if (nameOptPar.value(lexem) == ZeroOrMore)
                        {
                            Node *instance = 0;
                            if (lexem == "AXIS_DESCR")
                            {
                                if (!isChild("AXIS_DESCR"))
                                {
                                    Node *node = new Node(this, lex, errorList);
                                    node->name = (char*)"AXIS_DESCR";
                                    addChildNode(node);
                                    node->_pixmap = "";
                                }
                                instance = factoryOptNode->value(lexem)->createInstance( child("AXIS_DESCR", false));
                                child("AXIS_DESCR", false)->addChildNode(instance);
                            }
                            else
                            {
                                instance = factoryOptNode->value(lexem)->createInstance( this);
                                this->addChildNode(instance);
                            }
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
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else if (nameOptPar.value(lexem) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem)->createInstance( this);
                        this->addOptItem(instance);
                        token = nextToken();
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        this->showError(" Keyword : " + s + " can only be declared once");
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

QMap<std::string, std::string> *CHARACTERISTIC::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
        
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }        

    return par;
}

char* CHARACTERISTIC::getPar(std::string str)
{
    int i = namePar->indexOf(str);    
    return parameters.at(i);
}

std::string CHARACTERISTIC::pixmap()
{
    return ":/icones/CHAR.bmp";
}

// specific only for this node Charactetistic
QString CHARACTERISTIC::getSubsetName()
{
    if (subset)
    {
        return subset->name;
    }
    else
    {
        return "NO_SUBSET";
    }
}

void CHARACTERISTIC::setSubset(FUNCTION *fun)
{
    subset = fun;
}
