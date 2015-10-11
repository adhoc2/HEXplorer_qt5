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

#ifndef CHARACTERISTIC_H
#define CHARACTERISTIC_H

#include "node.h"
#include "item.h"
#include "lexer.h"
#include "basefactory.h"
#include "Items/format.h"
#include "Items/extended_limits.h"
#include "Items/max_refresh.h"
#include "Nodes/if_data.h"
#include "Nodes/annotation.h"
#include "Nodes/axis_descr.h"
#include "Nodes/function.h"
#include "Nodes/function_list.h"
#include "Items/number.h"
#include "Items/read_only.h"
#include "Items/display_identifier.h"
#include "Items/ecu_address_extension.h"
#include "Items/matrix_dim.h"

class CHARACTERISTIC : public Node
{
    public:
        CHARACTERISTIC( Node *parentNode);
        ~CHARACTERISTIC();

        QMap<std::string, std::string> *getParameters();
        static Factory<Node,CHARACTERISTIC> nodeFactory;
        std::string pixmap();
        char* getPar(std::string str);
        char* getPar(int i);
        // specific only for this node Characteristic
        QString getSubsetName();
        void setSubset(FUNCTION *fun);

    private:
        // specific only for this node Characteristic
        FUNCTION *subset;

        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
        void parseFixPar(QList<TokenTyp> *typePar);

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;        
        TokenTyp parseOptPar();
};

#endif // CHARACTERISTIC_H
