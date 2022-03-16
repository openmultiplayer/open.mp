#include "legacy_network_impl.hpp"
#include "Query/query.hpp"
#include "raknet/../../SAMPRakNet.hpp"
#include <netcode.hpp>
#include <raknet/PacketEnumerations.h>
#include <raknet/RakPeer.h>
#include <ttmath/ttmath.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>
#pragma clang diagnostic pop

#define RPCHOOK(id) rakNetServer.RegisterAsRemoteProcedureCall(id, &RakNetLegacyNetwork::RPCHook<id>, this)

RakNetLegacyNetwork::RakNetLegacyNetwork()
    : Network(256, 256)
    , core(nullptr)
    , rakNetServer(*RakNet::RakNetworkFactory::GetRakServerInterface())
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
    rakNetServer.RegisterAsRemoteProcedureCall(NetCode::RPC::PlayerConnect::PacketID, &RakNetLegacyNetwork::OnPlayerConnect, this);
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
    rakNetServer.RegisterAsRemoteProcedureCall(NetCode::RPC::NPCConnect::PacketID, &RakNetLegacyNetwork::OnNPCConnect, this);
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

RakNetLegacyNetwork::~RakNetLegacyNetwork()
{
    if (core) {
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
    }
    rakNetServer.Disconnect(300);
    RakNet::RakNetworkFactory::DestroyRakServerInterface(&rakNetServer);
}

static NetworkBitStream GetBitStream(RakNet::RPCParameters& rpcParams)
{
    unsigned int
        bits
        = rpcParams.numberOfBitsOfData;
    if (bits == 0)
        return NetworkBitStream();
    unsigned int
        bytes
        = (bits - 1) / 8 + 1;
    return NetworkBitStream(rpcParams.input, bytes, false);
}

enum LegacyClientVersion {
    LegacyClientVersion_037 = 4057,
    LegacyClientVersion_03DL = 4062
};

