#include "Nodes/mod_par.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,MOD_PAR> MOD_PAR::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

MOD_PAR::MOD_PAR(QTextStream &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->mod_par.namePar;
    typePar = &gram->mod_par.typePar;
    factoryOptNode = &gram->mod_par.factoryOptNode;
    factoryOptItem = &gram->mod_par.factoryOptItem;

    //opt parameters
    occOptPar = new QMap<std::string, Occurence>;
    occOptPar->insert("VERSION", ZeroOrOne);
    occOptPar->insert("EPK", ZeroOrOne);
    occOptPar->insert("CUSTOMER_NO", ZeroOrOne);
    occOptPar->insert("USER", ZeroOrOne);
    occOptPar->insert("PHONE_NO", ZeroOrOne);
    occOptPar->insert("ECU", ZeroOrOne);
    occOptPar->insert("CPU_TYPE", ZeroOrOne);
    occOptPar->insert("ADDR_EPK", ZeroOrMore);
    occOptPar->insert("MEMORY_SEGMENT", ZeroOrMore);
    occOptPar->insert("MEMORY_LAYOUT", ZeroOrMore);
    occOptPar->insert("CALIBRATION_METHOD", ZeroOrMore);
    occOptPar->insert("SYSTEM_CONSTANT", ZeroOrMore);

    //Set the line where the Node starts in ASAP file
    this->a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar ,in);

    name = (char*)"MOD_PAR";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar(occOptPar, in);

    //End
    if (token == BlockEnd)
    {
        token = nextToken(in);
        if (token == Keyword && lex->getLexem() == "MOD_PAR")
        {
            //Sort the childNodes
            //qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd MOD_PAR\nfind token : " + s);
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end MOD_PAR\nfind : " + s1 + " " + s2);
    }    
}

MOD_PAR::~MOD_PAR()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
    delete occOptPar;
}

void MOD_PAR::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
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

TokenTyp MOD_PAR::parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in)
{

    if (nameOptPar->isEmpty())
        return nextToken(in);
    else
    {
        TokenTyp token = nextToken(in);
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = nextToken(in);
                if (token == Keyword)
                {
                    std::string lexem = lex->getLexem();
                    if (factoryOptNode->contains(lexem))
                    {
                        if (this->occOptPar->value(lexem) == ZeroOrOne)
                        {
                            this->occOptPar->insert(lexem, Zero);
                            Node  *instance = factoryOptNode->value(lexem)->createInstance(in, this);
                            this->addChildNode(instance);
                            token = nextToken(in);
                        }
                        else if (this->occOptPar->value(lexem) == ZeroOrMore)
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
                        this->showError("Keyword  " + s + " not in MOD_PAR grammar");
                        return token;
                    }
                }
                else
                {
                    QString s(lex->getLexem().c_str());
                    this->showError("unknown Keyword : " + s);
                    return token;
                }
            }
            //Items
            else if (token == Keyword)
            {
                std::string lexem = lex->getLexem();
                if (factoryOptItem->contains(lexem))
                {
                    if (this->occOptPar->value(lexem) == ZeroOrOne)
                    {
                        this->occOptPar->insert(lexem, Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance(in, this);
                        this->addOptItem(instance);
                        token = nextToken(in);
                    }
                    else if (this->occOptPar->value(lexem) == ZeroOrMore)
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
                    this->showError("unknown Keyword : " + s);
                    return token;
                }
            }
        }
        return token;
    }
}

QMap<std::string, std::string> *MOD_PAR::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* MOD_PAR::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
