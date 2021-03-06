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



#include "34_statskillquery.hpp"

namespace fluo {
namespace net {
namespace packets {

StatSkillQuery::StatSkillQuery(Serial serial, uint8_t queryType) : Packet(0x34, 10), serial_(serial), queryType_(queryType) {
}

bool StatSkillQuery::write(int8_t* buf, unsigned int len, unsigned int& index) const {
    bool ret = true;

    ret &= writePacketId(buf, len, index);
    ret &= PacketWriter::write(buf, len, index, 0xEDEDEDEDU); // unknown
    ret &= PacketWriter::write(buf, len, index, queryType_);
    ret &= PacketWriter::write(buf, len, index, serial_);

    return ret;
}

}
}
}
