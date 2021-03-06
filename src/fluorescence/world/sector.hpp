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


#ifndef FLUO_WORLD_SECTOR_HPP
#define FLUO_WORLD_SECTOR_HPP

#include <boost/shared_ptr.hpp>

#include <typedefs.hpp>
#include "map.hpp"
#include "statics.hpp"

namespace fluo {

namespace data {
    class SectorReader;
}

namespace world {

class MiniMapBlock;

class Sector {

public:
    Sector(unsigned int mapId, const IsoIndex& sectorId, bool fullLoad);
    ~Sector();

    const IsoIndex& getSectorId() const;
    unsigned int getMapId() const;

    unsigned int getLocX() const;
    unsigned int getLocY() const;

    bool isVisible() const;

    void update(unsigned int elapsedMillis);

    std::list<world::IngameObject*>::iterator renderBegin();
    std::list<world::IngameObject*>::iterator renderEnd();

    bool repaintRequired() const;

    void addDynamicObject(world::IngameObject* obj);
    void removeDynamicObject(world::IngameObject* obj);
    void requestSort();

    boost::shared_ptr<world::IngameObject> getFirstObjectAt(int worldX, int worldY, bool getTopObject) const;

    // height that we can reach with one step from the given location
    int getStepReach(const CL_Vec3f& loc) const;
    bool checkMovement(const CL_Vec3f& curLocation, int stepReach, CL_Vec3f& outLoc) const;

    int getMapZAt(unsigned int worldX, unsigned int worldY) const;

    void getWalkObjectsOn(unsigned int x, unsigned int y, std::list<world::IngameObject*>& list) const;

    void invalidateAllTextures();

    boost::shared_ptr<world::MiniMapBlock> getMiniMapBlock() const;


    const uint32_t* getStaticMiniMapPixels() const;
    const int8_t* getStaticMiniMapHeight() const;
    const uint32_t* getMapMiniMapPixels() const;
    const int8_t * getMapMiniMapHeight() const;

    bool requireFullLoad() const;
    void setRequireFullLoad(bool value);

private:
    unsigned int mapId_;
    IsoIndex id_;

    boost::shared_ptr<MapBlock> mapBlock_;
    bool mapAddedToList_;

    boost::shared_ptr<StaticBlock> staticBlock_;
    bool staticsAddedToList_;

    bool visible_;

    bool fullUpdateRenderDataRequired_;
    bool renderListSortRequired_;
    bool repaintRequired_;

    // ownership of these objects is already provided by staticBlock_ or mapBlock_, thus no smart pointers here
    std::list<world::IngameObject*> quickRenderUpdateList_;
    std::list<world::IngameObject*> renderList_;

    static bool renderDepthSortHelper(const world::IngameObject* a, const world::IngameObject* b);

    bool checkFreeSpace(const std::list<world::IngameObject*>& list, int zFrom, int zTo) const;

    boost::shared_ptr<MiniMapBlock> miniMapBlock_;

    // if false, this sector is only required for the minimap (no need to update it)
    bool requireFullLoad_;
};

}
}

#endif
