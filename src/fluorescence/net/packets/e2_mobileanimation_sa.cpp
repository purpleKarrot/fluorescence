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



#include "e2_mobileanimation_sa.hpp"

#include <world/mobile.hpp>
#include <world/manager.hpp>

#include <misc/log.hpp>

namespace fluo {
namespace net {
namespace packets {

MobileAnimationSA::MobileAnimationSA() : Packet(0xE2, 10) {
}

bool MobileAnimationSA::read(const int8_t* buf, unsigned int len, unsigned int& index) {
    bool ret = true;

    ret = ret && PacketReader::read(buf, len, index, serial_);
    ret = ret && PacketReader::read(buf, len, index, actionId_);
    ret = ret && PacketReader::read(buf, len, index, frameCount_);
    ret = ret && PacketReader::read(buf, len, index, delay_);

    return ret;
}

void MobileAnimationSA::onReceive() {
    LOG_WARN << "Mobile animation (SA) action=" << actionId_ << " frameCount=" << frameCount_ << " delay=" << (unsigned int)delay_ << std::endl;

    // TODO: implement frameCount
    boost::shared_ptr<world::Mobile> mob = world::Manager::getSingleton()->getMobile(serial_, false);

    if (mob) {
        mob->animate(actionId_, delay_, AnimRepeatMode::DEFAULT);
    }
}

}
}
}
