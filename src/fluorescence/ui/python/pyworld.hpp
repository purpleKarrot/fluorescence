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


#ifndef FLUO_UI_PYTHON_PYWORLD_HPP
#define FLUO_UI_PYTHON_PYWORLD_HPP

#include <boost/shared_ptr.hpp>
#include <boost/python/list.hpp>

#include <typedefs.hpp>

namespace fluo {

namespace world {
class Mobile;
class DynamicItem;
}

namespace ui {
namespace python {

class PyWorld {
public:
    static boost::shared_ptr<world::Mobile> getPlayer();
    static boost::shared_ptr<world::Mobile> getMobile(Serial serial);
    static boost::shared_ptr<world::DynamicItem> getDynamicItem(Serial serial);

    static void openProfile(const boost::shared_ptr<world::Mobile>& self);
    static void openPaperdoll(const boost::shared_ptr<world::Mobile>& self);

    static boost::python::list getBuffs(const boost::shared_ptr<world::Mobile>& self);
};

}
}
}

#endif

