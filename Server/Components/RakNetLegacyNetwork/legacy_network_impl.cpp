#include "legacy_network_impl.hpp"
#include "raknet/../../SAMPRakNet.hpp"
#include "Query/query.hpp"
#include <netcode.hpp>
#include <raknet/PacketEnumerations.h>

#define RPCHOOK(id) rakNetServer.RegisterAsRemoteProcedureCall(id, &RakNetLegacyNetwork::RPCHook<id>, this)

RakNetLegacyNetwork::RakNetLegacyNetwork() :
    Network(256, 256),
    core(nullptr),
    rakNetServer(*RakNet::RakNetworkFactory::GetRakServerInterface()),
    wlbs(wbs)
{
    rakNetServer.SetMTUSize(512);

    RPCHOOK(0);
    RPCHOOK(1);
    RPCHOOK(2);
    RPCHOOK(3);
    RPCHOOK(4);
    RPCHOOK(5);
    RPCHOOK(6);
    RPCHOOK(7);
    RPCHOOK(8);
    RPCHOOK(9);
    RPCHOOK(10);
    RPCHOOK(11);
    RPCHOOK(12);
    RPCHOOK(13);
    RPCHOOK(14);
    RPCHOOK(15);
    RPCHOOK(16);
    RPCHOOK(17);
    RPCHOOK(18);
    RPCHOOK(19);
    RPCHOOK(20);
    RPCHOOK(21);
    RPCHOOK(22);
    RPCHOOK(23);
    RPCHOOK(24);
    rakNetServer.RegisterAsRemoteProcedureCall(NetCode::RPC::PlayerConnect::getID(ENetworkType_RakNetLegacy), &RakNetLegacyNetwork::OnPlayerConnect, this);
    RPCHOOK(26);
    RPCHOOK(27);
    RPCHOOK(28);
    RPCHOOK(29);
    RPCHOOK(30);
    RPCHOOK(31);
    RPCHOOK(32);
    RPCHOOK(33);
    RPCHOOK(34);
    RPCHOOK(35);
    RPCHOOK(36);
    RPCHOOK(37);
    RPCHOOK(38);
    RPCHOOK(39);
    RPCHOOK(40);
    RPCHOOK(41);
    RPCHOOK(42);
    RPCHOOK(43);
    RPCHOOK(44);
    RPCHOOK(45);
    RPCHOOK(46);
    RPCHOOK(47);
    RPCHOOK(48);
    RPCHOOK(49);
    RPCHOOK(50);
    RPCHOOK(51);
    RPCHOOK(52);
    RPCHOOK(53);
    RPCHOOK(54);
    RPCHOOK(55);
    RPCHOOK(56);
    RPCHOOK(57);
    RPCHOOK(58);
    RPCHOOK(59);
    RPCHOOK(60);
    RPCHOOK(61);
    RPCHOOK(62);
    RPCHOOK(63);
    RPCHOOK(64);
    RPCHOOK(65);
    RPCHOOK(66);
    RPCHOOK(67);
    RPCHOOK(68);
    RPCHOOK(69);
    RPCHOOK(70);
    RPCHOOK(71);
    RPCHOOK(72);
    RPCHOOK(73);
    RPCHOOK(74);
    RPCHOOK(75);
    RPCHOOK(76);
    RPCHOOK(77);
    RPCHOOK(78);
    RPCHOOK(79);
    RPCHOOK(80);
    RPCHOOK(81);
    RPCHOOK(82);
    RPCHOOK(83);
    RPCHOOK(84);
    RPCHOOK(85);
    RPCHOOK(86);
    RPCHOOK(87);
    RPCHOOK(88);
    RPCHOOK(89);
    RPCHOOK(90);
    RPCHOOK(91);
    RPCHOOK(92);
    RPCHOOK(93);
    RPCHOOK(94);
    RPCHOOK(95);
    RPCHOOK(96);
    RPCHOOK(97);
    RPCHOOK(98);
    RPCHOOK(99);
    RPCHOOK(100);
    RPCHOOK(101);
    RPCHOOK(102);
    RPCHOOK(103);
    RPCHOOK(104);
    RPCHOOK(105);
    RPCHOOK(106);
    RPCHOOK(107);
    RPCHOOK(108);
    RPCHOOK(109);
    RPCHOOK(110);
    RPCHOOK(111);
    RPCHOOK(112);
    RPCHOOK(113);
    RPCHOOK(114);
    RPCHOOK(115);
    RPCHOOK(116);
    RPCHOOK(117);
    RPCHOOK(118);
    RPCHOOK(119);
    RPCHOOK(120);
    RPCHOOK(121);
    RPCHOOK(122);
    RPCHOOK(123);
    RPCHOOK(124);
    RPCHOOK(125);
    RPCHOOK(126);
    RPCHOOK(127);
    RPCHOOK(128);
    RPCHOOK(129);
    RPCHOOK(130);
    RPCHOOK(131);
    RPCHOOK(132);
    RPCHOOK(133);
    RPCHOOK(134);
    RPCHOOK(135);
    RPCHOOK(136);
    RPCHOOK(137);
    RPCHOOK(138);
    RPCHOOK(139);
    RPCHOOK(140);
    RPCHOOK(141);
    RPCHOOK(142);
    RPCHOOK(143);
    RPCHOOK(144);
    RPCHOOK(145);
    RPCHOOK(146);
    RPCHOOK(147);
    RPCHOOK(148);
    RPCHOOK(149);
    RPCHOOK(150);
    RPCHOOK(151);
    RPCHOOK(152);
    RPCHOOK(153);
    RPCHOOK(154);
    RPCHOOK(155);
    RPCHOOK(156);
    RPCHOOK(157);
    RPCHOOK(158);
    RPCHOOK(159);
    RPCHOOK(160);
    RPCHOOK(161);
    RPCHOOK(162);
    RPCHOOK(163);
    RPCHOOK(164);
    RPCHOOK(165);
    RPCHOOK(166);
    RPCHOOK(167);
    RPCHOOK(168);
    RPCHOOK(169);
    RPCHOOK(170);
    RPCHOOK(171);
    RPCHOOK(172);
    RPCHOOK(173);
    RPCHOOK(174);
    RPCHOOK(175);
    RPCHOOK(176);
    RPCHOOK(177);
    RPCHOOK(178);
    RPCHOOK(179);
    RPCHOOK(180);
    RPCHOOK(181);
    RPCHOOK(182);
    RPCHOOK(183);
    RPCHOOK(184);
    RPCHOOK(185);
    RPCHOOK(186);
    RPCHOOK(187);
    RPCHOOK(188);
    RPCHOOK(189);
    RPCHOOK(190);
    RPCHOOK(191);
    RPCHOOK(192);
    RPCHOOK(193);
    RPCHOOK(194);
    RPCHOOK(195);
    RPCHOOK(196);
    RPCHOOK(197);
    RPCHOOK(198);
    RPCHOOK(199);
    RPCHOOK(200);
    RPCHOOK(201);
    RPCHOOK(202);
    RPCHOOK(203);
    RPCHOOK(204);
    RPCHOOK(205);
    RPCHOOK(206);
    RPCHOOK(207);
    RPCHOOK(208);
    RPCHOOK(209);
    RPCHOOK(210);
    RPCHOOK(211);
    RPCHOOK(212);
    RPCHOOK(213);
    RPCHOOK(214);
    RPCHOOK(215);
    RPCHOOK(216);
    RPCHOOK(217);
    RPCHOOK(218);
    RPCHOOK(219);
    RPCHOOK(220);
    RPCHOOK(221);
    RPCHOOK(222);
    RPCHOOK(223);
    RPCHOOK(224);
    RPCHOOK(225);
    RPCHOOK(226);
    RPCHOOK(227);
    RPCHOOK(228);
    RPCHOOK(229);
    RPCHOOK(230);
    RPCHOOK(231);
    RPCHOOK(232);
    RPCHOOK(233);
    RPCHOOK(234);
    RPCHOOK(235);
    RPCHOOK(236);
    RPCHOOK(237);
    RPCHOOK(238);
    RPCHOOK(239);
    RPCHOOK(240);
    RPCHOOK(241);
    RPCHOOK(242);
    RPCHOOK(243);
    RPCHOOK(244);
    RPCHOOK(245);
    RPCHOOK(246);
    RPCHOOK(247);
    RPCHOOK(248);
    RPCHOOK(249);
    RPCHOOK(250);
    RPCHOOK(251);
    RPCHOOK(252);
    RPCHOOK(253);
}

