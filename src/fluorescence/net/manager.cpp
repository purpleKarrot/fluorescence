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



#include "manager.hpp"

#include <client.hpp>
#include <misc/log.hpp>
#include <misc/exception.hpp>
#include <ui/manager.hpp>
#include <ui/gumpmenu.hpp>
#include <ui/gumpmenus.hpp>
#include <ui/components/lineedit.hpp>

#include "packetlist.hpp"
#include "twofishencryption.hpp"
#include "walkpacketmanager.hpp"

namespace fluo {
namespace net {

Manager* Manager::singleton_ = 0;

bool Manager::create(Config& config) {
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

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw fluo::Exception("fluo::net::Manager Singleton not created yet");
    }

    return singleton_;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager::Manager(Config& config) {
#ifdef WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) {
        LOG_EMERGENCY << "WSAStartup failed: " << err << " - " << strerror(err) << std::endl;
        throw std::exception();
    } else {
        LOG_INFO << "WSAStartup ok" << std::endl;
    }
#endif

    UnicodeString protocolVersionStr = config["/fluo/shard/protocol@version"].asString();
    if (protocolVersionStr == "pre-hs") {
        protocolVersion_ = ProtocolVersion::PRE_HS;
    } else if (protocolVersionStr == "hs") {
        protocolVersion_ = ProtocolVersion::HS;
    } else {
        LOG_ERROR << "Unknown network protocol version " << protocolVersionStr << ". Valid versions are \"hs\" and \"pre-hs\". Setting to default \"pre-hs\"." << std::endl;
        protocolVersion_ = ProtocolVersion::PRE_HS;
    }

