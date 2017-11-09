#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Engine.h"

class FVoxelTcpSender 
{
public:
	TSharedPtr<FInternetAddr> RemoteAddr;
	FSocket* Socket;

	FVoxelTcpSender();
	~FVoxelTcpSender();

	bool StartTCPSender(const FString& Ip, const int32 Port);

	bool SendData(TArray<uint8> Data);
};

class FVoxelTcpListener
{
public:
	FTcpListener* TcpListener;
	FSocket* Socket;

	FVoxelTcpListener();
	~FVoxelTcpListener();

	void StartTCPListener(const FString& Ip, const int32 Port);

	bool ReceiveData(TArray<uint8>& OutData);

	bool Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint);
};