#undef RPCHOOK

RakNetLegacyNetwork::~RakNetLegacyNetwork() {
    if (core) {
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
    }
    rakNetServer.Disconnect(300);
    RakNet::RakNetworkFactory::DestroyRakServerInterface(&rakNetServer);
}

static RakNet::BitStream GetBitStream(RakNet::RPCParameters& rpcParams)
{
    unsigned int
        bits = rpcParams.numberOfBitsOfData;
    if (bits == 0)
        return RakNet::BitStream();
    unsigned int
        bytes = (bits - 1) / 8 + 1;
    return RakNet::BitStream(rpcParams.input, bytes, false);
}

enum LegacyClientVersion {
    LegacyClientVersion_037 = 4057,
    LegacyClientVersion_03DL = 4062
};

void RakNetLegacyNetwork::OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra) {
    RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
    const RakNet::PlayerID rid = rpcParams->sender;

    if (network->playerFromRID.find(rpcParams->sender) != network->playerFromRID.end()) {
        // Connection already exists
        return;
    }

    char cIP[22];
    unsigned short cPort;
    network->rakNetServer.GetPlayerIPFromID(rpcParams->sender, cIP, &cPort);

    PeerNetworkData netData;
    netData.networkID.address = rid.binaryAddress;
    netData.networkID.port = rid.port;
    netData.network = network;
    netData.IP = cIP;
    netData.port = cPort;

    RakNet::BitStream bs = GetBitStream(*rpcParams);
    RakNetLegacyBitStream lbs(bs);

    Pair<NewConnectionResult, IPlayer*> newConnectionResult { NewConnectionResult_Ignore, nullptr };

    {
        NetCode::RPC::PlayerConnect playerConnectRPC;
        playerConnectRPC.read(lbs);

        if (playerConnectRPC.VersionNumber != LegacyClientVersion_037 || SAMPRakNet::GetToken() != (playerConnectRPC.ChallengeResponse ^ LegacyClientVersion_037)) {
            if (playerConnectRPC.VersionNumber != LegacyClientVersion_03DL || SAMPRakNet::GetToken() != (playerConnectRPC.ChallengeResponse ^ LegacyClientVersion_03DL)) {
                newConnectionResult.first = NewConnectionResult_VersionMismatch;
            }
        }
    }

    network->networkEventDispatcher.anyTrue(
        [&netData, &lbs, &newConnectionResult](NetworkEventHandler* handler) {
            lbs.reset(BSResetRead);
            return (newConnectionResult = handler->onPeerRequest(netData, lbs)).first == NewConnectionResult_Success;
        }
    );

    if (newConnectionResult.first != NewConnectionResult_Success) {
        if (newConnectionResult.first != NewConnectionResult_Ignore) {
            // Entry denied, send reason and disconnect
            RakNet::BitStream bss;
            bss.Write(uint8_t(newConnectionResult.first));
            network->rakNetServer.RPC(130, &bss, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
            network->rakNetServer.Kick(rid);
        }
        return;
    }

    IPlayer& player = *newConnectionResult.second;
    network->playerFromRID.emplace(rid, player);

    network->inOutEventDispatcher.all(
        [&player, &lbs](NetworkInOutEventHandler* handler) {
            lbs.reset(BSResetRead);
            handler->receivedRPC(player, NetCode::RPC::PlayerConnect::getID(ENetworkType_RakNetLegacy), lbs);
        }
    );

    network->rpcInOutEventDispatcher.all(
        NetCode::RPC::PlayerConnect::getID(ENetworkType_RakNetLegacy),
        [&player, &lbs](SingleNetworkInOutEventHandler* handler) {
            lbs.reset(BSResetRead);
            handler->received(player, lbs);
        }
    );

    network->networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, player);
}

