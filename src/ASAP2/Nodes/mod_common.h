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

#ifndef MOD_COMMON_H
#define MOD_COMMON_H

#include "node.h"
#include "basefactory.h"
#include "Items/byte_order.h"
#include "Items/alignment_byte.h"
#include "Items/alignment_float32_ieee.h"
#include "Items/alignment_word.h"
#include "Items/alignment_long.h"
#include "Items/s_rec_layout.h"


class MOD_COMMON : public Node
{
    public:
        MOD_COMMON( Node *parentNode);
        ~MOD_COMMON();
        QMap<std::string, std::string> *getParameters();
        char* fixPar(int n);
        static Factory<Node,MOD_COMMON> nodeFactory;
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

#endif // MOD_COMMON_H
