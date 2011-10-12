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

#ifndef FUNCTION_H
#define FUNCTION_H

#include "node.h"
#include "basefactory.h"
#include "Nodes/def_characteristic.h"
#include "Nodes/ref_characteristic.h"
#include "Nodes/loc_measurement.h"
#include "Nodes/in_measurement.h"
#include "Nodes/out_measurement.h"
#include "Nodes/sub_function.h"
#include "Items/function_version.h"

class FUNCTION : public Node
{
    public:
        FUNCTION( Node *parentNode);
        ~FUNCTION();
        static Factory<Node, FUNCTION> nodeFactory;

        QMap<std::string, std::string> *getParameters();
        std::string pixmap();
        char* getPar(std::string str);

    private:
        // Fix parameters
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;

        // Opt parameters
        QMap<std::string, FactoryPlant<Node> *>  *factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  *factoryOptItem;

        //#pragma omp threadprivate(typePar, namePar, factoryOptNode, factoryOptItem)

        void parseFixPar(QList<TokenTyp> *typePar);
        TokenTyp parseOptPar();
};

#endif // FUNCTION_H
