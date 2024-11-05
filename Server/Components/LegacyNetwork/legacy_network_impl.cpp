/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "legacy_network_impl.hpp"
#include "Query/query.hpp"
#include "raknet/../../SAMPRakNet.hpp"
#include <sstream>
#include <netcode.hpp>
#include <raknet/PacketEnumerations.h>
#include <raknet/RakPeer.h>
#include <ttmath/ttmath.h>

#define RPCHOOK(id) rakNetServer.RegisterAsRemoteProcedureCall(id, &RakNetLegacyNetwork::RPCHook<id>, this)

RakNetLegacyNetwork::RakNetLegacyNetwork()
	: Network(256, 256)
	, core(nullptr)
	, rakNetServer(*RakNet::RakNetworkFactory::GetRakServerInterface())
{
	rakNetServer.SetMTUSize(512);
	playerRemoteSystem.fill(nullptr);

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
	if (core)
	{
		core->getEventDispatcher().removeEventHandler(this);
		core->getPlayers().getPlayerChangeDispatcher().removeEventHandler(this);
		core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
	}
	rakNetServer.Disconnect(300);
	RakNet::RakNetworkFactory::DestroyRakServerInterface(&rakNetServer);
}

static NetworkBitStream GetBitStream(RakNet::RPCParameters& rpcParams)
{
	const unsigned int
		bits
		= rpcParams.numberOfBitsOfData;
	if (bits == 0)
		return NetworkBitStream();
	NetworkBitStream bs(rpcParams.input, bitsToBytes(bits), false /* copyData */);
	bs.SetWriteOffset(bits);
	return bs;
}

enum LegacyClientVersion
{
	LegacyClientVersion_037 = 4057,
	LegacyClientVersion_03DL = 4062
};

