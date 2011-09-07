#include "Nodes/project.h"
#include <QMessageBox>
#include <sstream>
#include "a2lgrammar.h"

using namespace std;

PROJECT::PROJECT(Node *parentNode, A2lLexer *lexer)
    : Node(parentNode, lexer, parentNode->errorList)
{
    //get grammar
    A2lGrammar* gram = lexer->grammar;
    namePar = &gram->project.namePar;
    typePar = &gram->project.typePar;
    factoryOptNode = &gram->project.factoryOptNode;
    factoryOptItem = &gram->project.factoryOptItem;

    //optional parameters
    occOptPar = new QMap<std::string, Occurence>;
    occOptPar->insert("HEADER", ZeroOrOne);
    occOptPar->insert("MODULE", ZeroOrMore);    

    //Set the line where the Node starts in ASAP file
    this->a2lLine = lex->getLine();

    //Parse Mandatory PARAMETERS
    parseFixPar(typePar);
    if (parameters.count() > 0)
        name = parameters.at(0);
    else
        name = (char*)"project";

    //Parse optional PARAMETERS
    TokenTyp token = parseOptPar(occOptPar);

    //End
    if (token == BlockEnd)
    {
        token = nextToken();
        if (token == Keyword && lex->getLexem() == "PROJECT")
        {

        }
        else
        {			
            QString s(lex->toString(token).c_str());
            this->showError("expected token : BlockEnd PROJECT\nfind token : " + s);
        }
    }
    else if (token == Identifier && lex->getLexem() == "CHUNKend")
    {
        qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);
        stopped = true;
        lexer->in->seek(lexer->in->pos() - 8);
    }
    else
    {
        QString s1(lex->toString(token).c_str());
        QString s2(lex->getLexem().c_str());
        this->showError("expected end PROJECT\nfind : " + s1 + " " + s2);
    }

    qSort(this->childNodes.begin(), this->childNodes.end(), nodeLessThan);
}

PROJECT::~PROJECT()
{    
   //qDeleteAll(parameters);
    foreach (char* ptr, parameters)
    {
        delete[] ptr;
    }

   delete occOptPar;
}

void PROJECT::parseFixPar(QList<TokenTyp> *typePar)
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

TokenTyp PROJECT::parseOptPar(QMap<std::string, Occurence> *nameOptPar)
{

    if (nameOptPar->isEmpty())
        return nextToken();
    else
    {
        TokenTyp token = nextToken();
        while (token == BlockBegin || token == Keyword)
        {
            //Nodes
            if (token == BlockBegin)
            {
                token = nextToken();
                if (token == Keyword)
                {
                    std::string lexem = lex->getLexem();
                    if (factoryOptNode->contains(lexem))
                    {
                        if (occOptPar->value(lexem) == ZeroOrOne)
                        {
                            occOptPar->insert(lexem, Zero);
                            Node  *instance = factoryOptNode->value(lexem)->createInstance(this);
                            addChildNode(instance);
                            token = nextToken();
                        }
                        else if (occOptPar->value(lexem) == ZeroOrMore)
                        {
                            Node *instance;
                            if (lexem == "MODULE")
                            {
                                if (!isChild("MODULE"))
                                {
                                    Node *Char = new Node(this, lex, errorList);
                                    Char->name = (char*)"MODULE";
                                    addChildNode(Char);
                                    Char->_pixmap = "";
                                }
                                instance = factoryOptNode->value(lexem)->createInstance(child("MODULE", false));
                                child("MODULE", false)->addChildNode(instance);
                            }
                            token = nextToken();
                        }
                        else
                        {
                            QString s(lexem.c_str());
                            showError(" Keyword : " + s + " can only be once declared");
                            return token;
                        }
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        showError("unknown Keyword : " + s);
                        return token;
                    }
                }
                else
                {
                    QString s(lex->toString(token).c_str());
                    showError("expected token : BlockBegin or Keyword\nfind token : " + s);
                    return token;
                }
            }
            //Items
            else if (token == Keyword)
            {
                std::string lexem = lex->getLexem();
                if (factoryOptItem->contains(lexem))
                {
                    if (occOptPar->value(lexem) == ZeroOrOne)
                    {
                        occOptPar->insert(lexem, Zero);
                        Item  *instance = factoryOptItem->value(lexem)->createInstance(this);
                        addOptItem(instance);
                        token = nextToken();
                    }
                    else if (occOptPar->value(lexem) == ZeroOrMore)
                    {
                        Item  *instance = factoryOptItem->value(lexem)->createInstance(this);
                        addOptItem(instance);
                        token = nextToken();
                    }
                    else
                    {
                        QString s(lexem.c_str());
                        showError(" Keyword : " + s + " can only be once declared");
                        return token;
                    }
                }
                else
                {
                    QString s(lexem.c_str());
                    showError("unknown Keyword : " + s);
                    return token;
                }
            }
        }
        return token;
    }
}

std::string PROJECT::pixmap()
{
    return ":/icones/MacHD.png";
}

QMap<std::string, std::string> *PROJECT::getParameters()
{
    QMap<std::string, std::string> *par = new QMap<std::string, std::string>;
    for (int i = 0; i < namePar->count(); i++)
    {
        par->insert(namePar->at(i), parameters.at(i));
    }
    return par;
}

char* PROJECT::getPar(std::string str)
{
    int i = namePar->indexOf(str);
    return parameters.at(i);
}

QList<MODULE*> PROJECT::listModule()
{
    QList<MODULE*> list;
    Node *module = getNode("MODULE");
    if (module)
    {
        foreach(Node *module, module->childNodes)
        {
            list.append((MODULE*)module);
        }
    }
    return list;
}