void RakNetLegacyNetwork::OnRakNetDisconnect(RakNet::PlayerID rid, PeerDisconnectReason reason) {
    PlayerFromRIDMap::iterator pos = playerFromRID.find(rid);
    if (pos == playerFromRID.end()) {
        return;
    }

    IPlayer& player = pos->second;
    playerFromRID.erase(rid);

    networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerDisconnect, player, reason);
}

template <size_t ID>
void RakNetLegacyNetwork::RPCHook(RakNet::RPCParameters* rpcParams, void* extra) {
    RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
    PlayerFromRIDMap::iterator pos = network->playerFromRID.find(rpcParams->sender);
    if (pos == network->playerFromRID.end()) {
        return;
    }

    IPlayer& player = pos->second;

    RakNet::BitStream bs = GetBitStream(*rpcParams);
    RakNetLegacyBitStream lbs(bs);

    bool processed = false;
    network->inOutEventDispatcher.all(
        [&player, &lbs, &processed](NetworkInOutEventHandler* handler) {
            lbs.reset(BSResetRead);
            processed |= handler->receivedRPC(player, ID, lbs);
        }
    );

    network->rpcInOutEventDispatcher.all(
        ID,
        [&player, &lbs, &processed](SingleNetworkInOutEventHandler* handler) {
            lbs.reset(BSResetRead);
            processed |= handler->received(player, lbs);
        }
    );

    if (!processed) {
        network->core->printLn("Received unprocessed RPC %i", ID);
    }
}