IPlayer* RakNetLegacyNetwork::OnPeerConnect(RakNet::RPCParameters* rpcParams, bool isNPC, StringView serial, uint32_t version, StringView versionName, uint32_t challenge, StringView name, bool isUsingOfficialClient)
{
	const RakNet::PlayerID rid = rpcParams->sender;

	if (playerFromRakIndex[rpcParams->senderIndex])
	{
		// Connection already exists
		return nullptr;
	}

	PeerNetworkData netData {};
	netData.networkID.address.ipv6 = false;
	netData.networkID.address.v4 = rid.binaryAddress;
	netData.networkID.port = rid.port;
	netData.network = this;

	Pair<NewConnectionResult, IPlayer*> newConnectionResult { NewConnectionResult_Ignore, nullptr };

	const bool isDL = version == LegacyClientVersion_03DL && (SAMPRakNet::GetToken() == (challenge ^ LegacyClientVersion_03DL));
	static bool* artwork = core->getConfig().getBool("artwork.enable");
	static bool* allow037 = core->getConfig().getBool("network.allow_037_clients");

	if (isDL || ((!artwork || !*artwork || (*artwork && *allow037)) && (version == LegacyClientVersion_037 && (SAMPRakNet::GetToken() == (challenge ^ LegacyClientVersion_037)))))
	{
		PeerRequestParams params;
		params.version = version == LegacyClientVersion_037 ? ClientVersion::ClientVersion_SAMP_037 : ClientVersion::ClientVersion_SAMP_03DL;
		params.versionName = versionName;
		params.name = name;
		params.bot = isNPC;
		params.serial = serial;
		params.isUsingOfficialClient = isUsingOfficialClient;
		newConnectionResult = core->getPlayers().requestPlayer(netData, params);
	}
	else
	{
		newConnectionResult.first = NewConnectionResult_VersionMismatch;
	}

	if (newConnectionResult.first != NewConnectionResult_Success)
	{
		if (newConnectionResult.first != NewConnectionResult_Ignore)
		{
			// Entry denied, send reason and disconnect
			RakNet::BitStream bss;
			bss.Write(uint8_t(newConnectionResult.first));
			rakNetServer.RPC(130, &bss, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
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

	if (!remoteSystem || remoteSystem->connectMode != RakNet::RakPeer::RemoteSystemStruct::ConnectMode::CONNECTED)
	{
		return;
	}

	if (remoteSystem->sampData.authType != SAMPRakNet::AuthType_Player)
	{
		network->rakNetServer.Kick(rpcParams->sender);
		return;
	}

	NetworkBitStream bs = GetBitStream(*rpcParams);
	NetCode::RPC::PlayerConnect playerConnectRPC;
	if (!playerConnectRPC.read(bs))
	{
		network->rakNetServer.Kick(rpcParams->sender);
		return;
	}

	bool serialIsInvalid = true;
	String serial;
	ttmath::UInt<100> serialNumber;
	ttmath::uint remainder = 0;

	serialNumber.FromString(String(StringView(playerConnectRPC.Key)), 16);
	serialNumber.DivInt(1001, remainder);

	if (remainder == 0)
	{
		serial = serialNumber.ToString(16);
		if (serial.size() != 0 && serial.size() < 50)
		{
			serialIsInvalid = false;
		}
	}

	const bool versionIsInvalid = (playerConnectRPC.VersionString.length() > 24);

	if (serialIsInvalid || versionIsInvalid)
	{
		PeerAddress address;
		address.v4 = rpcParams->sender.binaryAddress;
		address.ipv6 = false;

		PeerAddress::AddressString addressString;
		PeerAddress::ToString(address, addressString);

		network->core->logLn(LogLevel::Warning, "Invalid client connecting from %.*s", int(addressString.length()), addressString.data());
		network->rakNetServer.Kick(rpcParams->sender);
		return;
	}

	IPlayer* newPeer = network->OnPeerConnect(rpcParams, false, serial, playerConnectRPC.VersionNumber, playerConnectRPC.VersionString, playerConnectRPC.ChallengeResponse, playerConnectRPC.Name, playerConnectRPC.IsUsingOfficialClient);
	if (!newPeer)
	{
		network->rakNetServer.Kick(rpcParams->sender);
		return;
	}

	if (!network->inEventDispatcher.stopAtFalse(
			[newPeer, &bs](NetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceiveRPC(*newPeer, NetCode::RPC::PlayerConnect::PacketID, bs);
			}))
	{
		return;
	}

	if (!network->rpcInEventDispatcher.stopAtFalse(
			NetCode::RPC::PlayerConnect::PacketID,
			[newPeer, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceive(*newPeer, bs);
			}))
	{
		return;
	}

	network->networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, *newPeer);
	network->playerRemoteSystem[newPeer->getID()] = remoteSystem;
	remoteSystem->isLogon = true;
}

void RakNetLegacyNetwork::OnNPCConnect(RakNet::RPCParameters* rpcParams, void* extra)
{
	RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
	RakNet::RakPeer::RemoteSystemStruct* remoteSystem = network->rakNetServer.GetRemoteSystemFromPlayerID(rpcParams->sender);

	if (!remoteSystem || remoteSystem->connectMode != RakNet::RakPeer::RemoteSystemStruct::ConnectMode::CONNECTED)
	{
		return;
	}

	if (remoteSystem->sampData.authType == SAMPRakNet::AuthType_NPC)
	{
		NetworkBitStream bs = GetBitStream(*rpcParams);
		NetCode::RPC::NPCConnect NPCConnectRPC;
		if (NPCConnectRPC.read(bs))
		{
			IPlayer* newPeer = network->OnPeerConnect(rpcParams, true, "", NPCConnectRPC.VersionNumber, "npc", NPCConnectRPC.ChallengeResponse, NPCConnectRPC.Name);
			if (newPeer)
			{
				if (!network->inEventDispatcher.stopAtFalse(
						[newPeer, &bs](NetworkInEventHandler* handler)
						{
							bs.resetReadPointer();
							return handler->onReceiveRPC(*newPeer, NetCode::RPC::NPCConnect::PacketID, bs);
						}))
				{
					return;
				}

				if (!network->rpcInEventDispatcher.stopAtFalse(
						NetCode::RPC::NPCConnect::PacketID,
						[newPeer, &bs](SingleNetworkInEventHandler* handler)
						{
							bs.resetReadPointer();
							return handler->onReceive(*newPeer, bs);
						}))
				{
					return;
				}

				network->networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, *newPeer);
				remoteSystem->isLogon = true;
			}
		}
	}
}

void RakNetLegacyNetwork::OnRakNetDisconnect(RakNet::PlayerIndex rid, PeerDisconnectReason reason)
{
	IPlayer* player = playerFromRakIndex[rid];

	if (!player)
	{
		return;
	}

	playerFromRakIndex[rid] = nullptr;
	playerRemoteSystem[player->getID()] = nullptr;
	networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerDisconnect, *player, reason);
}