IPlayer* RakNetLegacyNetwork::OnPeerConnect(RakNet::RPCParameters* rpcParams, bool isNPC, StringView serial, uint32_t version, StringView versionName, uint32_t challenge, StringView name)
{
    const RakNet::PlayerID rid = rpcParams->sender;

    if (playerFromRakIndex[rpcParams->senderIndex]) {
        // Connection already exists
        return nullptr;
    }

    PeerNetworkData netData {};
    netData.networkID.address.ipv6 = false;
    netData.networkID.address.v4 = rid.binaryAddress;
    netData.networkID.port = rid.port;
    netData.network = this;

    Pair<NewConnectionResult, IPlayer*> newConnectionResult { NewConnectionResult_Ignore, nullptr };

    //Allow only 0.3.7 client for now
    if (version != LegacyClientVersion_037 || SAMPRakNet::GetToken() != (challenge ^ LegacyClientVersion_037)) {
        newConnectionResult.first = NewConnectionResult_VersionMismatch;
    } else {
        PeerRequestParams params;
        params.version = version;
        params.versionName = versionName;
        params.name = name;
        params.bot = isNPC;
        params.serial = serial;
        newConnectionResult = core->getPlayers().requestPlayer(netData, params);
    }

    if (newConnectionResult.first != NewConnectionResult_Success) {
        if (newConnectionResult.first != NewConnectionResult_Ignore) {
            // Entry denied, send reason and disconnect
            RakNet::BitStream bss;
            bss.Write(uint8_t(newConnectionResult.first));
            rakNetServer.RPC(130, &bss, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
            rakNetServer.Kick(rid);
        }
        return nullptr;
    }

    playerFromRakIndex[rpcParams->senderIndex] = newConnectionResult.second;
    return newConnectionResult.second;
}

void RakNetLegacyNetwork::OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra)
{
    RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
    RakNet::RakPeer::RemoteSystemStruct* remoteSystem = network->rakNetServer.GetRemoteSystemFromPlayerID(rpcParams->sender);

    if (!remoteSystem || remoteSystem->connectMode != RakNet::RakPeer::RemoteSystemStruct::ConnectMode::CONNECTED) {
        return;
    }

    if (remoteSystem->sampData.authType == SAMPRakNet::AuthType_Player) {
        NetworkBitStream bs = GetBitStream(*rpcParams);
        NetCode::RPC::PlayerConnect playerConnectRPC;
        if (playerConnectRPC.read(bs)) {

            bool serialIsInvalid = true;
            String serial;
            ttmath::UInt<100> serialNumber;
            ttmath::uint remainder = 0;

            serialNumber.FromString(String(StringView(playerConnectRPC.Key)), 16);
            serialNumber.DivInt(1001, remainder);

            if (remainder == 0) {
                serial = serialNumber.ToString(16);
                if (serial.size() != 0 && serial.size() < 50) {
                    serialIsInvalid = false;
                }
            }

            const bool versionIsInvalid = (playerConnectRPC.VersionString.length() > 24);

            if (serialIsInvalid || versionIsInvalid) {
                PeerAddress address;
                address.v4 = rpcParams->sender.binaryAddress;
                address.ipv6 = false;

                PeerAddress::AddressString addressString;
                PeerAddress::ToString(address, addressString);

                network->core->logLn(LogLevel::Warning, "Invalid client connecting from %.*s", int(addressString.length()), addressString.data());
                network->rakNetServer.Kick(rpcParams->sender);
                return;
            }

            IPlayer* newPeer = network->OnPeerConnect(rpcParams, false, serial, playerConnectRPC.VersionNumber, playerConnectRPC.VersionString, playerConnectRPC.ChallengeResponse, playerConnectRPC.Name);
            if (newPeer) {
                if (!network->inEventDispatcher.stopAtFalse(
                        [newPeer, &bs](NetworkInEventHandler* handler) {
                            bs.resetReadPointer();
                            return handler->receivedRPC(*newPeer, NetCode::RPC::PlayerConnect::PacketID, bs);
                        })) {
                    return;
                }

                if (!network->rpcInEventDispatcher.stopAtFalse(
                        NetCode::RPC::PlayerConnect::PacketID,
                        [newPeer, &bs](SingleNetworkInEventHandler* handler) {
                            bs.resetReadPointer();
                            return handler->received(*newPeer, bs);
                        })) {
                    return;
                }

                network->networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, *newPeer);
                return;
            }
        }
    }

    network->rakNetServer.Kick(rpcParams->sender);
}

void RakNetLegacyNetwork::OnNPCConnect(RakNet::RPCParameters* rpcParams, void* extra)
{
    RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
    RakNet::RakPeer::RemoteSystemStruct* remoteSystem = network->rakNetServer.GetRemoteSystemFromPlayerID(rpcParams->sender);

    if (!remoteSystem || remoteSystem->connectMode != RakNet::RakPeer::RemoteSystemStruct::ConnectMode::CONNECTED) {
        return;
    }

    if (remoteSystem->sampData.authType == SAMPRakNet::AuthType_NPC) {
        NetworkBitStream bs = GetBitStream(*rpcParams);
        NetCode::RPC::NPCConnect NPCConnectRPC;
        if (NPCConnectRPC.read(bs)) {
            IPlayer* newPeer = network->OnPeerConnect(rpcParams, true, "", NPCConnectRPC.VersionNumber, "npc", NPCConnectRPC.ChallengeResponse, NPCConnectRPC.Name);
            if (newPeer) {
                if (!network->inEventDispatcher.stopAtFalse(
                        [newPeer, &bs](NetworkInEventHandler* handler) {
                            bs.resetReadPointer();
                            return handler->receivedRPC(*newPeer, NetCode::RPC::NPCConnect::PacketID, bs);
                        })) {
                    return;
                }

                if (!network->rpcInEventDispatcher.stopAtFalse(
                        NetCode::RPC::NPCConnect::PacketID,
                        [newPeer, &bs](SingleNetworkInEventHandler* handler) {
                            bs.resetReadPointer();
                            return handler->received(*newPeer, bs);
                        })) {
                    return;
                }

                network->networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, *newPeer);
                return;
            }
        }
    }

    network->rakNetServer.Kick(rpcParams->sender);
}

