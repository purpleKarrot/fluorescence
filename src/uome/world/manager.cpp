
#include "manager.hpp"

#include "sectormanager.hpp"
#include "lightmanager.hpp"

#include <misc/exception.hpp>
#include <misc/log.hpp>

#include <net/packets/playerinit.hpp>

namespace uome {
namespace world {

Manager* Manager::singleton_ = NULL;

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw Exception("uome::world::Manager Singleton not created yet");
    }
    return singleton_;
}

bool Manager::create(const Config& config) {
    if (!singleton_) {
        try {
            singleton_ = new Manager(config);
        } catch (const std::exception& ex) {
            LOG_EMERGENCY << "Error initializing world::Manager: " << ex.what() << std::endl;
            return false;
        }
    }

    return true;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager::Manager(const Config& config) : currentMapId_(0) {
    sectorManager_.reset(new SectorManager(config));
    lightManager_.reset(new LightManager());
}

Manager::~Manager() {
    LOG_INFO << "world::Manager shutdown" << std::endl;
}

boost::shared_ptr<SectorManager> Manager::getSectorManager() {
    return sectorManager_;
}

unsigned int Manager::getCurrentMapId() {
    return currentMapId_;
}

void Manager::setCurrentMapId(unsigned int id) {
    currentMapId_ = id;
}

boost::shared_ptr<LightManager> Manager::getLightManager() {
    return lightManager_;
}

void Manager::initPlayer(const net::packets::PlayerInit* packet) {
    player_.reset(new Mobile(packet->serial_));
    player_->setLocation(packet->locX_, packet->locY_, packet->locZ_);
    player_->setBodyId(packet->bodyId_);
    player_->setDirection(packet->direction_);

    LOG_DEBUG << "Location after player init: " << player_->getLocX() << "/" << player_->getLocY() << "/" << (unsigned int)player_->getLocZ() << std::endl;
}

boost::shared_ptr<Mobile> Manager::getPlayer() {
    return player_;
}

}
}