template <size_t ID>
void RakNetLegacyNetwork::RPCHook(RakNet::RPCParameters* rpcParams, void* extra)
{
	RakNetLegacyNetwork* network = reinterpret_cast<RakNetLegacyNetwork*>(extra);
	const RakNet::PlayerIndex senderId = rpcParams->senderIndex;

	if (senderId >= network->playerFromRakIndex.size())
	{
		return;
	}

	IPlayer* player = network->playerFromRakIndex[senderId];

	if (player == nullptr)
	{
		return;
	}

	NetworkBitStream bs = GetBitStream(*rpcParams);

	if (!network->inEventDispatcher.stopAtFalse(
			[&player, &bs](NetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceiveRPC(*player, ID, bs);
			}))
	{
		return;
	}

	if (!network->rpcInEventDispatcher.stopAtFalse(
			ID,
			[&player, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceive(*player, bs);
			}))
	{
		return;
	}
#ifdef _DEBUG
	if (network->inEventDispatcher.count() == 0 && network->rpcInEventDispatcher.count(ID) == 0)
	{
		network->core->logLn(LogLevel::Debug, "Received unprocessed RPC %zu", ID);
	}
#endif
}

void RakNetLegacyNetwork::synchronizeBans()
{
	char addr[22] = { 0 };
	unsigned short port;
	for (IPlayer* player : core->getPlayers().entries())
	{
		const PeerNetworkData& netData = player->getNetworkData();
		if (netData.network != this)
		{
			continue;
		}

		const PeerNetworkData::NetworkID& nid = netData.networkID;
		const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
		rakNetServer.GetPlayerIPFromID(rid, addr, &port);
		if (rakNetServer.IsBanned(addr))
		{
			player->kick();
		}
	}
}

void RakNetLegacyNetwork::ban(const BanEntry& entry, Milliseconds expire)
{
	// Only support ipv4
	if (entry.address != StringView("127.0.0.1"))
	{
		rakNetServer.AddToBanList(entry.address.data(), expire.count());
		synchronizeBans();
	}
}

void RakNetLegacyNetwork::unban(const BanEntry& entry)
{
	rakNetServer.RemoveFromBanList(entry.address.data());
}

NetworkStats RakNetLegacyNetwork::getStatistics(IPlayer* player)
{
	NetworkStats stats = { 0 };
	RakNet::PlayerID playerID;

	if (player == nullptr)
	{
		playerID = RakNet::UNASSIGNED_PLAYER_ID;
	}
	else
	{
		const PeerNetworkData& netData = player->getNetworkData();

		// Return empty statistics structure if player is not connected to this network
		if (netData.network != this)
		{
			return stats;
		}
		else
		{
			const PeerNetworkData::NetworkID& nid = netData.networkID;
			playerID = { unsigned(nid.address.v4), nid.port };
		}
	}

	RakNet::RakNetStatisticsStruct* raknetStats = rakNetServer.GetStatistics(playerID);

	// Return empty statistics structure if raknet failed to provide statistics
	if (raknetStats == nullptr)
	{
		return stats;
	}

	RakNet::RakNetTime time = RakNet::GetTime();

	stats.connectionStartTime = raknetStats->connectionStartTime;
	stats.connectionElapsedTime = time - raknetStats->connectionStartTime;

	double elapsedTime = stats.connectionElapsedTime / 1000.0f;

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

	if (raknetStats->totalBitsSent)
		stats.packetloss = 100.0f * raknetStats->messagesTotalBitsResent / raknetStats->totalBitsSent;
	else
		stats.packetloss = 0.0f;

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

	if (playerID != RakNet::UNASSIGNED_PLAYER_ID)
	{
		RakNet::RakPeer::RemoteSystemStruct* remoteSystem = rakNetServer.GetRemoteSystemFromPlayerID(playerID);

		if (remoteSystem)
		{
			stats.isActive = remoteSystem->isActive;
			stats.connectMode = remoteSystem->connectMode;
		}
	}

	return stats;
}

