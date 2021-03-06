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


#ifndef FLUO_DATA_FILE_PATH_LOADER_HPP
#define FLUO_DATA_FILE_PATH_LOADER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

#include <misc/string.hpp>

#include "ondemandurlloader.hpp"
#include "weakptrcache.hpp"

namespace fluo {
namespace ui {
    class Texture;
}

namespace data {

class FilePathLoader {
public:
    FilePathLoader();

    boost::shared_ptr<ui::Texture> getTexture(const boost::filesystem::path& url);

    void readTextureCallback(const boost::filesystem::path& url, boost::shared_ptr<ui::Texture> tex);

    void clearCache();

private:
    WeakPtrCache<boost::filesystem::path, ui::Texture, OnDemandUrlLoader> cache_;
};

}
}

#endif

