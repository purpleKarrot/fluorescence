/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FLUO_DATA_MOBTYPESLOADER_HPP
#define FLUO_DATA_MOBTYPESLOADER_HPP

#include "defstructs.hpp"

namespace fluo {
namespace data {

class MobTypesLoader {
public:
    enum {
        TYPE_HUMAN = 0,
        TYPE_MONSTER = 1,
        TYPE_ANIMAL = 2,
        TYPE_EQUIPMENT = 3,
        TYPE_SEA_MONSTER = 4,
    };
    
    static void parseType(MobTypeDef& def, unsigned int strIndex, const char* str, int*& ptr);
};
}
}


#endif
