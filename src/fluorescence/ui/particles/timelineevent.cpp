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
 
#include "timelineevent.hpp"

#include <misc/log.hpp>

namespace fluo {
namespace ui {
namespace particles {
    
TimelineEvent::TimelineEvent(const UnicodeString& event) : event_(event) {
}
    
void TimelineEvent::activate() {
    LOG_DEBUG << "timeline event activated, event=" << event_ << std::endl;
}

float TimelineEvent::step(float elapsedSeconds) {
    return elapsedSeconds;
}

float TimelineEvent::numberOfNewParticles(float elapsedSeconds) const {
    return 0;
}

void TimelineEvent::initParticle(Particle& particle, const CL_Vec3f& emitterLocation, float emitterAge) const {
    // should never be called
    LOG_ERROR << "initParticle called on TimelineEvent" << std::endl;
}

CL_Vec3f TimelineEvent::getEmitterLocationOffset() const {
    return CL_Vec3f(0, 0, 0);
}

float TimelineEvent::getMaxParticleLifetime() const {
    return 0;
}

bool TimelineEvent::consumesEvent(const UnicodeString& ev) const {
    return event_ == ev;
}

}
}
}
