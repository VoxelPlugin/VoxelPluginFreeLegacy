// Copyright 2020 Phyronnaz

#include "VoxelMultiplayer/VoxelMultiplayerTcp.h"
#include "VoxelMessages.h"
#include "VoxelThreadingUtilities.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Common/TcpSocketBuilder.h"
#include "Common/TcpListener.h"
#include "Async/Async.h"

#define TCP_MAX_PACKET_SIZE 128000

bool UVoxelMultiplayerTcpInterface::ConnectToServer(FString& OutError, const FString& Ip, int32 Port)
{
	VOXEL_PRO_ONLY();
}

bool UVoxelMultiplayerTcpInterface::StartServer(FString& OutError, const FString& Ip, int32 Port)
{
	VOXEL_PRO_ONLY();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

