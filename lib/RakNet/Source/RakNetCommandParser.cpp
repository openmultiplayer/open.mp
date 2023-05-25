#include "RakNetCommandParser.h"
#include "TransportInterface.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning( push )
#endif

RakNetCommandParser::RakNetCommandParser()
{
	RegisterCommand(4, "Initialize","( unsigned short maxConnections, unsigned short localPort, int _threadSleepTimer, const char *forceHostAddress );");
	RegisterCommand(0,"InitializeSecurity","();");
	RegisterCommand(0,"DisableSecurity","( void );");
	RegisterCommand(1,"SetMaximumIncomingConnections","( unsigned short numberAllowed );");
	RegisterCommand(0,"GetMaximumIncomingConnections","( void ) const;");
	RegisterCommand(4,"Connect","( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength );");
	RegisterCommand(2,"Disconnect","( unsigned int blockDuration, unsigned char orderingChannel=0 );");
	RegisterCommand(0,"IsActive","( void ) const;");
	RegisterCommand(0,"GetConnectionList","() const;");
	RegisterCommand(4,"CloseConnection","( const PlayerID target, bool sendDisconnectionNotification, unsigned char orderingChannel=0 );");
	RegisterCommand(2,"GetIndexFromPlayerID","( const PlayerID playerId );");
	RegisterCommand(1,"GetPlayerIDFromIndex","( int index );");
	RegisterCommand(2,"AddToBanList","( const char *IP, RakNetTime milliseconds=0 );");
	RegisterCommand(1,"RemoveFromBanList","( const char *IP );");
	RegisterCommand(0,"ClearBanList","( void );");
	RegisterCommand(1,"IsBanned","( const char *IP );");
	RegisterCommand(2,"Ping1","( const PlayerID target );");
	RegisterCommand(3,"Ping2","( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections );");
	RegisterCommand(2,"GetAveragePing","( const PlayerID playerId );");
	RegisterCommand(2,"GetLastPing","( const PlayerID playerId ) const;");
	RegisterCommand(2,"GetLowestPing","( const PlayerID playerId ) const;");
	RegisterCommand(1,"SetOccasionalPing","( bool doPing );");
	RegisterCommand(2,"SetOfflinePingResponse","( const char *data, const unsigned int length );");
	RegisterCommand(0,"GetInternalID","( void ) const;");
	RegisterCommand(2,"GetExternalID","( const PlayerID target ) const;");
	RegisterCommand(3,"SetTimeoutTime","( RakNetTime timeMS, const PlayerID target );");
	RegisterCommand(1,"SetMTUSize","( int size );");
	RegisterCommand(0,"GetMTUSize","( void ) const;");
	RegisterCommand(0,"GetNumberOfAddresses","( void );");
	RegisterCommand(2,"PlayerIDToDottedIP","( const PlayerID playerId ) const;");
	RegisterCommand(2,"IPToPlayerID","( const char* host, unsigned short remotePort );");
	RegisterCommand(1,"GetLocalIP","( unsigned int index );");
	RegisterCommand(1,"AllowConnectionResponseIPMigration","( bool allow );");
	RegisterCommand(4,"AdvertiseSystem","( const char *host, unsigned short remotePort, const char *data, int dataLength );");
	RegisterCommand(2,"SetIncomingPassword","( const char* passwordData, int passwordDataLength );");
	RegisterCommand(0,"GetIncomingPassword","( void );");
}
RakNetCommandParser::~RakNetCommandParser()
{
}
void RakNetCommandParser::SetRakPeerInterface(RakPeerInterface *rakPeer)
{
	peer=rakPeer;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
bool RakNetCommandParser::OnCommand(const char *command, unsigned numParameters, char **parameterList, TransportInterface *transport, PlayerID playerId, const char *originalString)
{
	if (peer==0)
		return false;

	if (strcmp(command, "Initialize")==0)
	{
		ReturnResult(peer->Initialize((unsigned short)atoi(parameterList[0]), (unsigned short)atoi(parameterList[1]),atoi(parameterList[2]),parameterList[3]), command, transport, playerId);
	}
	else if (strcmp(command, "InitializeSecurity")==0)
	{
		peer->InitializeSecurity(0,0,0,0);
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "DisableSecurity")==0)
	{
		peer->DisableSecurity();
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "SetMaximumIncomingConnections")==0)
	{
		peer->SetMaximumIncomingConnections((unsigned short)atoi(parameterList[0]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "GetMaximumIncomingConnections")==0)
	{
		ReturnResult(peer->GetMaximumIncomingConnections(), command, transport, playerId);
	}
	else if (strcmp(command, "Connect")==0)
	{
		ReturnResult(peer->Connect(parameterList[0], (unsigned short)atoi(parameterList[1]),parameterList[2],atoi(parameterList[3])), command, transport, playerId);
	}
	else if (strcmp(command, "Disconnect")==0)
	{
		peer->Disconnect(atoi(parameterList[0]), (unsigned char)atoi(parameterList[1]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "IsActive")==0)
	{
		ReturnResult(peer->IsActive(), command, transport, playerId);
	}
	else if (strcmp(command, "GetConnectionList")==0)
	{
		PlayerID remoteSystems[32];
		unsigned short count=32;
		unsigned i;
		if (peer->GetConnectionList(remoteSystems, &count))
		{
			if (count==0)
			{
				transport->Send(playerId, "GetConnectionList() returned no systems connected.\r\n");
			}
			else
			{
				transport->Send(playerId, "GetConnectionList() returned:\r\n");
				for (i=0; i < count; i++)
					transport->Send(playerId, "%i %s %i:%i\r\n", i, peer->PlayerIDToDottedIP(remoteSystems[i]), remoteSystems[i].binaryAddress, remoteSystems[i].port);
			}
		}
		else
			transport->Send(playerId, "GetConnectionList() returned false.\r\n");
	}
	else if (strcmp(command, "CloseConnection")==0)
	{
		peer->CloseConnection(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1])),atoi(parameterList[2])!=0,(unsigned char)atoi(parameterList[3]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "GetIndexFromPlayerID")==0)
	{
		ReturnResult(peer->GetIndexFromPlayerID(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "GetPlayerIDFromIndex")==0)
	{
		ReturnResult(peer->GetPlayerIDFromIndex(atoi(parameterList[0])), command, transport, playerId);
	}
	else if (strcmp(command, "AddToBanList")==0)
	{
		peer->AddToBanList(parameterList[0], atoi(parameterList[1]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "RemoveFromBanList")==0)
	{
		peer->RemoveFromBanList(parameterList[0]);
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "ClearBanList")==0)
	{
		peer->ClearBanList();
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "IsBanned")==0)
	{
		ReturnResult(peer->IsBanned(parameterList[0]), command, transport, playerId);
	}
	else if (strcmp(command, "Ping1")==0)
	{
		peer->Ping(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1])));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "Ping2")==0)
	{
		peer->Ping(parameterList[0], (unsigned short) atoi(parameterList[1]), atoi(parameterList[2])!=0);
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "GetAveragePing")==0)
	{
		ReturnResult(peer->GetAveragePing(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "GetLastPing")==0)
	{
		ReturnResult(peer->GetLastPing(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "GetLowestPing")==0)
	{
		ReturnResult(peer->GetLowestPing(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "SetOccasionalPing")==0)
	{
		peer->SetOccasionalPing(atoi(parameterList[0])!=0);
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "SetOfflinePingResponse")==0)
	{
		peer->SetOfflinePingResponse(parameterList[0], atoi(parameterList[1]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "GetInternalID")==0)
	{
		ReturnResult(peer->GetInternalID(), command, transport, playerId);
	}
	else if (strcmp(command, "GetExternalID")==0)
	{
		ReturnResult(peer->GetExternalID(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "SetTimeoutTime")==0)
	{
		peer->SetTimeoutTime(atoi(parameterList[0]), IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1])));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "SetMTUSize")==0)
	{
		ReturnResult(peer->SetMTUSize(atoi(parameterList[0])), command, transport, playerId);
	}
	else if (strcmp(command, "GetMTUSize")==0)
	{
		ReturnResult(peer->GetMTUSize(), command, transport, playerId);
	}
	else if (strcmp(command, "GetNumberOfAddresses")==0)
	{
		ReturnResult((int)peer->GetNumberOfAddresses(), command, transport, playerId);
	}
	else if (strcmp(command, "PlayerIDToDottedIP")==0)
	{
		ReturnResult((char*)peer->PlayerIDToDottedIP(IntegersToPlayerID(atoi(parameterList[0]), atoi(parameterList[1]))), command, transport, playerId);
	}
	else if (strcmp(command, "IPToPlayerID")==0)
	{
		PlayerID output;
		peer->IPToPlayerID(parameterList[0],(unsigned short) atoi(parameterList[1]),&output);
		if (output==UNASSIGNED_PLAYER_ID)
		{
			transport->Send(playerId, "IPToPlayerID(): UNASSIGNED_PLAYER_ID.\r\n");
		}
		else
		{
			transport->Send(playerId, "IPToPlayerID(): %s %i:%i\r\n", peer->PlayerIDToDottedIP(output), output.binaryAddress, output.port);
		}
	}
	else if (strcmp(command, "GetLocalIP")==0)
	{
		ReturnResult((char*) peer->GetLocalIP(atoi(parameterList[0])), command, transport, playerId);
	}
	else if (strcmp(command, "AllowConnectionResponseIPMigration")==0)
	{
		peer->AllowConnectionResponseIPMigration(atoi(parameterList[0])!=0);
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "AdvertiseSystem")==0)
	{
		peer->AdvertiseSystem(parameterList[0], (unsigned short) atoi(parameterList[1]),parameterList[2],atoi(parameterList[3]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "SetIncomingPassword")==0)
	{
		peer->SetIncomingPassword(parameterList[0], atoi(parameterList[1]));
		ReturnResult(command, transport, playerId);
	}
	else if (strcmp(command, "GetIncomingPassword")==0)
	{
		char password[256];
		int passwordLength;
		peer->GetIncomingPassword(password, &passwordLength);
		if (passwordLength)
			ReturnResult((char*)password, command, transport, playerId);
		else
			ReturnResult(0, command, transport, playerId);
	}

	return true;
}
char *RakNetCommandParser::GetName(void) const
{
	return (char *)"RakNet";
}
void RakNetCommandParser::SendHelp(TransportInterface *transport, PlayerID playerId)
{
	if (peer)
	{
		transport->Send(playerId, "The RakNet parser provides mirror functions to RakPeer\r\n");
		transport->Send(playerId, "PlayerIDs take two parameters: send <BinaryAddress> <Port>.\r\n");
		transport->Send(playerId, "For bool, send 1 or 0.\r\n");
	}
	else
	{
		transport->Send(playerId, "Parser not active.  Call SetRakPeerInterface.\r\n");
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
