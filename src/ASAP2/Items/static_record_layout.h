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

#ifndef STATIC_RECORD_LAYOUT_H
#define STATIC_RECORD_LAYOUT_H

#include "item.h"
#include "lexer.h"
#include "basefactory.h"

class STATIC_RECORD_LAYOUT : public Item
{
    public:
        STATIC_RECORD_LAYOUT( Node *parentNode);
        ~STATIC_RECORD_LAYOUT();
        QMap<std::string, std::string> getParameters();
        static Factory<Item, STATIC_RECORD_LAYOUT> itemFactory;
        char* getPar(std::string str);

    private:
        void parseFixPar(QList<TokenTyp> *typePar);
        QList<TokenTyp> *typePar;
        QList<std::string> *namePar;
        QList<char*> parameters;
};

#endif // STATIC_RECORD_LAYOUT_H