void RakNetLegacyNetwork::init(ICore* c) {
    core = c;
    core->getEventDispatcher().addEventHandler(this);
    core->getPlayers().getEventDispatcher().addEventHandler(this);
    SAMPRakNet::ServerCoreInit(c);
    query = Query(c);

    IConfig& config = core->getConfig();
    int maxPlayers = *config.getInt("max_players");
    StringView serverName = config.getString("server_name");
    int port = *config.getInt("port");
    int sleep = *config.getInt("sleep");

    rakNetServer.Start(
        maxPlayers,
        0,
        sleep,
        port
    );
    rakNetServer.StartOccasionalPing();

    SAMPRakNet::SetPort(port);
    query.setMaxPlayers(maxPlayers);
    query.setServerName(serverName);
}

void RakNetLegacyNetwork::onTick(std::chrono::microseconds elapsed) {
    for (RakNet::Packet* pkt = rakNetServer.Receive(); pkt; pkt = rakNetServer.Receive()) {
        auto pos = playerFromRID.find(pkt->playerId);
        if (pos != playerFromRID.end()) {
            IPlayer& player = pos->second;
            RakNet::BitStream bs(pkt->data, pkt->length, false);
            uint8_t type;
            if (bs.Read(type)) {
                if (type == RakNet::ID_DISCONNECTION_NOTIFICATION) {
                    OnRakNetDisconnect(pkt->playerId, PeerDisconnectReason_Quit);
                }
                else if (type == RakNet::ID_CONNECTION_LOST) {
                    OnRakNetDisconnect(pkt->playerId, PeerDisconnectReason_Timeout);
                }
                inOutEventDispatcher.all([&player, type, &bs](NetworkInOutEventHandler* handler) {
                    bs.SetReadOffset(8); // Ignore packet ID
                    RakNetLegacyBitStream lbs(bs);
                    handler->receivedPacket(player, type, lbs);
                    });

                packetInOutEventDispatcher.all(type, [&player, &bs](SingleNetworkInOutEventHandler* handler) {
                    bs.SetReadOffset(8); // Ignore packet ID
                    RakNetLegacyBitStream lbs(bs);
                    handler->received(player, lbs);
                    });
            }
        }
        rakNetServer.DeallocatePacket(pkt);
    }
}

int RakNetLegacyNetwork::handleQuery(const char * buffer, char * output) {
    int outputLength = query.handleQuery(buffer, output);
    return outputLength;
}