void RakNetLegacyNetwork::OnRakNetDisconnect(RakNet::PlayerIndex rid, PeerDisconnectReason reason)
{
    IPlayer* player = playerFromRakIndex[rid];

    if (!player) {
        return;
    }

    playerFromRakIndex[rid] = nullptr;
    networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerDisconnect, *player, reason);
}

template <size_t ID>
void RakNetLegacyNetwork::RPCHook(RakNet::RPCParameters* rpcParams, void* extra)
{
    RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);

    if (rpcParams->senderIndex >= network->playerFromRakIndex.size()) {
        return;
    }

    IPlayer* player = network->playerFromRakIndex[rpcParams->senderIndex];

    if (player == nullptr) {
        return;
    }

    NetworkBitStream bs = GetBitStream(*rpcParams);

    if (!network->inEventDispatcher.stopAtFalse(
            [&player, &bs](NetworkInEventHandler* handler) {
                bs.resetReadPointer();
                return handler->receivedRPC(*player, ID, bs);
            })) {
        return;
    }

    if (!network->rpcInEventDispatcher.stopAtFalse(
            ID,
            [&player, &bs](SingleNetworkInEventHandler* handler) {
                bs.resetReadPointer();
                return handler->received(*player, bs);
            })) {
        return;
    }
#ifdef _DEBUG
    if (network->inEventDispatcher.count() == 0 && network->rpcInEventDispatcher.count(ID) == 0) {
        network->core->printLn("Received unprocessed RPC %zu", ID);
    }
#endif
}

void RakNetLegacyNetwork::ban(const BanEntry& entry, Milliseconds expire)
{
    // Only support ipv4
    if (entry.address != StringView("127.0.0.1")) {
        rakNetServer.AddToBanList(entry.address.data(), expire.count());
    }
}

void RakNetLegacyNetwork::unban(const BanEntry& entry)
{
    rakNetServer.RemoveFromBanList(entry.address.data());
}

