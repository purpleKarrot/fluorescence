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


#ifndef FLUO_NET_PACKETS_PLAYSOUND_HPP
#define FLUO_NET_PACKETS_PLAYSOUND_HPP

#include <net/packet.hpp>

namespace fluo {
namespace net {

namespace packets {

// needs to have "Packet" in the name as well to enable VS to distinguish it from the windows PlaySound function
class PlaySoundPacket : public Packet {
public:
    PlaySoundPacket();

    virtual bool read(const int8_t* buf, unsigned int len, unsigned int& index);

    virtual void onReceive();

private:
    // playMode_ is not used yet
    uint8_t playMode_;
    
    uint16_t soundId_;
    
    // location is not used yet
    uint16_t locX_;
    uint16_t locY_;
    int16_t locZ_;
};

}
}
}

#endif