void RakNetLegacyNetwork::update()
{
	IConfig& config = core->getConfig();

	cookieSeedTime = Milliseconds(*config.getInt("network.cookie_reseed_time"));

	SAMPRakNet::SetTimeout(*config.getInt("network.player_timeout"));
	SAMPRakNet::SetMinConnectionTime(*config.getInt("network.minimum_connection_time"));
	SAMPRakNet::SetMessagesLimit(*config.getInt("network.messages_limit"));
	SAMPRakNet::SetMessageHoleLimit(*config.getInt("network.message_hole_limit"));
	SAMPRakNet::SetAcksLimit(*config.getInt("network.acks_limit"));
	SAMPRakNet::SetNetworkLimitsBanTime(*config.getInt("network.limits_ban_time"));

	SAMPRakNet::SetLogCookies(*config.getBool("logging.log_cookies"));

	query.setLogQueries(*config.getBool("logging.log_queries"));
	if (*config.getBool("enable_query"))
	{
		SAMPRakNet::SetQuery(&query);
	}

	StringView hostName = config.getString("name");
	if (!hostName.empty())
	{
		query.setServerName(hostName);
	}

	query.setRuleValue<false>("lagcomp", (*config.getInt("game.lag_compensation_mode") != LagCompMode_Disabled) ? "On" : "Off");

	StringView language = config.getString("language");
	if (!language.empty())
	{
		query.setLanguage(String(language));
	}

	StringView modeName = config.getString("game.mode");
	if (!modeName.empty())
	{
		query.setGameModeName(modeName);
	}

	StringView mapName = config.getString("game.map");
	if (!mapName.empty())
	{
		query.setRuleValue<false>("mapname", String(mapName));
	}
	else
	{
		query.setRuleValue<false>("mapname", "San Andreas");
	}

	query.setRuleValue<false>("weather", std::to_string(*config.getInt("game.weather")));

	StringView website = config.getString("website");
	if (!website.empty())
	{
		query.setRuleValue<false>("weburl", String(website));
	}

	StringView discordLink = config.getString("discord.invite");
	if (!discordLink.empty())
	{
		query.setDiscordLink(discordLink);
	}

	StringView bannerUrl = config.getString("banners.light");
	if (!bannerUrl.empty())
	{
		query.setLightBannerUrl(bannerUrl);
	}

	bannerUrl = config.getString("banners.dark");
	if (!bannerUrl.empty())
	{
		query.setDarkBannerUrl(bannerUrl);
	}

	StringView logoUrl = config.getString("logo");
	if (!logoUrl.empty())
	{
		query.setLogoUrl(logoUrl);
	}

	query.setRuleValue<false>("worldtime", String(std::to_string(*config.getInt("game.time")) + ":00"));

	StringView rconPassword = config.getString("rcon.password");
	query.setRconPassword(rconPassword);
	query.setRconEnabled(*config.getBool("rcon.enable"));

	StringView password = config.getString("password");
	query.setPassworded(!password.empty());
	rakNetServer.SetPassword(password.empty() ? 0 : password.data());

	query.buildConfigDependentBuffers();

	int mtu = *config.getInt("network.mtu");
	rakNetServer.SetMTUSize(mtu);
}