NetworkStats RakNetLegacyNetwork::getStatistics(int playerIndex)
{
    NetworkStats stats = { 0 };

    RakNet::PlayerID playerID;
    IPlayer* player;

    if (playerIndex == -1) {
        playerID = RakNet::UNASSIGNED_PLAYER_ID;
    } else {
        playerID = rakNetServer.GetPlayerIDFromIndex(playerIndex);
        // Return empty statistics structure if passed index does not exist
        if (playerID == RakNet::UNASSIGNED_PLAYER_ID) {
            return stats;
        }

        player = core->getPlayers().get(playerIndex);
        // Return empty statistics structure if player is not found
        if (player == nullptr) {
            return stats;
        }
    }

    RakNet::RakNetStatisticsStruct* raknetStats = rakNetServer.GetStatistics(playerID);

    // Return empty statistics structure if raknet failed to provide statistics
    if (raknetStats == nullptr) {
        return stats;
    }

    RakNet::RakNetTime time = RakNet::GetTime();
    double elapsedTime;
    elapsedTime = (time - raknetStats->connectionStartTime) / 1000.0f;

    stats.connectionStartTime = raknetStats->connectionStartTime;
    stats.messageSendBuffer
        = raknetStats->messageSendBuffer[RakNet::SYSTEM_PRIORITY] + raknetStats->messageSendBuffer[RakNet::HIGH_PRIORITY] + raknetStats->messageSendBuffer[RakNet::MEDIUM_PRIORITY] + raknetStats->messageSendBuffer[RakNet::LOW_PRIORITY];

    stats.messagesSent
        = raknetStats->messagesSent[RakNet::SYSTEM_PRIORITY] + raknetStats->messagesSent[RakNet::HIGH_PRIORITY] + raknetStats->messagesSent[RakNet::MEDIUM_PRIORITY] + raknetStats->messagesSent[RakNet::LOW_PRIORITY];

    stats.totalBytesSent = BITS_TO_BYTES(raknetStats->totalBitsSent);
    stats.acknowlegementsSent = raknetStats->acknowlegementsSent;
    stats.acknowlegementsPending = raknetStats->acknowlegementsPending;
    stats.messagesOnResendQueue = raknetStats->messagesOnResendQueue;
    stats.messageResends = raknetStats->messageResends;
    stats.messagesTotalBytesResent = BITS_TO_BYTES(raknetStats->messagesTotalBitsResent);
    stats.packetloss = 100.0f * static_cast<float>(raknetStats->messagesTotalBitsResent) / static_cast<float>(raknetStats->totalBitsSent);

    stats.messagesReceived
        = raknetStats->duplicateMessagesReceived + raknetStats->invalidMessagesReceived + raknetStats->messagesReceived;

    stats.messagesReceivedPerSecond = stats.messagesReceived - raknetStats->perSecondReceivedMsgCount;
    stats.bytesReceived = BITS_TO_BYTES(raknetStats->bitsReceived + raknetStats->bitsWithBadCRCReceived);
    stats.acknowlegementsReceived = raknetStats->acknowlegementsReceived;
    stats.duplicateAcknowlegementsReceived = raknetStats->duplicateAcknowlegementsReceived;
    stats.bitsPerSecond = raknetStats->bitsPerSecond;
    stats.bpsSent = static_cast<double>(raknetStats->totalBitsSent) / elapsedTime;
    stats.bpsReceived = static_cast<double>(raknetStats->bitsReceived) / elapsedTime;

    stats.isActive = false;
    stats.connectMode = 0;

    if (playerID != RakNet::UNASSIGNED_PLAYER_ID) {
        RakNet::RakPeer::RemoteSystemStruct* remoteSystem = rakNetServer.GetRemoteSystemFromPlayerID(playerID);

        if (remoteSystem) {
            stats.isActive = remoteSystem->isActive;
            stats.connectMode = remoteSystem->connectMode;
        }
    }

    return stats;
}

void RakNetLegacyNetwork::update()
{
    IConfig& config = core->getConfig();

    cookieSeedTime = Milliseconds(*config.getInt("cookie_reseed_time"));

    SAMPRakNet::SetTimeout(*config.getInt("player_timeout"));
    SAMPRakNet::SetMinConnectionTime(*config.getInt("min_connection_time"));
    SAMPRakNet::SetMessagesLimit(*config.getInt("messages_limit"));
    SAMPRakNet::SetMessageHoleLimit(*config.getInt("message_hole_limit"));
    SAMPRakNet::SetAcksLimit(*config.getInt("acks_limit"));
    SAMPRakNet::SetNetworkLimitsBanTime(*config.getInt("network_limits_ban_time"));

    SAMPRakNet::SetLogCookies(*config.getInt("logging_cookies"));

    query.setLogQueries(*config.getInt("logging_queries"));
    if (*config.getInt("enable_query")) {
        SAMPRakNet::SetQuery(&query);
    }

    StringView hostName = config.getString("server_name");
    if (!hostName.empty()) {
        query.setServerName(hostName);
    }
    StringView language = config.getString("language");
    if (!language.empty()) {
        query.setRuleValue("language", String(language));
    }
    StringView modeName = config.getString("mode_name");
    if (!modeName.empty()) {
        query.setGameModeName(modeName);
    }
    StringView mapName = config.getString("map_name");
    if (!mapName.empty()) {
        query.setRuleValue("mapname", String(mapName));
    }
    StringView website = config.getString("website");
    if (!website.empty()) {
        query.setRuleValue("weburl", String(website));
    }

    StringView rconPassword = config.getString("rcon_password");
    query.setRconPassword(rconPassword);
    query.setRconEnabled(*config.getInt("enable_rcon"));

    StringView password = config.getString("password");
    query.setPassworded(!password.empty());
    rakNetServer.SetPassword(password.empty() ? 0 : password.data());

    query.buildConfigDependentBuffers();

    int mtu = *config.getInt("network_mtu");
    rakNetServer.SetMTUSize(mtu);
}

