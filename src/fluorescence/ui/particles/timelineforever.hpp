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


#ifndef FLUO_UI_PARTICLES_TIMELINEFOREVER_HPP
#define FLUO_UI_PARTICLES_TIMELINEFOREVER_HPP

#include "timelineelement.hpp"
#include "particleemitterstate.hpp"

namespace fluo {
namespace ui {
namespace particles {
    
class TimelineForever : public TimelineElement {
public:
    TimelineForever(const ParticleEmitterState& state);
    
    virtual void activate();
    virtual float step(float elapsedSeconds);
    
    virtual float getMaxParticleLifetime() const;
    
    virtual float numberOfNewParticles(float elapsedSeconds) const;
    virtual void initParticle(Particle& particle, const CL_Vec3f& emitterLocation, float emitterAge) const;
    
    virtual CL_Vec3f getEmitterLocationOffset() const;
    
private:
    ParticleEmitterState state_;
};

}
}
}

#endif

