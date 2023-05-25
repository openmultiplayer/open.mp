/// \file
/// \brief A structure that holds all statistical data returned by RakNet.
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


#ifndef __RAK_NET_STATISTICS_H
#define __RAK_NET_STATISTICS_H

#include "PacketPriority.h"
#include "Export.h"
#include "NetworkTypes.h"

namespace RakNet
{
	/// \brief Network Statisics Usage 
	///
	/// Store Statistics information related to network usage 
	struct RAK_DLL_EXPORT RakNetStatisticsStruct
	{
		///  Number of Messages in the send Buffer (high, medium, low priority)
		unsigned messageSendBuffer[ NUMBER_OF_PRIORITIES ];
		///  Number of messages sent (high, medium, low priority)
		unsigned messagesSent[ NUMBER_OF_PRIORITIES ];
		///  Number of data bits used for user messages
		unsigned messageDataBitsSent[ NUMBER_OF_PRIORITIES ];
		///  Number of total bits used for user messages, including headers
		unsigned messageTotalBitsSent[ NUMBER_OF_PRIORITIES ];
		
		///  Number of packets sent containing only acknowledgements
		unsigned packetsContainingOnlyAcknowlegements;
		///  Number of acknowledgements sent
		unsigned acknowlegementsSent;
		///  Number of acknowledgements waiting to be sent
		unsigned acknowlegementsPending;
		///  Number of acknowledgements bits sent
		unsigned acknowlegementBitsSent;
		
		///  Number of packets containing only acknowledgements and resends
		unsigned packetsContainingOnlyAcknowlegementsAndResends;
		
		///  Number of messages resent
		unsigned messageResends;
		///  Number of bits resent of actual data
		unsigned messageDataBitsResent;
		///  Total number of bits resent, including headers
		unsigned long long messagesTotalBitsResent;
		///  Number of messages waiting for ack (// TODO - rename this)
		unsigned messagesOnResendQueue;
		
		///  Number of messages not split for sending
		unsigned numberOfUnsplitMessages;
		///  Number of messages split for sending
		unsigned numberOfSplitMessages;
		///  Total number of splits done for sending
		unsigned totalSplits;
		
		///  Total packets sent
		unsigned packetsSent;
		
		///  Number of bits added by encryption
		unsigned encryptionBitsSent;
		///  total bits sent
		unsigned long long totalBitsSent;
		
		///  Number of sequenced messages arrived out of order
		unsigned sequencedMessagesOutOfOrder;
		///  Number of sequenced messages arrived in order
		unsigned sequencedMessagesInOrder;
		
		///  Number of ordered messages arrived out of order
		unsigned orderedMessagesOutOfOrder;
		///  Number of ordered messages arrived in order
		unsigned orderedMessagesInOrder;
		
		///  Packets with a good CRC received
		unsigned packetsReceived;
		///  Packets with a bad CRC received
		unsigned packetsWithBadCRCReceived;
		///  Bits with a good CRC received
		unsigned bitsReceived;
		///  Bits with a bad CRC received
		unsigned bitsWithBadCRCReceived;
		///  Number of acknowledgement messages received for packets we are resending
		unsigned acknowlegementsReceived;
		///  Number of acknowledgement messages received for packets we are not resending
		unsigned duplicateAcknowlegementsReceived;
		///  Number of data messages (anything other than an ack) received that are valid and not duplicate
		unsigned messagesReceived;
		///  Number of data messages (anything other than an ack) received that are invalid
		unsigned invalidMessagesReceived;
		///  Number of data messages (anything other than an ack) received that are duplicate
		unsigned duplicateMessagesReceived;
		///  Number of messages received in one second
		unsigned perSecondReceivedMsgCount;
		///  Number of messages waiting for reassembly
		unsigned messagesWaitingForReassembly;
		///  Number of messages in reliability output queue
		unsigned internalOutputQueueSize;
		///  Current bits per second
		double bitsPerSecond;
		///  connection start time
		RakNetTime connectionStartTime;
		///  Last time raknet stored per second received messages
		RakNetTimeNS lastRecvMsgProcess;

		/// Used for messages_limit config option in open.mp
		RakNetTimeNS perFrameMessagesLimitCheckLastTick;
		unsigned int perFrameMessagesLimitCounter;
		unsigned int perSecondMessagesLimitCounter;

		/// Used for acks_limit config option in open.mp
		RakNetTimeNS perFrameAcksLimitCheckLastTick;
        unsigned int perFrameAcksLimitCounter;
        unsigned int perSecondAcksLimitCounter;

		RakNetStatisticsStruct operator +=(const RakNetStatisticsStruct& other)
		{
			unsigned i;
			for (i=0; i < NUMBER_OF_PRIORITIES; i++)
			{
				messageSendBuffer[i]+=other.messageSendBuffer[i];
				messagesSent[i]+=other.messagesSent[i];
				messageDataBitsSent[i]+=other.messageDataBitsSent[i];
				messageTotalBitsSent[i]+=other.messageTotalBitsSent[i];
			}

			packetsContainingOnlyAcknowlegements+=other.packetsContainingOnlyAcknowlegements;
			acknowlegementsSent+=other.packetsContainingOnlyAcknowlegements;
			acknowlegementsPending+=other.acknowlegementsPending;
			acknowlegementBitsSent+=other.acknowlegementBitsSent;
			packetsContainingOnlyAcknowlegementsAndResends+=other.packetsContainingOnlyAcknowlegementsAndResends;
			messageResends+=other.messageResends;
			messageDataBitsResent+=other.messageDataBitsResent;
			messagesTotalBitsResent+=other.messagesTotalBitsResent;
			messagesOnResendQueue+=other.messagesOnResendQueue;
			numberOfUnsplitMessages+=other.numberOfUnsplitMessages;
			numberOfSplitMessages+=other.numberOfSplitMessages;
			totalSplits+=other.totalSplits;
			packetsSent+=other.packetsSent;
			encryptionBitsSent+=other.encryptionBitsSent;
			totalBitsSent+=other.totalBitsSent;
			sequencedMessagesOutOfOrder+=other.sequencedMessagesOutOfOrder;
			sequencedMessagesInOrder+=other.sequencedMessagesInOrder;
			orderedMessagesOutOfOrder+=other.orderedMessagesOutOfOrder;
			orderedMessagesInOrder+=other.orderedMessagesInOrder;
			packetsReceived+=other.packetsReceived;
			packetsWithBadCRCReceived+=other.packetsWithBadCRCReceived;
			bitsReceived+=other.bitsReceived;
			bitsWithBadCRCReceived+=other.bitsWithBadCRCReceived;
			acknowlegementsReceived+=other.acknowlegementsReceived;
			duplicateAcknowlegementsReceived+=other.duplicateAcknowlegementsReceived;
			messagesReceived+=other.messagesReceived;
			invalidMessagesReceived+=other.invalidMessagesReceived;
			duplicateMessagesReceived+=other.duplicateMessagesReceived;
			messagesWaitingForReassembly+=other.messagesWaitingForReassembly;
			internalOutputQueueSize+=other.internalOutputQueueSize;

			return *this;
		}
	};

	/// Verbosity level currently supports 0 (low), 1 (medium), 2 (high)
	/// \param[in] s The Statistical information to format out
	/// \param[in] buffer The buffer containing a formated report
	/// \param[in] verbosityLevel 
	/// 0 low
	/// 1 medium 
	/// 2 high 
	void StatisticsToString( RakNetStatisticsStruct *s, char *buffer, int verbosityLevel );
}

#endif
