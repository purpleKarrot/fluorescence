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


#ifndef FLUO_UI_PARTICLES_XMLLOADER_HPP
#define FLUO_UI_PARTICLES_XMLLOADER_HPP

#include <boost/shared_ptr.hpp>
#include <misc/pugixml/pugixml.hpp>
#include <ClanLib/Core/Math/vec4.h>

#include <misc/string.hpp>

#include "particleemitterstate.hpp"
#include "baseparticleeffect.hpp"

namespace fluo {

namespace ui {
namespace particles {

class ParticleEmitter;

class XmlLoader {
public:
    static bool fromFile(const UnicodeString& name, const boost::shared_ptr<BaseParticleEffect>& effect);
    static bool fromString(const UnicodeString& str, const boost::shared_ptr<BaseParticleEffect>& effect);

private:
    static XmlLoader* singleton_;
    static XmlLoader* getSingleton();
    XmlLoader();
    XmlLoader(const XmlLoader& l) { }
    XmlLoader& operator=(const XmlLoader& l) { return *this; }

    bool parse(pugi::xml_document& doc, const boost::shared_ptr<BaseParticleEffect>& effect) const;

    boost::shared_ptr<ParticleEmitter> parseEmitter(pugi::xml_node& node, std::map<UnicodeString, ParticleEmitterState>& stateMap) const;
    ParticleEmitterState parseState(pugi::xml_node& node, const ParticleEmitterState& defaultState) const;

    // throws if the attribute is not found
    void checkAttribute(pugi::xml_node& node, const char* name) const;
};

}
}
}

#endif
