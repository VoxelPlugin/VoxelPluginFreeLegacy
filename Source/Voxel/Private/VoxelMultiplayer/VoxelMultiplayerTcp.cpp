// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelMultiplayer/VoxelMultiplayerTcp.h"
#include "VoxelMessages.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Common/TcpSocketBuilder.h"
#include "Common/TcpListener.h"
#include "Async/Async.h"

#define TCP_MAX_PACKET_SIZE 128000

bool UVoxelMultiplayerTcpInterface::ConnectToServer(FString& OutError, const FString& Ip, int32 Port)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Multiplayer with TCP sockets is only available in Voxel Plugin Pro"));
	return false;
}

bool UVoxelMultiplayerTcpInterface::StartServer(FString& OutError, const FString& Ip, int32 Port)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Multiplayer with TCP sockets is only available in Voxel Plugin Pro"));
	return false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

