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



#include "maploader.hpp"
#include "fullfileloader.hpp"

#include <world/map.hpp>

#include <boost/bind.hpp>

namespace fluo {
namespace data {

MapLoader::MapLoader(const boost::filesystem::path& mulPath, const boost::filesystem::path& difOffsetsPath, const boost::filesystem::path& difPath,
                     unsigned int blockCountX, unsigned int blockCountY):
        blockCountX_(blockCountX), blockCountY_(blockCountY), difEnabled_(true) {

    if (boost::filesystem::exists(difOffsetsPath) && boost::filesystem::exists(difPath)) {
        FullFileLoader difOffsetsLoader(difOffsetsPath);
        difOffsetsLoader.read(boost::bind(&MapLoader::readCallbackDifOffsets, this, _1, _2));

        boost::shared_ptr<FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock> > difStream(new FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock>(difPath, 196,
            boost::bind(&MapLoader::readCallbackMul, this, _1, _2, _3, _4, _5, _6)));
        difCache_.init(difStream);
    } else {
        LOG_WARN << "Unable to open dif files" << std::endl;
        difEnabled_ = false;
    }

    boost::shared_ptr<FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock> > mulStream(new FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock>(mulPath, 196,
            boost::bind(&MapLoader::readCallbackMul, this, _1, _2, _3, _4, _5, _6)));
    mulCache_.init(mulStream);

}

MapLoader::MapLoader(const boost::filesystem::path& mulPath, unsigned int blockCountX, unsigned int blockCountY) :
        blockCountX_(blockCountX), blockCountY_(blockCountY), difEnabled_(false) {

    boost::shared_ptr<FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock> > mulStream(new FixedSizeOnDemandFileLoader<unsigned int, world::MapBlock>(mulPath, 196,
            boost::bind(&MapLoader::readCallbackMul, this, _1, _2, _3, _4, _5, _6)));
    mulCache_.init(mulStream);
}

void MapLoader::readCallbackMul(unsigned int index, int8_t* buf, unsigned int len, boost::shared_ptr<world::MapBlock> item, unsigned int extra, unsigned int userData) {
    unsigned int blockX = userData / blockCountY_;
    unsigned int blockY = userData % blockCountY_;

    item->blockIndexX_ = blockX;
    item->blockIndexY_ = blockY;
    // 4 bytes header
    item->setRawData(buf + 4, len - 4);
    item->generateMiniMap();
}

void MapLoader::setSurroundingZ(world::MapBlock* centerBlock) {
    int8_t zValues[15][15];
    memset(&zValues, 0, 225);

    world::MapBlock* blocks[3][3];
    bool blocksLoaded[3][3];

    // the current block
    blocks[1][1] = centerBlock;
    blocksLoaded[1][1] = true;

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }

            int idxX = (int)centerBlock->blockIndexX_ + i;
            int idxY = (int)centerBlock->blockIndexY_ + j;

            if (idxX < 0 || idxX >= (int)blockCountX_ || idxY < 0 || idxY >= (int)blockCountY_) {
                blocksLoaded[i+1][j+1] = false;
                continue;
            }

            blocks[i+1][j+1] = getNoCreate(idxX, idxY).get();
            blocksLoaded[i+1][j+1] = blocks[i+1][j+1] && blocks[i+1][j+1]->isReadComplete() && blocks[i+1][j+1]->mapTilesLoaded();
        }
    }

    //LOGARG_DEBUG(LOGTYPE_DATA, "blocks loaded: \n [%i %i %i\n %i %i %i\n %i %i %i]\n",
        //blocksLoaded[0][0], blocksLoaded[0][1], blocksLoaded[0][2],
        //blocksLoaded[1][0], blocksLoaded[1][1], blocksLoaded[1][2],
        //blocksLoaded[2][0], blocksLoaded[2][1], blocksLoaded[2][2]);

    // step 1: fill the z map with the neighboring z values
    unsigned int blockIndices[] = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 };
    unsigned int tileIndices[] = { 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };

    //unsigned int blockIndices[] = { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2 };
    //unsigned int tileIndices[] = { 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 };

    for (unsigned int x = 0; x < 15; ++x) {
        for (unsigned int y = 0; y < 15; ++y) {
            if (blocksLoaded[blockIndices[x]][blockIndices[y]]) {
                zValues[x][y] = blocks[blockIndices[x]][blockIndices[y]]->get(tileIndices[x], tileIndices[y])->getLocZGame();
            }
        }
    }

    /*  _____ _____
     * | cur | rig |
     * |_____|_____|
     * | lef | bot |
     * |_____|_____|
     *
     */
    // set the surrounding z values for tiles with a new important neighboring tile
    for (unsigned int x = 2; x < 11; ++x) {
        for (unsigned int y = 2; y < 11; ++y) {
            if (blocksLoaded[blockIndices[x]][blockIndices[y]]) {
                blocks[blockIndices[x]][blockIndices[y]]->get(tileIndices[x], tileIndices[y])->setSurroundingZ(zValues[x][y + 1], zValues[x+1][y], zValues[x + 1][y+1]);
            }
        }
    }

    /*  _____ _____ _____ _____
     * |     | t10 | t20 |     |
     * |_____|_____|_____|_____|
     * | t01 | cur | t21 | t31 |
     * |_____|_____|_____|_____|
     * | t02 | t12 | t22 | t32 |
     * |_____|_____|_____|_____|
     * |     | t13 | t23 |     |
     * |_____|_____|_____|_____|
     */
    // calculate new normals for all tiles that are influenced by this sector's z values
    int8_t cur, t10, t20, t01, t21, t31, t02, t12, t22, t32, t13, t23;
    world::MapTile* curTile;
    CL_Vec3f topNormal, rightNormal, bottomNormal, leftNormal;

    for (unsigned int x = 1; x < 13; ++x) {
        for (unsigned int y = 1; y < 13; ++y) {
            if (blocksLoaded[blockIndices[x]][blockIndices[y]]) {
                curTile = blocks[blockIndices[x]][blockIndices[y]]->get(tileIndices[x], tileIndices[y]);

                cur = zValues[x][y];
                t10 = zValues[x][y-1];
                t20 = zValues[x+1][y-1];
                t01 = zValues[x-1][y];
                t21 = zValues[x+1][y];
                t31 = zValues[x+2][y];
                t02 = zValues[x-1][y+1];
                t12 = zValues[x][y+1];
                t22 = zValues[x+1][y+1];
                t32 = zValues[x+2][y+1];
                t13 = zValues[x][y+2];
                t23 = zValues[x+1][y+2];

                //bool enableDebug = (curTile->getLocX() == 1401 || curTile->getLocX() == 1401) && curTile->getLocY() == 1652;
                //if (enableDebug) {
                    //LOG_DEBUG << "z calc debug:\n" <<
                            //"\t\t"                     << (int)t10 << "\t" << (int)t20 << "\n" <<
                            //"\t"   << (int)t01 << "\t" << (int)cur << "\t" << (int)t21 << "\t" << (int)t31 << "\n" <<
                            //"\t"   << (int)t02 << "\t" << (int)t12 << "\t" << (int)t22 << "\t" << (int)t31 <<"\n" <<
                            //"\t\t"                     << (int)t13 << "\t" << (int)t23 <<
                        //std::endl;

                    //LOG_DEBUG << "normal: cur=" << (int)cur << " top=" << (int)t10 << " right=" << (int)t21 << " bottom=" << (int)t12 << " left=" << (int)t01 << std::endl;
                //}

                topNormal = calculateNormal(cur, t10, t21, t12, t01);
                rightNormal = calculateNormal(t21, t20, t31, t22, cur);
                bottomNormal = calculateNormal(t22, t21, t32, t23, t12);
                leftNormal = calculateNormal(t12, cur, t22, t13, t02);

                curTile->setVertexNormals(topNormal, rightNormal, bottomNormal, leftNormal);
            }
        }
    }


    // request repaint of all involved map blocks
    for (unsigned int x = 0; x < 3; ++x) {
        for (unsigned int y = 0; y < 3; ++y) {
            if (blocksLoaded[x][y]) {
                blocks[x][y]->repaintRequested_ = true;
            }
        }
    }
}

