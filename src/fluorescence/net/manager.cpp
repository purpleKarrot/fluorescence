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
#include "packets/ef_seed.hpp"

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
    } else if (protocolVersionStr == "hs-7013") {
        protocolVersion_ = ProtocolVersion::HS_7013;
    } else {
        LOG_ERROR << "Unknown network protocol version " << protocolVersionStr << ". Valid versions are \"pre-hs\", \"hs\" and \"hs-7013\" . Setting to default \"pre-hs\"." << std::endl;
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

        // send ping packets, if nothing else has been sent in the last 20 seconds
        if (lastSendTime_ + 20 < time(NULL)) {
            net::packets::Ping ping;
            send(ping);
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
        case 0x65: ret.reset(new packets::Weather()); break;
        case 0x6C: ret.reset(new packets::Target()); break;
        case 0x6D: ret.reset(new packets::PlayMusic()); break;
        case 0x6E: ret.reset(new packets::MobileAnimation()); break;
        case 0x72: ret.reset(new packets::WarMode()); break;
        case 0x73: ret.reset(new packets::Ping()); break;
        case 0x77: ret.reset(new packets::NakedMobile()); break;
        case 0x78: ret.reset(new packets::EquippedMobile()); break;
        case 0x7C: ret.reset(new packets::ObjectPicker()); break;
        case 0x85: ret.reset(new packets::CharacterDeleteReject()); break;
        case 0x86: ret.reset(new packets::CharacterListUpdate()); break;
        case 0x88: ret.reset(new packets::OpenPaperdoll()); break;
        case 0x9E: ret.reset(new packets::VendorSellList()); break;
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
        case 0xC2: ret.reset(new packets::UnicodePrompt()); break;
        case 0xC7: ret.reset(new packets::OsiEffectExtended()); break;
        case 0xC8: ret.reset(new packets::UpdateRange()); break;
        case 0xCC: ret.reset(new packets::LocalizedTextAffix()); break;
        case 0xD6: ret.reset(new packets::ObjectProperties()); break;
        case 0xDC: ret.reset(new packets::ObjectPropertiesHash()); break;
        case 0xDD: ret.reset(new packets::CompressedGumpMenu()); break;
        case 0xDF: ret.reset(new packets::Buff()); break;
        case 0xE2: ret.reset(new packets::MobileAnimationSA()); break;
        case 0xF3: ret.reset(new packets::WorldObject()); break;
        default: ret.reset(new packets::Unknown(id)); break;
    }

    return ret;
}

bool Manager::connect(const UnicodeString& host, unsigned int port, const UnicodeString& account, const UnicodeString& password) {
    if (socket_.connect(host, port)) {
        Client* clientSing = Client::getSingleton();
        clientSing->setState(Client::STATE_LOGIN);

        clientSing->getConfig()["/fluo/shard/account@name"].setString(account);
        clientSing->getConfig()["/fluo/shard/account@password"].setString(password);
        clientSing->getConfig()["/fluo/shard/address@host"].setString(host);
        clientSing->getConfig()["/fluo/shard/address@port"].setInt(port);

        packets::Seed seed(0xDEADBEEF,
            clientSing->getConfig()["/fluo/shard/clientversion@major"].asInt(),
            clientSing->getConfig()["/fluo/shard/clientversion@minor"].asInt(),
            clientSing->getConfig()["/fluo/shard/clientversion@revision"].asInt(),
            clientSing->getConfig()["/fluo/shard/clientversion@build"].asInt()
        );
        send(seed);

        // send packet
        packets::LoginRequest req(account, password);
        send(req);

        return true;
    } else {
        return false;
    }
}

void Manager::disconnect() {
    socket_.close();
}

bool Manager::selectServer(unsigned int index) {
    packets::GameServerSelect pkt(index);
    send(pkt);
    return true;
}

bool Manager::selectCharacter(unsigned int index, const UnicodeString& name, const UnicodeString& password) {
    packets::CharacterSelect reply(name, password, index, getSeed());
    send(reply);
    return true;
}

void Manager::handleServerRedirect(const packets::ServerRedirect* packet) {
    socket_.close();

    // TODO: encryption stuff

    socket_.connect(packet->ipaddr_, packet->port_);
    socket_.setUseDecompress(true);
    socket_.setSeed(packet->encryptionKey_);
    socket_.writeShortSeed();

    Config& config = Client::getSingleton()->getConfig();
    packets::GameServerLoginRequest loginReq(config["/fluo/shard/account@name"].asString(),
            config["/fluo/shard/account@password"].asString(),
            packet->encryptionKey_);
    send(loginReq);
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
