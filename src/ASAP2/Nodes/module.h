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

#ifndef MODULE_H
#define MODULE_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Nodes/axis_pts.h"
#include "Nodes/a2ml.h"
#include "Nodes/measurement.h"
#include "Nodes/mod_par.h"
#include "Nodes/if_data.h"
#include "Nodes/function.h"
#include "Nodes/mod_common.h"
#include "Nodes/compu_method.h"
#include "Nodes/compu_tab.h"
#include "Nodes/compu_vtab.h"
#include "Nodes/compu_vtab_range.h"
#include "Nodes/record_layout.h"
#include "Nodes/characteristic.h"
#include "Nodes/group.h"
#include "Nodes/unit.h"

class MODULE : public Node
{
    public:
        MODULE( Node *parentNode);
        ~MODULE();
        QMap<std::string, std::string> *getParameters();
        static Factory<Node, MODULE> nodeFactory;
        char* getPar(std::string str);
        QStringList listChar;
        QStringList listMeas;

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, Occurence> *occOptPar;
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar(QMap<std::string, Occurence> *nameOptPar);
};

#endif // MODULE_H
