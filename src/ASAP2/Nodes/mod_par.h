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

#ifndef MOD_PAR_H
#define MOD_PAR_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/memory_segment.h"
#include "Nodes/memory_layout.h"
#include "Nodes/calibration_method.h"
#include "Items/version.h"
#include "Items/addr_epk.h"
#include "Items/epk.h"
#include "Items/customer_no.h"
#include "Items/user.h"
#include "Items/phone_no.h"
#include "Items/ecu.h"
#include "Items/cpu_type.h"
#include "Items/system_constant.h"

class MOD_PAR : public Node
{
    public:
        MOD_PAR( Node *parentNode);
        ~MOD_PAR();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, MOD_PAR> nodeFactory;
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

       // #pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar);
};

#endif // MOD_PAR_H
