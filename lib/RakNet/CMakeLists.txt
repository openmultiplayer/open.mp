if(WIN32)
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-DWIN32_LEAN_AND_MEAN
	)
endif()

set(RAKNET_SOURCES
	Source/BitStream.cpp
	Source/CheckSum.cpp
	Source/CommandParserInterface.cpp
	Source/ConsoleServer.cpp
	Source/DataBlockEncryptor.cpp
	Source/DS_HuffmanEncodingTree.cpp
	Source/GetTime.cpp
	Source/InternalPacketPool.cpp
	Source/NetworkTypes.cpp
	Source/RakClient.cpp
	Source/RakNetCommandParser.cpp
	Source/RakNetworkFactory.cpp
	Source/RakPeer.cpp
	Source/rakserver.cpp
	Source/RakSleep.cpp
	Source/Rand.cpp
	Source/ReliabilityLayer.cpp
	Source/rijndael.cpp
	Source/RPCMap.cpp
	Source/SHA1.cpp
	Source/SimpleMutex.cpp
	Source/SocketLayer.cpp
	Source/StringCompressor.cpp
	Source/StringTable.cpp
	Source/PluginInterface.cpp
	SAMPRakNet.cpp
)

add_library(raknet STATIC ${RAKNET_SOURCES})

set_property(TARGET raknet PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(raknet
	PUBLIC
		-DRAKNET_LEGACY
		-DPACKETS_PER_TICK=3
)
target_include_directories(raknet 
	INTERFACE Include
	PRIVATE Include/raknet
)

if (WIN32)
	target_link_libraries(raknet Ws2_32 OMP-SDK)
else()
	target_link_libraries(raknet OMP-SDK)
endif()