void RakNetLegacyNetwork::init(ICore* c)
{
	core = c;

	core->getEventDispatcher().addEventHandler(this);
	core->getPlayers().getPlayerChangeDispatcher().addEventHandler(this);
	core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this, EventPriority_Lowest);
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

	int port = *config.getInt("network.port");
	int sleep = static_cast<int>(*config.getFloat("sleep"));
	StringView bind = config.getString("network.bind");

	bool* artwork_config = config.getBool("artwork.enable");
	bool artwork = !artwork_config ? false : *artwork_config;
	bool allow037 = *config.getBool("network.allow_037_clients");

	query.setCore(core);

	std::stringstream version;
	version << "omp " << OMP_VERSION_MAJOR << "." << OMP_VERSION_MINOR << "." << OMP_VERSION_PATCH << "." << BUILD_NUMBER;
	query.setRuleValue<false>("version", version.str());
	// Send as a rule to announce which clients are allowed to join
	query.setRuleValue<false>("allowed_clients", !allow037 ? "0.3.DL" : "0.3.7, 0.3.DL");

	query.setRuleValue<false>("artwork", artwork ? "Yes" : "No");

	query.setMaxPlayers(maxPlayers);
	query.buildPlayerDependentBuffers();

	update();

	for (size_t i = 0; i < config.getBansCount(); ++i)
	{
		ban(config.getBan(i));
	}

	if (!rakNetServer.Start(maxPlayers, 0, sleep, port, bind.data()))
	{
		if (!bind.empty())
		{
			core->logLn(LogLevel::Error, "Unable to start legacy network on %.*s:%d. Port in use?", PRINT_VIEW(bind), port);
		}
		else
		{
			core->logLn(LogLevel::Error, "Unable to start legacy network on port %d. Port in use?", port);
		}
	}
	else
	{
		if (!bind.empty())
		{
			core->logLn(LogLevel::Message, "Legacy Network started on %.*s:%d.", PRINT_VIEW(bind), port);
		}
		else
		{
			core->logLn(LogLevel::Message, "Legacy Network started on port %d", port);
		}

		// Do the request after network is started.
		if (*config.getBool("announce"))
		{
			const String get = "https://api.open.mp/0.3.7/announce/" + std::to_string(port);
			core->requestHTTP4(new AnnounceHTTPResponseHandler(core), HTTPRequestType::HTTPRequestType_Get, get.data());
		}
	}

	rakNetServer.StartOccasionalPing();
	SAMPRakNet::SetPort(port);

	int* gracePeriod = config.getInt("network.grace_period");

	if (gracePeriod)
	{
		SAMPRakNet::SetGracePeriod(*gracePeriod);
	}
}

void RakNetLegacyNetwork::onTick(Microseconds elapsed, TimePoint now)
{
	for (RakNet::Packet* pkt = rakNetServer.Receive(); pkt; pkt = rakNetServer.Receive())
	{
		if (pkt->playerIndex >= playerFromRakIndex.size())
		{
			rakNetServer.DeallocatePacket(pkt);
			continue;
		}

		IPlayer* player = playerFromRakIndex[pkt->playerIndex];

		// We shouldn't be needing this, it's only here IF somehow this is happening again
		// So users can report it to us
		if (player && player->getID() == -1)
		{
			rakNetServer.Kick(pkt->playerId);
			playerFromRakIndex[pkt->playerIndex] = nullptr;
			core->logLn(LogLevel::Warning, "RakNet player %d with open.mp player pool id -1  was found and deleted. Packet ID: %d", pkt->playerIndex, pkt->data[0]);
			core->logLn(LogLevel::Warning, "Please contact us by creating an issue in our repository at https://github.com/openmultiplayer/open.mp");
			rakNetServer.DeallocatePacket(pkt);
			continue;
		}

		if (player)
		{
			const unsigned int bits = pkt->bitSize;
			NetworkBitStream bs(pkt->data, bitsToBytes(bits), false);
			bs.SetWriteOffset(bits);
			uint8_t type;
			if (bs.readUINT8(type))
			{
				// Call event handlers for packet receive
				const bool res = inEventDispatcher.stopAtFalse([&player, type, &bs](NetworkInEventHandler* handler)
					{
						bs.SetReadOffset(8); // Ignore packet ID
						return handler->onReceivePacket(*player, type, bs);
					});

				if (res)
				{
					packetInEventDispatcher.stopAtFalse(type, [&player, &bs](SingleNetworkInEventHandler* handler)
						{
							bs.SetReadOffset(8); // Ignore packet ID
							return handler->onReceive(*player, bs);
						});
				}

				if (type == RakNet::ID_DISCONNECTION_NOTIFICATION)
				{
					OnRakNetDisconnect(pkt->playerIndex, PeerDisconnectReason_Quit);
				}
				else if (type == RakNet::ID_CONNECTION_LOST)
				{
					OnRakNetDisconnect(pkt->playerIndex, PeerDisconnectReason_Timeout);
				}
			}
		}

		rakNetServer.DeallocatePacket(pkt);
	}

	if (now - lastCookieSeed > cookieSeedTime)
	{
		SAMPRakNet::SeedCookie();
		lastCookieSeed = now;
	}
}
