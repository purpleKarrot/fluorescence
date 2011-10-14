#ifndef FLUO_DATA_HUESLOADER_HPP
#define FLUO_DATA_HUESLOADER_HPP

#include <boost/shared_ptr.hpp>

#include <misc/string.hpp>

#include "fullfileloader.hpp"

namespace fluo {

namespace ui {
    class Texture;
}

namespace data {

struct Hue {
    uint32_t colorTable_[32];
    uint16_t tableStart;
    uint16_t tableEnd;
    UnicodeString name_;
};

class HuesLoader {
public:
    HuesLoader(const boost::filesystem::path& path);
    ~HuesLoader();

    void readCallback(int8_t* buf,unsigned int len);

    unsigned int getHueCount() const;

    boost::shared_ptr<ui::Texture> getHuesTexture();
    uint32_t getFontRgbColor(unsigned int hue) const;

    unsigned int translateHue(unsigned int hue) const;

private:
    unsigned int hueCount_;
    Hue* hues_;

    boost::shared_ptr<ui::Texture> huesTexture_;
};

}
}

#endif