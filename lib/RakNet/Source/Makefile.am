# Makefile.am 
# RakNet configure script 
SUBDIRS=Autopatcher RakVoice .

lib_LTLIBRARIES=libRakNet.la


#RakNet headers 
libRakNet_ladir=$(includedir)/RakNet
libRakNet_la_HEADERS=\
	AES128.h\
	ArrayList.h\
	AsynchronousFileIO.h\
	BigTypes.h\
	BinarySearchTree.h\
	BitStream.h\
	CheckSum.h\
	ClientContextStruct.h\
	DataBlockEncryptor.h\
	DistributedNetworkObject.h\
	DistributedNetworkObjectHeader.h\
	DistributedNetworkObjectManager.h\
	DistributedNetworkObjectStub.h\
	EncodeClassName.h\
	ExtendedOverlappedPool.h\
	GetTime.h\
	HuffmanEncodingTree.h\
	HuffmanEncodingTreeFactory.h\
	HuffmanEncodingTreeNode.h\
	InternalPacket.h\
	InternalPacketPool.h\
	LinkedList.h\
	MTUSize.h\
	Multiplayer.h\
	NetworkObject.h\
	NetworkTypes.h\
	PacketEnumerations.h\
	PacketPool.h\
	PacketPriority.h\
	Queue.h\
	QueueLinkedList.h\
	RPCNode.h\
	RSACrypt.h\
	RakClient.h\
	RakClientInterface.h\
	RakNetStatistics.h\
	RakNetworkFactory.h\
	RakPeer.h\
	RakPeerInterface.h\
	RakServer.h\
	RakServerInterface.h\
	Rand.h\
	ReliabilityLayer.h\
	SHA1.h\
	SimpleMutex.h\
	SocketLayer.h\
	StringCompressor.h\
	Types.h
	
#RakNet sources files 
libRakNet_la_SOURCES=\
	AES128.cpp\
	AsynchronousFileIO.cpp\
	BitStream.cpp\
	CheckSum.cpp\
	DataBlockEncryptor.cpp\
	DistributedNetworkObject.cpp\
	DistributedNetworkObjectManager.cpp\
	DistributedNetworkObjectStub.cpp\
	EncodeClassName.cpp\
	ExtendedOverlappedPool.cpp\
	GetTime.cpp\
	HuffmanEncodingTree.cpp\
	HuffmanEncodingTreeFactory.cpp\
	InternalPacketPool.cpp\
	NetworkObject.cpp\
	NetworkTypes.cpp\
	PacketPool.cpp\
	RPCNode.cpp\
	RakClient.cpp\
	RakNetStatistics.cpp\
	RakNetworkFactory.cpp\
	RakPeer.cpp\
	RakServer.cpp\
	Rand.cpp\
	ReliabilityLayer.cpp\
	SHA1.cpp\
	SimpleMutex.cpp\
	SocketLayer.cpp\
	StringCompressor.cpp

if BUILD_RAKVOICE 

if   BUILD_MONOLITHIC
libRakNet_la_LIBADD=\
	Autopatcher/libRakNet_autopatcher.la\
	RakVoice/libRakNet_voice.la
else 
libRakNet_la_LIBADD=\
	Autopatcher/libRakNet_autopatcher.la
endif 

else 
libRakNet_la_LIBADD=\
	Autopatcher/libRakNet_autopatcher.la
endif 
