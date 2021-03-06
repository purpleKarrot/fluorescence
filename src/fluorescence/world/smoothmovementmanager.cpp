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



#include "smoothmovementmanager.hpp"

#include <boost/bind.hpp>

namespace fluo {
namespace world {

void SmoothMovementManager::update(unsigned int elapsedMillis) {
    std::map<Serial, std::list<SmoothMovement> >::iterator iter = movementQueues_.begin();
    std::map<Serial, std::list<SmoothMovement> >::iterator end = movementQueues_.end();

    while (iter != end) {
        if (iter->second.empty()) {
            movementQueues_.erase(iter++);
            continue;
        }

        unsigned int millisLeft = elapsedMillis;
        while (millisLeft > 0 && !iter->second.empty()) {
            SmoothMovement* curMov = &iter->second.front();

            if (!curMov->wasStarted()) {
                curMov->start();
            }

            millisLeft = curMov->update(millisLeft);

            if (curMov->isFinished()) {
                curMov->finish(false);
                iter->second.pop_front();
            }
        }

        ++iter;
    }
}

void SmoothMovementManager::add(Serial serial, SmoothMovement& movement) {
    movementQueues_[serial].push_back(movement);
}

void SmoothMovementManager::clear(Serial serial) {
    std::map<Serial, std::list<SmoothMovement> >::iterator it = movementQueues_.find(serial);
    if (it != movementQueues_.end()) {
        if (!it->second.empty()) {
            it->second.front().finish(true);
        }

        movementQueues_.erase(it);
    }
}

void SmoothMovementManager::clear() {
    movementQueues_.clear();
}

CL_Vec3f SmoothMovementManager::getTargetLoc(Serial serial) const {
    std::map<Serial, std::list<SmoothMovement> >::const_iterator it = movementQueues_.find(serial);
    if (it != movementQueues_.end()) {
        return it->second.back().getTargetLoc();
    }

    return CL_Vec3f(0, 0, 0);
}

}
}
