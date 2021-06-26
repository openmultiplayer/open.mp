#include "CommandParserInterface.h"
#include "TransportInterface.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h"
#elif defined(_WIN32)
// IP_DONTFRAGMENT is different between winsock 1 and winsock 2.  Therefore, Winsock2.h must be linked againt Ws2_32.lib
// winsock.h must be linked against WSock32.lib.  If these two are mixed up the flag won't work correctly
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#if (defined(__GNUC__)  || defined(__GCCXML__)) && !defined(_stricmp)
#define _stricmp strcasecmp
#endif

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning( push )
#endif

const unsigned char CommandParserInterface::VARIABLE_NUMBER_OF_PARAMETERS=255;

namespace RakNet
{
	int RegisteredCommandComp( const char* const & key, const RegisteredCommand &data )
	{
		return _stricmp(key,data.command);
	}
}

CommandParserInterface::CommandParserInterface() {}
CommandParserInterface::~CommandParserInterface() {}

void CommandParserInterface::ParseConsoleString(char *str, const char delineator, unsigned char delineatorToggle, unsigned *numParameters, char **parameterList, unsigned parameterListLength)
{
	unsigned strIndex, parameterListIndex;
	unsigned strLen;
	bool replaceDelineator=true;

	strLen = (unsigned) strlen(str);

	// Replace every instance of delineator, \n, \r with 0
	for (strIndex=0; strIndex < strLen; strIndex++)
	{
		if (str[strIndex]==delineator && replaceDelineator)
			str[strIndex]=0;

		if (str[strIndex]=='\n' || str[strIndex]=='\r')
			str[strIndex]=0;

		if (str[strIndex]==delineatorToggle)
		{
			str[strIndex]=0;
			replaceDelineator=!replaceDelineator;
		}
	}

	// Fill up parameterList starting at each non-0
	for (strIndex=0, parameterListIndex=0; strIndex < strLen; )
	{
		if (str[strIndex]!=0)
		{
			parameterList[parameterListIndex]=str+strIndex;
			parameterListIndex++;
			RakAssert(parameterListIndex < parameterListLength);
			if (parameterListIndex >= parameterListLength)
				break;

			strIndex++;
			while (str[strIndex]!=0 && strIndex < strLen)
				strIndex++;
		}
		else
			strIndex++;
	}

	parameterList[parameterListIndex]=0;
	*numParameters=parameterListIndex;
}
void CommandParserInterface::SendCommandList(TransportInterface *transport, PlayerID playerId)
{
	unsigned i;
	if (commandList.Size())
	{
		for (i=0; i < commandList.Size(); i++)
		{
			transport->Send(playerId, "%s", commandList[i].command);
			if (i < commandList.Size()-1)
				transport->Send(playerId, ", ");
		}
		transport->Send(playerId, "\r\n");
	}
	else
		transport->Send(playerId, "No registered commands\r\n");
}
void CommandParserInterface::RegisterCommand(unsigned char parameterCount, const char *command, const char *commandHelp)
{
	RegisteredCommand rc;
	rc.command=command;
	rc.commandHelp=commandHelp;
	rc.parameterCount=parameterCount;
	commandList.Insert( command, rc);
}
bool CommandParserInterface::GetRegisteredCommand(const char *command, RegisteredCommand *rc)
{
	bool objectExists;
	unsigned index;
	index=commandList.GetIndexFromKey(command, &objectExists);
	if (objectExists)
		*rc=commandList[index];
	return objectExists;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void CommandParserInterface::OnTransportChange(TransportInterface *transport)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void CommandParserInterface::OnNewIncomingConnection(PlayerID playerId, TransportInterface *transport)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void CommandParserInterface::OnConnectionLost(PlayerID playerId, TransportInterface *transport)
{
}
void CommandParserInterface::ReturnResult(bool res, const char *command,TransportInterface *transport, PlayerID playerId)
{
	if (res)
		transport->Send(playerId, "%s returned true.\r\n", command);
	else
		transport->Send(playerId, "%s returned false.\r\n", command);
}
void CommandParserInterface::ReturnResult(int res, const char *command,TransportInterface *transport, PlayerID playerId)
{
	transport->Send(playerId, "%s returned %i.\r\n", command, res);
}
void CommandParserInterface::ReturnResult(const char *command, TransportInterface *transport, PlayerID playerId)
{
	transport->Send(playerId, "Successfully called %s.\r\n", command);
}
void CommandParserInterface::ReturnResult(char *res, const char *command, TransportInterface *transport, PlayerID playerId)
{
	transport->Send(playerId, "%s returned %s.\r\n", command, res);
}
void CommandParserInterface::ReturnResult(PlayerID res, const char *command, TransportInterface *transport, PlayerID playerId)
{
#if !defined(_COMPATIBILITY_1)
	in_addr in;
	in.s_addr = playerId.binaryAddress;
	inet_ntoa( in );
	transport->Send(playerId, "%s returned %s %i:%i\r\n", command,inet_ntoa( in ),res.binaryAddress, res.port);
#else
	transport->Send(playerId, "%s returned %i:%i\r\n", command,res.binaryAddress, res.port);
#endif
}
PlayerID CommandParserInterface::IntegersToPlayerID(int binaryAddress, int port)
{
	PlayerID playerId;
	playerId.binaryAddress=binaryAddress;
	playerId.port=(unsigned short)port;
	return playerId;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