void RakNetLegacyNetwork::init(ICore* c)
{
    core = c;
    
    core->getEventDispatcher().addEventHandler(this);
    core->getPlayers().getEventDispatcher().addEventHandler(this);
}

void RakNetLegacyNetwork::start()
{
    SAMPRakNet::Init(core);
    SAMPRakNet::SeedToken();
    lastCookieSeed = Time::now();
    SAMPRakNet::SeedCookie();

    playerFromRakIndex.fill(nullptr);

    IConfig& config = core->getConfig();
    int maxPlayers = *config.getInt("max_players");

    int port = *config.getInt("port");
    int sleep = *config.getInt("sleep");
    StringView bind = config.getString("bind");

    query.setCore(core);
    query.setRuleValue("version", "0.3.7-R2 open.mp");
    query.setMaxPlayers(maxPlayers);
    query.buildPlayerDependentBuffers();

    update();

    if (*config.getInt("announce")) {
        const String get = "/0.3.7/announce/" + std::to_string(port);
        {
            httplib::Client request("https://api.open.mp");
            request.enable_server_certificate_verification(true);
            request.set_follow_location(true);
            request.set_connection_timeout(Seconds(5));
            request.set_read_timeout(Seconds(5));
            request.set_write_timeout(Seconds(5));
            const httplib::Result res = request.Get(get.c_str());
            if (!res || res.value().status != 200) {
                core->printLn("Failed to announce legacy network to open.mp list");
            }
        }
    }

    for (size_t i = 0; i < config.getBansCount(); ++i) {
        ban(config.getBan(i));
    }

    if (!rakNetServer.Start(maxPlayers, 0, sleep, port, bind.data())) {
        if (!bind.empty()) {
            core->logLn(LogLevel::Error, "Unable to start legacy network on %.*s:%d. Port in use?.", PRINT_VIEW(bind), port);
        } else {
            core->logLn(LogLevel::Error, "Unable to start legacy network on port %d. Port in use?", port);
        }
    } else {
        if (!bind.empty()) {
            core->logLn(LogLevel::Message, "Legacy Network started on %.*s:%d.", PRINT_VIEW(bind), port);
        } else {
            core->logLn(LogLevel::Message, "Legacy Network started on port %d", port);
        }
    }

    rakNetServer.StartOccasionalPing();
    SAMPRakNet::SetPort(port);
}

void RakNetLegacyNetwork::onTick(Microseconds elapsed, TimePoint now)
{
    for (RakNet::Packet* pkt = rakNetServer.Receive(); pkt; pkt = rakNetServer.Receive()) {

        if (pkt->playerIndex >= playerFromRakIndex.size()) {
            continue;
        }

        IPlayer* player = playerFromRakIndex[pkt->playerIndex];

        if (player) {
            NetworkBitStream bs(pkt->data, pkt->length, false);
            uint8_t type;
            if (bs.readUINT8(type)) {
                const bool res = inEventDispatcher.stopAtFalse([&player, type, &bs](NetworkInEventHandler* handler) {
                    bs.SetReadOffset(8); // Ignore packet ID
                    return handler->receivedPacket(*player, type, bs);
                });

                if (res) {
                    packetInEventDispatcher.stopAtFalse(type, [&player, &bs](SingleNetworkInEventHandler* handler) {
                        bs.SetReadOffset(8); // Ignore packet ID
                        return handler->received(*player, bs);
                    });
                }

                if (type == RakNet::ID_DISCONNECTION_NOTIFICATION) {
                    OnRakNetDisconnect(pkt->playerIndex, PeerDisconnectReason_Quit);
                } else if (type == RakNet::ID_CONNECTION_LOST) {
                    OnRakNetDisconnect(pkt->playerIndex, PeerDisconnectReason_Timeout);
                }
            }
        }
        rakNetServer.DeallocatePacket(pkt);
    }

    if (now - lastCookieSeed > cookieSeedTime) {
        SAMPRakNet::SeedCookie();
        lastCookieSeed = now;
    }
}
