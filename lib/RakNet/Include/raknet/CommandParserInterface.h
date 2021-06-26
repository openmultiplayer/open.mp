/// \file
/// \brief Contains CommandParserInterface , from which you derive custom command parsers
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#ifndef __COMMAND_PARSER_INTERFACE
#define __COMMAND_PARSER_INTERFACE

#include "NetworkTypes.h"
#include "DS_OrderedList.h"
#include "Export.h"

namespace RakNet
{
	class TransportInterface;

	/// \internal
	/// Contains the information related to one command registered with RegisterCommand()
	/// Implemented so I can have an automatic help system via SendCommandList()
	struct RAK_DLL_EXPORT RegisteredCommand
	{
		const char *command;
		const char *commandHelp;
		unsigned char parameterCount;
	};

	/// List of commands registered with RegisterCommand()
	int RAK_DLL_EXPORT RegisteredCommandComp( const char* const & key, const RegisteredCommand &data );

	/// CommandParserInterface provides a set of functions and interfaces that plug into the ConsoleServer class.
	/// Each CommandParserInterface works at the same time as other interfaces in the system.
	/// \brief The interface used by command parsers.
	class RAK_DLL_EXPORT CommandParserInterface
	{
	public:
		CommandParserInterface();
		virtual ~CommandParserInterface();

		/// You are responsible for overriding this function and returning a static string, which will identifier your parser.
		/// This should return a static string
		/// \return The name that you return.
		virtual char *GetName(void) const=0;

		/// A callback for when \a playerId has connected to us.
		/// \param[in] playerId The player that has connected.
		/// \param[in] transport The transport interface that sent us this information.  Can be used to send messages to this or other players.
		virtual void  OnNewIncomingConnection(PlayerID playerId, TransportInterface *transport);

		/// A callback for when \a playerId has disconnected, either gracefully or forcefully
		/// \param[in] playerId The player that has disconnected.
		/// \param[in] transport The transport interface that sent us this information.
		virtual void OnConnectionLost(PlayerID playerId, TransportInterface *transport);

		/// A callback for when you are expected to send a brief description of your parser to \a playerId
		/// \param[in] transport The transport interface we can use to write to
		/// \param[in] playerId The player that requested help.
		virtual void SendHelp(TransportInterface *transport, PlayerID playerId)=0;

		/// Given \a command with parameters \a parameterList , do whatever processing you wish.
		/// \param[in] command The command to process
		/// \param[in] numParameters How many parameters were passed along with the command
		/// \param[in] parameterList The list of parameters.  parameterList[0] is the first parameter and so on.
		/// \param[in] transport The transport interface we can use to write to
		/// \param[in] playerId The player that sent this command.
		/// \param[in] originalString The string that was actually sent over the network, in case you want to do your own parsing
		virtual bool OnCommand(const char *command, unsigned numParameters, char **parameterList, TransportInterface *transport, PlayerID playerId, const char *originalString)=0;

		/// This is called every time transport interface is registered.  If you want to save a copy of the TransportInterface pointer
		/// This is the place to do it
		/// \param[in] transport The new TransportInterface
		virtual void OnTransportChange(TransportInterface *transport);

		/// \internal
		/// Scan commandList and return the associated array
		/// \param[in] command The string to find
		/// \param[out] rc Contains the result of this operation
		/// \return True if we found the command, false otherwise
		virtual bool GetRegisteredCommand(const char *command, RegisteredCommand *rc);

		/// \internal
		/// Goes through str, replacing the delineating character with 0's.
		/// \param[in] str The string sent by the transport interface
		/// \param[in] delineator The character to scan for to use as a delineator
		/// \param[in] delineatorToggle When encountered the delineator replacement is toggled on and off
		/// \param[out] numParameters How many pointers were written to \a parameterList
		/// \param[out] parameterList An array of pointers to characters.  Will hold pointers to locations inside \a str
		/// \param[in] parameterListLength How big the \a parameterList array is
		static void ParseConsoleString(char *str, const char delineator, unsigned char delineatorToggle, unsigned *numParameters, char **parameterList, unsigned parameterListLength);

		/// \internal
		/// Goes through the variable commandList and sends the command portion of each struct
		/// \param[in] transport The transport interface we can use to write to
		/// \param[in] playerId The player to write to
		virtual void SendCommandList(TransportInterface *transport, PlayerID playerId);

		static const unsigned char VARIABLE_NUMBER_OF_PARAMETERS;

	protected:
		// Currently only takes static strings - doesn't make a copy of what you pass.
		// parameterCount is the number of parameters that the sender has to include with the command.
		// Pass 255 to parameterCount to indicate variable number of parameters

		/// Registers a command.
		/// \param[in] parameterCount How many parameters your command requires.  If you want to accept a variable number of commands, pass CommandParserInterface::VARIABLE_NUMBER_OF_PARAMETERS
		/// \param[in] command A pointer to a STATIC string that has your command.  I keep a copy of the pointer here so don't deallocate the string.
		/// \param[in] commandHelp A pointer to a STATIC string that has the help information for your command.  I keep a copy of the pointer here so don't deallocate the string.
		virtual void RegisterCommand(unsigned char parameterCount, const char *command, const char *commandHelp);

		/// Just writes a string to the remote system based on the result ( \a res )of your operation
		/// This is not necessary to call, but makes it easier to return results of function calls
		/// \param[in] res The result to write
		/// \param[in] command The command that this result came from
		/// \param[in] transport The transport interface that will be written to
		/// \param[in] playerId The player this result will be sent to
		virtual void ReturnResult(bool res, const char *command, TransportInterface *transport, PlayerID playerId);
		virtual void ReturnResult(char *res, const char *command, TransportInterface *transport, PlayerID playerId);
		virtual void ReturnResult(PlayerID res, const char *command, TransportInterface *transport, PlayerID playerId);
		virtual void ReturnResult(int res, const char *command,TransportInterface *transport, PlayerID playerId);

		/// Just writes a string to the remote system when you are calling a function that has no return value
		/// This is not necessary to call, but makes it easier to return results of function calls
		/// \param[in] res The result to write
		/// \param[in] command The command that this result came from
		/// \param[in] transport The transport interface that will be written to
		/// \param[in] playerId The player this result will be sent to
		virtual void ReturnResult(const char *command,TransportInterface *transport, PlayerID playerId);


		/// Since there's no way to specify a playerID directly, the user needs to specify both the binary address and port.
		/// Given those parameters, this returns the corresponding PlayerID
		/// \param[in] binaryAddress The binaryAddress portion of PlayerID
		/// \param[in] port The port portion of PlayerID
		PlayerID IntegersToPlayerID(int binaryAddress, int port);

		DataStructures::OrderedList<const char*, RegisteredCommand, RegisteredCommandComp> commandList;
	};
}

#endif