    walkPacketManager_.reset(new WalkPacketManager());
}

Manager::~Manager() {
    LOG_INFO << "net::Manager shutdown" << std::endl;

#ifdef WIN32
    WSACleanup();
#endif
}

void Manager::step() {
    if (socket_.isConnected()) {
        // handle received packets
        boost::shared_ptr<Packet> packet = socket_.getNextPacket();
        while (packet) {
            LOG_DEBUG << "Calling handler for packet " << std::hex << (unsigned int)packet->getId() << std::dec << std::endl;
            packet->onReceive();
            packet = socket_.getNextPacket();
        }

        // send packets to server
        socket_.sendAll();

        if (socket_.hasCriticalError()) {
            ui::GumpMenus::openMessageBox("Lost connection to server");
            Client::getSingleton()->disconnect();
        }
    }
}

boost::shared_ptr<Packet> Manager::createPacket(uint8_t id) {
    boost::shared_ptr<Packet> ret;

    switch (id) {
        case 0x11: ret.reset(new packets::StatUpdateFull()); break;
        case 0x1A: ret.reset(new packets::WorldItem()); break;
        case 0x1B: ret.reset(new packets::PlayerInit()); break;
        case 0x1C: ret.reset(new packets::AsciiText()); break;
        case 0x1D: ret.reset(new packets::DeleteObject()); break;
        case 0x20: ret.reset(new packets::Teleport()); break;
        case 0x21: ret.reset(new packets::MovementDeny()); break;
        case 0x22: ret.reset(new packets::MovementAccept()); break;
        case 0x24: ret.reset(new packets::DisplayContainer()); break;
        case 0x25: ret.reset(new packets::ContainerUpdate()); break;
        case 0x27: ret.reset(new packets::PickUpReject()); break;
        case 0x2E: ret.reset(new packets::EquippedItem()); break;
        case 0x3A: ret.reset(new packets::SkillsUpdate()); break;
        case 0x3C: ret.reset(new packets::ContainerContent()); break;
        case 0x54: ret.reset(new packets::PlaySoundPacket()); break;
        case 0x55: ret.reset(new packets::LoginComplete()); break;
        case 0x6C: ret.reset(new packets::Target()); break;
        case 0x6D: ret.reset(new packets::PlayMusic()); break;
        case 0x6E: ret.reset(new packets::MobileAnimation()); break;
        case 0x72: ret.reset(new packets::WarMode()); break;
        case 0x77: ret.reset(new packets::NakedMobile()); break;
        case 0x78: ret.reset(new packets::EquippedMobile()); break;
        case 0x85: ret.reset(new packets::CharacterDeleteReject()); break;
        case 0x86: ret.reset(new packets::CharacterListUpdate()); break;
        case 0x88: ret.reset(new packets::OpenPaperdoll()); break;
        case 0xA8: ret.reset(new packets::ServerList()); break;
        case 0x8C: ret.reset(new packets::ServerRedirect()); break;
        case 0xA1: ret.reset(new packets::StatUpdateHitpoints()); break;
        case 0xA2: ret.reset(new packets::StatUpdateMana()); break;
        case 0xA3: ret.reset(new packets::StatUpdateStamina()); break;
        case 0xA9: ret.reset(new packets::CharacterList()); break;
        case 0xAE: ret.reset(new packets::UnicodeText()); break;
        case 0xB0: ret.reset(new packets::GumpMenu()); break;
        case 0xB8: ret.reset(new packets::ProfileResponse()); break;
        case 0xBD: ret.reset(new packets::ClientVersion()); break;
        case 0xBF: ret.reset(new packets::BF()); break;
        case 0xC0: ret.reset(new packets::OsiEffect()); break;
        case 0xC1: ret.reset(new packets::LocalizedText()); break;
        case 0xC7: ret.reset(new packets::OsiEffectExtended()); break;
        case 0xC8: ret.reset(new packets::UpdateRange()); break;
        case 0xCC: ret.reset(new packets::LocalizedTextAffix()); break;
        case 0xE2: ret.reset(new packets::MobileAnimationSA()); break;
        case 0xF3: ret.reset(new packets::WorldObject()); break;
        default: ret.reset(new packets::Unknown(id)); break;
    }

    return ret;
}

bool Manager::connect(ui::GumpMenu* menu, const UnicodeString& action, unsigned int parameterCount, const UnicodeString* parameters) {
    UnicodeString host;
    if (!menu->getComponentText<ui::components::LineEdit>("loginhost", host)) {
        ui::GumpMenus::openMessageBox("Unable to find input field for the server host. Did you change the login gump template?");
        return false;
    }

    ui::components::LineEdit* line = dynamic_cast<ui::components::LineEdit*>(menu->get_named_item("loginport"));
    if (!line) {
        ui::GumpMenus::openMessageBox("Unable to find input field for the server port. Did you change the login gump template?");
        return false;
    }

    int port = line->get_text_int();
    if (port <= 0) {
        ui::GumpMenus::openMessageBox("Unable to parse port number");
        return false;
    }


    UnicodeString accName;
    if (!menu->getComponentText<ui::components::LineEdit>("loginaccount", accName)) {
        ui::GumpMenus::openMessageBox("Unable to find input field for the account name. Did you change the login gump template?");
        return false;
    }

    UnicodeString accPw;
    if (!menu->getComponentText<ui::components::LineEdit>("loginpassword", accPw)) {
        ui::GumpMenus::openMessageBox("Unable to find input field for the password. Did you change the login gump template?");
        return false;
    }




    if (socket_.connect(host, port)) {
        Client* clientSing = Client::getSingleton();
        clientSing->setState(Client::STATE_LOGIN);

        clientSing->getConfig()["/fluo/shard/account@name"].setString(accName);
        clientSing->getConfig()["/fluo/shard/account@password"].setString(accPw);
        clientSing->getConfig()["/fluo/shard/address@host"].setString(host);
        clientSing->getConfig()["/fluo/shard/address@port"].setInt(port);

        socket_.writeSeed(0xDEADBEEF);
        // send packet
        packets::LoginRequest req(accName, accPw);
        socket_.write(req);

        return true;
    } else {
        ui::GumpMenus::openMessageBox("Could not connect to server");
        return false;
    }
}

void Manager::disconnect() {
    socket_.close();
}

bool Manager::selectServer(ui::GumpMenu* menu, const UnicodeString& action, unsigned int parameterCount, const UnicodeString* parameters) {
    unsigned int index = StringConverter::toInt(parameters[0]);

    packets::GameServerSelect pkt(index);
    socket_.write(pkt);

    return true;
}

void Manager::handleServerRedirect(const packets::ServerRedirect* packet) {
    socket_.close();

    // TODO: encryption stuff

    socket_.connect(packet->ipaddr_, packet->port_);
    socket_.setUseDecompress(true);

    socket_.writeSeed(packet->encryptionKey_);

    Config& config = Client::getSingleton()->getConfig();
    packets::GameServerLoginRequest loginReq(config["/fluo/shard/account@name"].asString(),
            config["/fluo/shard/account@password"].asString(),
            packet->encryptionKey_);
    socket_.write(loginReq);
}

uint32_t Manager::getSeed() const {
    return socket_.getSeed();
}

boost::shared_ptr<WalkPacketManager> Manager::getWalkPacketManager() {
    return getSingleton()->walkPacketManager_;
}

unsigned int Manager::getProtocolVersion() const {
    return protocolVersion_;
}

}
}
