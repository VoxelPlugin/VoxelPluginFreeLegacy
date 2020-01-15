// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMultiplayer/VoxelMultiplayerInterfaceWithSocket.h"
#include "VoxelMultiplayerTcp.generated.h"

class FSocket;
class FTcpListener;
struct FIPv4Endpoint;
class FVoxelMultiplayerTcpServer;
class FVoxelMultiplayerTcpClient;

// TCP interface, only accepts IPv4
UCLASS()
class VOXEL_API UVoxelMultiplayerTcpInterface : public UVoxelMultiplayerInterface
{
	GENERATED_BODY()
	
public:
	/**
	 * Connect to a TCP server
	 * @param	Ip		The IPv4 of the server
	 * @param	Port	The port of the server
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer|Tcp")
	bool ConnectToServer(FString& OutError, const FString& Ip = TEXT("127.0.0.1"), int32 Port = 10000);
	
	/**
	 * Start a TCP server
	 * @param	Ip		The IPv4 to accept connection on. 0.0.0.0 to accept all
	 * @param	Port	The port of the server
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer|Tcp")
	bool StartServer(FString& OutError, const FString& Ip = TEXT("0.0.0.0"), int32 Port = 10000);

};

