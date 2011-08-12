#include "Nodes/calibration_handle.h"
#include <QMessageBox>
#include <typeinfo>
#include "a2lgrammar.h"

//initialise static variables
Factory<Node,CALIBRATION_HANDLE> CALIBRATION_HANDLE::nodeFactory;

// Functions (Predicate)
bool nodeLessThan( const Node *a, const Node *b );
bool itemLessThan( const Item *a, const Item *b );

CALIBRATION_HANDLE::CALIBRATION_HANDLE(QTextStream &in, Node *parentNode)
    : Node(parentNode, parentNode->lex, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = parentNode->lex->grammar;
    namePar = &gram->calibration_handle.namePar;
    typePar = &gram->calibration_handle.typePar;
    factoryOptNode = &gram->calibration_handle.factoryOptNode;
    factoryOptItem = &gram->calibration_handle.factoryOptItem;

    //opt parameters
    occOptPar = new QMap<std::string, Occurence>;
    occOptPar->insert("CALIBRATION_HANDLE_TEXT", ZeroOrOne);

    //Set the line where the Node starts in ASAP file
    this->a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    //parseFixPar(typePar ,in);
    name = (char*)"calibration_handle";


    //Parse optional PARAMETERS (modified only for CALIBRATION_HANDLE)
    TokenTyp token = parseOptPar(occOptPar, in);

    //End
    if (token == BlockEnd)
    {
        token = nextToken(in);
        if (token == Keyword && lex->getLexem() == "CALIBRATION_HANDLE")
        {
            //Sort the childNodes
            //qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);

            //Sort The childItems
            //qSort(this->optItems.begin(), this->optItems.end(), itemLessThan);
        }
        else
        {
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd CALIBRATION_HANDLE\nfind token : " + s );
        }
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end CALIBRATION_HANDLE\nfind : " + s1 + " " + s2 );
    }    
}

CALIBRATION_HANDLE::~CALIBRATION_HANDLE()
{
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }
    delete occOptPar;
}

void CALIBRATION_HANDLE::parseFixPar(QList<TokenTyp> *typePar, QTextStream &in)
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
            this->showError("expected token : " + t +"\nfind token : " + s );
        }
    }
}

TokenTyp CALIBRATION_HANDLE::parseOptPar(QMap<std::string, Occurence> *nameOptPar, QTextStream &in)
{
    // read (Handles)* !!!!! Only for CALIBRATION_HANDLE
    TokenTyp token = lex->getNextToken(in);
    while (token == Hex || token == Integer || token == Comment)
    {
        if (token != Comment)
        {
            // add the lexem to the list of Handle
            char *c = new char[lex->getLexem().length()+1];
            strcpy(c, lex->getLexem().c_str());
            parameters.append(c);
        }

        // get next Token
        token = lex->getNextToken(in);
    }

    // read optional parameters
    if (nameOptPar->isEmpty())
        return nextToken(in);
    else
    {
        //TokenTyp token = nextToken(in);
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = this->nextToken(in);
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
                    this->showError("unknown Keyword : " + s );
                    return token;
                }
            }
        }
        return token;
    }
}

QMap<std::string, std::string> *CALIBRATION_HANDLE::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* CALIBRATION_HANDLE::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}