/*  _____ _____ _____
 * |     | top |     |
 * |_____|_____|_____|
 * | lef | til | rig |
 * |_____|_____|_____|
 * |     | bot |     |
 * |_____|_____|_____|
 */
CL_Vec3f MapLoader::calculateNormal(int8_t tile, int8_t top, int8_t right, int8_t bottom, int8_t left) {
    if (tile == top && tile == right && tile == bottom && tile == left) {
        return CL_Vec3f(0, 0, 1);
    }

    CL_Vec3f u, v, ret;

    u = CL_Vec3f(-22, -22, (left-tile)*4);
    v = CL_Vec3f(-22, 22, (bottom-tile)*4);
    ret = CL_Vec3f::cross(v, u).normalize();

    u = CL_Vec3f(-22, 22, (bottom-tile)*4);
    v = CL_Vec3f(22, 22, (right-tile)*4);
    ret += CL_Vec3f::cross(v, u).normalize();

    u = CL_Vec3f(22, 22, (right-tile)*4);
    v = CL_Vec3f(22, -22, (top-tile)*4);
    ret += CL_Vec3f::cross(v, u).normalize();

    u = CL_Vec3f(22, -22, (top-tile)*4);
    v = CL_Vec3f(-22, -22, (left-tile)*4);
    ret += CL_Vec3f::cross(v, u).normalize();

    return ret.normalize();
}

void MapLoader::readCallbackDifOffsets(int8_t* buf, unsigned int len) {
    unsigned int entryCount = len / 4;
    uint32_t* ptr = reinterpret_cast<uint32_t*>(buf);

    for (unsigned int i = 0; i < entryCount; ++i) {
        unsigned int curEntry = *ptr;
        ptr++;
        difEntries_[curEntry] = i;
    }
}

boost::shared_ptr<world::MapBlock> MapLoader::get(unsigned int x, unsigned int y) {
    if (x >= blockCountX_) {
        x = 0;
    }

    if (y >= blockCountY_) {
        y = 0;
    }

    unsigned int idx = x * blockCountY_ + y;

    if (difEnabled_) {
        // check if there is a dif entry for this block
        std::map<unsigned int, unsigned int>::const_iterator iter = difEntries_.find(idx);
        if (iter != difEntries_.end()) {
            return difCache_.get(iter->second, idx);
        } else {
            return mulCache_.get(idx, idx);
        }
    } else {
        return mulCache_.get(idx, idx);
    }
}

boost::shared_ptr<world::MapBlock> MapLoader::getNoCreate(unsigned int x, unsigned int y) {
    if (x >= blockCountX_) {
        x = 0;
    }

    if (y >= blockCountY_) {
        y = 0;
    }

    unsigned int idx = x * blockCountY_ + y;

    if (difEnabled_) {
        // check if there is a dif entry for this block
        std::map<unsigned int, unsigned int>::const_iterator iter = difEntries_.find(idx);
        if (iter != difEntries_.end()) {
            return difCache_.getNoCreate(iter->second);
        } else {
            return mulCache_.getNoCreate(idx);
        }
    } else {
        return mulCache_.getNoCreate(idx);
    }
}

unsigned int MapLoader::getBlockCountX() {
    return blockCountX_;
}

unsigned int MapLoader::getBlockCountY() {
    return blockCountY_;
}


}
}
